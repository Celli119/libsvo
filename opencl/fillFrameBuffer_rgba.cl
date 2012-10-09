




///////////////////////////////////////////////////////////////////////////////

//
__constant float scale = 1.0f;


__kernel void fillBufferWithColor (__write_only image2d_t pixels,
                                   const float4           color)
{

  float time = color.z*0.075f;


  unsigned x = get_global_id(0);
  unsigned y = get_global_id(1);


  float dist = length((float2)(x,y));

  float a = sin(dist*dist*0.1f*scale + time * 0.2f)*0.5f + 0.5f;
  float b = sin(dist*dist*0.2f*scale + time * 1.0f)*0.5f + 0.5f;

   float val = length((float2)(a,b));

   write_imagef (pixels,
                (int2)(x,y),
                (float4)(a,b,1.0f-val,1.0f));
}


///////////////////////////////////////////////////////////////////////////////


//
typedef struct
{
  unsigned _firstChildIndex;
  unsigned _masks;
//  unsigned _attibutePos;
} __attribute__ ( ( aligned ( 8 ) ) ) SvoNode;


//
typedef struct
{
  unsigned _packed_normal;
  unsigned _packed_color;
} __attribute__ ( ( aligned ( 8 ) ) ) Attribs;


//
typedef struct
{
  unsigned parentNodeIndex;
  float    parentTMin;
  float    parentTMax;
  float3   parentCenter;
} __attribute__ ( ( aligned ( 32 ) ) ) StackElement;


//
typedef struct
{
  bool     hit;
  unsigned nodeIndex;
  unsigned attribIndex;
  unsigned depth;
  float    t;
  unsigned numWhileLoops;
  float3   nodeCenter;
} __attribute__ ( ( aligned ( 32 ) ) ) SampleResult;



///////////////////////////////////////////////////////////////////////////////



inline void swapToIncreasing(float* value1, float* value2)
{
  if (*value1 > *value2)
  {
    float temp = *value1;
    *value1 = *value2;
    *value2 = temp;
  }
}


///////////////////////////////////////////////////////////////////////////////

#define SVO_CPUSVONODE_OFFSET_LEAFMASK  0
#define SVO_CPUSVONODE_OFFSET_VALIDMASK 8


inline bool getValidMaskFlag(const unsigned masks, const unsigned flag)
{
  return (bool)(masks & (1 << (flag+SVO_CPUSVONODE_OFFSET_VALIDMASK)));
}


inline bool getLeafMaskFlag(const unsigned masks, const unsigned flag)
{
  return (bool)(masks & (1 << (flag+SVO_CPUSVONODE_OFFSET_LEAFMASK)));
}


inline unsigned getNthChildIndex(const unsigned masks,
                                 const unsigned firstChildIndex,
                                 const unsigned childIndex)
{
  unsigned index = firstChildIndex;

  for (unsigned i=0; i!=childIndex; ++i)
  {
    index += getValidMaskFlag(masks, i);
  }

  return index;
}

///////////////////////////////////////////////////////////////////////////////


inline bool
intersectAABB( const float3 rayOrigin,
               const float3 rayDirection,
               float* tMin,
               float* tMax)
{


	// source: http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm


	/*
	For each pair of planes P associated with X, Y, and Z do:
  (example using X planes)
  if direction Xd = 0 then the ray is parallel to the X planes.
  So, if origin Xo is not between the slabs ( Xo < Xl or Xo > Xh) then return false
  else, if the ray is not parallel to the plane then
  begin
  compute the intersection distance of the planes
	T1 = (Xl - Xo) / Xd
  T2 = (Xh - Xo) / Xd
  If T1 > T2 swap (T1, T2) // since T1 intersection with near plane

  If T1 > Tnear set Tnear =T1 // want largest Tnear
  If T2 < Tfar set Tfar="T2" // want smallest Tfar

  If Tnear > Tfar box is missed so return false
  If Tfar < 0 box is behind ray return false end

  end of for loop
  If Box survived all above tests, return true with
  intersection point Tnear and exit point Tfar.
	*/

	float tNear                = -10000000.0f;
	float tFar                 =  10000000.0f;
	const float boxRadius      = 0.5f;
//	const float epsilon        = 0.00001f;
	const float rayOriginArray[3]    = {rayOrigin.x, rayOrigin.y, rayOrigin.z};
	const float rayDirectionArray[3] = {rayDirection.x, rayDirection.y, rayDirection.z};


  // parallel to plane 0..1
	for (int i=0; i!=3; ++i)
	{
    if (rayDirectionArray[i] == 0)
    {
      if ((-boxRadius > rayOriginArray[i] || boxRadius < rayOriginArray[i]))
      {
        return false;
      }
    }

    float t1 = (-boxRadius - rayOriginArray[i]) / rayDirectionArray[i];
    float t2 = (boxRadius - rayOriginArray[i]) / rayDirectionArray[i];

    // swap
    if (t1 > t2)
    {
      float temp = t1;
      t1 = t2;
      t2 = temp;
    }

    if (t1 > tNear)
    {
      tNear = t1;
    }

    if (t2 < tFar)
    {
      tFar = t2;
    }

    if (tNear > tFar)
    {
      return false;
    }

    if (tFar < 0.0f)
    {
      return false;
    }
	}

  *tMin = tNear;
  *tMax = tFar;

  return true;
}


///////////////////////////////////////////////////////////////////////////////

inline float4
unpackRgbaFromUnsigned(unsigned value32)
{
  unsigned char a = (unsigned char)(value32 & 255);
  value32 >>= 8;
  unsigned char b = (unsigned char)(value32 & 255);
  value32 >>= 8;
  unsigned char g = (unsigned char)(value32 & 255);
  value32 >>= 8;
  unsigned char r = (unsigned char)(value32 & 255);

  return (float4)(r,g,b,a);
}


///////////////////////////////////////////////////////////////////////////////


bool
sample( __global const SvoNode* svo,
        float3 rayOrigin,
        float3 rayDirection,
        const float tScaleRatio,
        SampleResult* result)
{
  result->numWhileLoops = 0;
  result->attribIndex   = 0;
  result->hit           = false;

  float tMin,tMax;
  if (!intersectAABB( rayOrigin, rayDirection, &tMin, &tMax))
  {
    return false;
  }


  const int   scaleMax   = 12;
  int         scale      = scaleMax-1;
  float       scale_exp2 = 0.5f;// exp2f(scale - s_max)
  const float epsilon    = 0.00004f;

  StackElement stack[12];

  // init stack with root node
  stack[scale].parentNodeIndex = 0;
  stack[scale].parentTMin      = tMin;
  stack[scale].parentTMax      = tMax;
  stack[scale].parentCenter    = (float3)(0.0f,0.0f,0.0f);

  if ( fabs(rayDirection.x) < epsilon) rayDirection.x = epsilon * sign(rayDirection.x)*10.0f;
  if ( fabs(rayDirection.y) < epsilon) rayDirection.y = epsilon * sign(rayDirection.y)*10.0f;
  if ( fabs(rayDirection.z) < epsilon) rayDirection.z = epsilon * sign(rayDirection.z)*10.0f;
//  rayDirection = (fabs(rayDirection.x) < epsilon) ? epsilon * sign(rayDirection.x) : rayDirection.x;


  // precalculate ray coefficients, tx(x) = "(1/dx)"x + "(-px/dx)"
  float dxReziprok = 1.0f/rayDirection.x;
  float minusPx_dx = -rayOrigin.x*dxReziprok;

  float dyReziprok = 1.0f/rayDirection.y;
  float minusPy_dy = -rayOrigin.y*dyReziprok;

  float dzReziprok = 1.0f/rayDirection.z;
  float minusPz_dz = -rayOrigin.z*dzReziprok;

  float  childSizeHalf;

  StackElement* parent = 0;


  unsigned       whileCounter = 0;
  const unsigned maxLoops     = (scaleMax+1)*(scaleMax+1)*5.0f;
//  const unsigned maxLoops     = 500;

//  return false;

/////////////////// LOOP ///////////////////////////////XS

  while (scale < scaleMax)
  {
    ++whileCounter;

    if (whileCounter == maxLoops)
    {
      break;
    }

    if (!parent)
    {
      parent = &stack[scale];
      /// hier den Punkt tMin vom parent benutzen um einstiegskind zu bekommen
      /// x0,x1,y0,y1,z0,z1 für das einstiegskind setzen.
      /// dann mit tx1, ty1 und tz1 < tcmax die folgekinder bestimmen
      scale_exp2       = pow(2.0f, scale - scaleMax);
      childSizeHalf    = scale_exp2*0.5f;
    }


    // POP if parent is behind the camera
    if ( parent->parentTMax < 0.0f)
    {
      ++scale;
      parent = 0;
      continue;
    }

    if (fabs(parent->parentTMin - parent->parentTMax) > epsilon)
    {
      // childEntryPoint in parent voxel coordinates
      float3 childEntryPoint = (rayOrigin + (parent->parentTMin + epsilon) * rayDirection) - parent->parentCenter;
      int childIndex         =  (int) (   4*(childEntryPoint.x > 0.0f)
                                        + 2*(childEntryPoint.y > 0.0f)
                                        +   (childEntryPoint.z > 0.0f));


      // childCenter in world coordinates
      float3 childCenter = (float3)(-0.5f + (bool)(childIndex & 4),
                                    -0.5f + (bool)(childIndex & 2),
                                    -0.5f + (bool)(childIndex & 1))*scale_exp2 + parent->parentCenter;

      // tx(x) = (1/dx)x + (-px/dx)
      // dx...Direction of the Ray in x
      // px...Origin of the Ray in x
      float tx0 = dxReziprok*(childCenter.x - childSizeHalf) + minusPx_dx;
      float tx1 = dxReziprok*(childCenter.x + childSizeHalf) + minusPx_dx;
      swapToIncreasing(&tx0, &tx1);

      float ty0 = dyReziprok*(childCenter.y - childSizeHalf) + minusPy_dy;
      float ty1 = dyReziprok*(childCenter.y + childSizeHalf) + minusPy_dy;
      swapToIncreasing(&ty0, &ty1);

      float tz0 = dzReziprok*(childCenter.z - childSizeHalf) + minusPz_dz;
      float tz1 = dzReziprok*(childCenter.z + childSizeHalf) + minusPz_dz;
      swapToIncreasing(&tz0, &tz1);

      float tcMin = max(tx0, max(ty0, tz0)); // <- you can only enter once
      float tcMax = min(tx1, min(ty1, tz1)); // <- you can only leave once


      // if child is valid
      if (getValidMaskFlag(svo[parent->parentNodeIndex]._masks, childIndex))
      {
        // TERMINATE if voxel is a leaf
        if (getLeafMaskFlag(svo[parent->parentNodeIndex]._masks, childIndex))
        {
          unsigned returnChildIndex = getNthChildIndex(svo[parent->parentNodeIndex]._masks,
                                                       svo[parent->parentNodeIndex]._firstChildIndex,
                                                       childIndex);

          result->hit           = true;
          result->nodeIndex     = returnChildIndex;
//          result->attribIndex   = svo[returnChildIndex]._attibutePos;
          result->depth         = scaleMax-scale;
          result->t             = parent->parentTMin;
          result->numWhileLoops = whileCounter;
          result->nodeCenter    = childCenter;
          return true;
        }
        else
        {
          // TERMINATE if voxel is small enough
          if (tScaleRatio*tcMax > scale_exp2)
          {
            unsigned returnChildIndex = getNthChildIndex(svo[parent->parentNodeIndex]._masks,
                                                         svo[parent->parentNodeIndex]._firstChildIndex,
                                                         childIndex);
            result->hit           = true;
            result->nodeIndex     = returnChildIndex;
//            result->attribIndex   = svo[returnChildIndex]._attibutePos;
            result->depth         = scaleMax-scale;
            result->t             = parent->parentTMin;
            result->numWhileLoops = whileCounter;
            result->nodeCenter    = childCenter;
            return true;
          }

          // update parent befor push
          parent->parentTMin = tcMax;

          // PUSH
          --scale;
          stack[scale].parentNodeIndex = getNthChildIndex(svo[parent->parentNodeIndex]._masks,
                                                          svo[parent->parentNodeIndex]._firstChildIndex,
                                                          childIndex);
          stack[scale].parentTMin      = tcMin;
          stack[scale].parentTMax      = tcMax;
          stack[scale].parentCenter    = childCenter;

          parent = 0;
          continue;
        }
      }
      else
      {
        // ADVANCE
//        parentTMin = tcMax;
        parent->parentTMin = tcMax;
      }
    }
    else
    {
      // POP
      ++scale;
      parent = 0;
      continue;
    }

  } // while


  result->hit           = false;
  result->nodeIndex     = 0;
  result->attribIndex   = 0;
  result->depth         = 0;
  result->t             = tMax;
  result->numWhileLoops = whileCounter;


  return false;
}


///////////////////////////////////////////////////////////////////////////////


inline float3
getNormal(unsigned attribPos,
          __global Attribs* attribs)
{
  return ((unpackRgbaFromUnsigned(attribs[attribPos]._packed_normal)*0.003921569f).xyz-((float3)(0.5f,0.5f,0.5f)))*2.0f;
}


///////////////////////////////////////////////////////////////////////////////


inline float4
getColor(unsigned attribPos,
          __global Attribs* attribs)
{
  return ((unpackRgbaFromUnsigned(attribs[attribPos]._packed_color)*0.003921569f));
}


///////////////////////////////////////////////////////////////////////////////


inline float4
shade_iterationDepth(const SampleResult* result)
{
  const float value =  1.0f - result->numWhileLoops/300.0f;

  return (float4)( value,
                   value,
                   value, 1.0f);

}


///////////////////////////////////////////////////////////////////////////////


inline float4
shade_pixelDepth(const SampleResult* result)
{
  const float value = 1.0f - result->t * 0.1f;
  return (float4)( value,
                   value,
                   value, 1.0f);
}


///////////////////////////////////////////////////////////////////////////////


inline float4
shade_svoDepth(const SampleResult* result)
{
  const float value = result->depth/12.0f;
  return (float4)( value*result->hit,
                   value*result->hit,
                   value*result->hit, 1.0f);
}


///////////////////////////////////////////////////////////////////////////////


inline float4
shade_diffuse_color(const SampleResult* result,
                    __global const SvoNode* svo,
                    __global Attribs* attribs)
{
  if (result->hit)
  {
    const unsigned normalPacked = attribs[result->attribIndex]._packed_normal;
    const unsigned colorPacked  = attribs[result->attribIndex]._packed_color;

    float3 normal = (unpackRgbaFromUnsigned(normalPacked)*0.003921569f).xyz;
    normal.x = ((normal.x)-0.5f) * 2.0f;
    normal.y = ((normal.y)-0.5f) * 2.0f;
    normal.z = ((normal.z)-0.5f) * 2.0f;

    float4 color = unpackRgbaFromUnsigned(colorPacked)*0.003921569f;


    const float3 lightDirection = normalize((float3)(-0.5f, 1.0f, 0.6f));
    float nDotL = max(0.0f, dot(normal, lightDirection));

#if 1
    SampleResult resultShadow;
    float3 shadowRayOrigin = result->nodeCenter + normal*0.01f;
    float shadow = sample( svo,
                          shadowRayOrigin, lightDirection,
                          0.001f,
                          &resultShadow);
#else
    float shadow = 1.0f;
#endif

    color = color*nDotL*(1.0f-shadow)+color*0.2f;
    color.w = 1.0f;

    return color;
  }

  return (float4)( 0.2f,
                   0.3f,
                   0.2f,
                   1.0f);
}


///////////////////////////////////////////////////////////////////////////////


inline float4
shade_diffuse_color_reflection(const SampleResult* result,
                                __global const SvoNode* svo,
                                __global Attribs* attribs,
                                const float3 rayDirection,
                                int          numReflections)
{

  if (result->hit)
  {
    SampleResult refectResult = *result;

    float4 resultcolor   = (float4)(0.0f,0.0f,0.0f,0.0f);
    float3 viewDirection = rayDirection;

    int reflectCounter = numReflections;

    float strength = 0.75f;

    while (true)
    {
      // color and normal for current sample
      float3 normal = getNormal(refectResult.attribIndex, attribs);

      resultcolor = resultcolor * (1.0f-strength) + shade_diffuse_color(&refectResult, svo, attribs)*strength;

      strength *= 0.75f;

      if (reflectCounter == 0 || !refectResult.hit)
      {
        break;
      }
      else
      {

        const float3 reflectDir    = normalize(viewDirection + ( -2.0f * normal * dot( normal, viewDirection )));
        float3       reflectOrigin = refectResult.nodeCenter  + normal*0.01f;

        sample( svo,
                reflectOrigin, reflectDir,
                0.001f,
                &refectResult);

        viewDirection = reflectDir;

        --reflectCounter;
      }
    }
    return resultcolor;
  }
  return (float4)( 0.2f,
                   0.3f,
                   0.2f,
                   1.0f);
}


///////////////////////////////////////////////////////////////////////////////


__kernel void
renderToBuffer ( __write_only image2d_t renderbuffer,
                 __global SvoNode*      svo,
                 __global unsigned*     attribIndices,
                 __global Attribs*      attribs,
                 const float4           frameBufferSize,
                 const float4           pixelFarVectorH,
                 const float4           pixelFarVectorV,
                 const float4           far_lower_left,
                 const float4           cameraPosition,
                 const float4           otherParams)
{
  const unsigned x = get_global_id(0);
  const unsigned y = get_global_id(1);


  // create ray for this pixel
  float4 pickPointOnFarPlane = far_lower_left
                               + (pixelFarVectorH * x)
                               + (pixelFarVectorV * ((int)frameBufferSize.y - y));

  float4 rayDirection = pickPointOnFarPlane - cameraPosition;
  rayDirection.w      = 0.0f;
  rayDirection        = normalize(rayDirection);

  SampleResult result;

  // primary ray
  sample( svo,
          cameraPosition.xyz, rayDirection.xyz,
          frameBufferSize.z,
          &result); // wich is the tScaleRatio

  switch ((int)otherParams.x) // rendermode select
  {
    case 0:
        {
          if (result.hit)
          {
            result.attribIndex = attribIndices[result.nodeIndex];


            float4 color = getColor(result.attribIndex, attribs);
            color.w = 1.0f;
//
            write_imagef ( renderbuffer,
                          (int2)(x,y),
                          color);
            return;
          }

           write_imagef ( renderbuffer,
                          (int2)(x,y),
                          (float4)(0.2f,0.3f,0.2f,1.0f));
          }
          break;
    case 1:
        write_imagef ( renderbuffer,
                      (int2)(x,y),
                     (float4)(getNormal(result.attribIndex, attribs), 1.0));
        break;

    case 2:
        write_imagef ( renderbuffer,
                      (int2)(x,y),
                      shade_svoDepth(&result));
        break;

    case 3:
        write_imagef ( renderbuffer,
                      (int2)(x,y),
                      shade_pixelDepth(&result));
        break;

    case 4:
        write_imagef ( renderbuffer,
                      (int2)(x,y),
                      shade_iterationDepth(&result));
        break;

    case 5:
        write_imagef ( renderbuffer,
                      (int2)(x,y),
                      shade_diffuse_color_reflection(&result,
                                                svo,
                                                attribs,
                                                rayDirection.xyz,
                                                2));
        break;

    case 6:
        write_imagef ( renderbuffer,
                      (int2)(x,y),
                      shade_diffuse_color(&result,
                                          svo,
                                          attribs));
        break;
  }
}



///////////////////////////////////////////////////////////////////////////////


