


///////////////////////////////////////////////////////////////////////////////

//
//__constant float scale = 1.0f;
#define MAX_STACK_SIZE        12
#define MAX_SVO_RAYCAST_DEPTH 12


///////////////////////////////////////////////////////////////////////////////


//
typedef struct {
  unsigned _firstchildIndex;
  unsigned _masks;
//  unsigned _attibutePos;
} __attribute__ ( ( aligned ( 8 ) ) ) SvoNode;


//Ŕ
typedef struct {
  unsigned _packed_normal;
  unsigned _packed_color;
} __attribute__ ( ( aligned ( 8 ) ) ) Attribs;


//
typedef struct {
  unsigned parentNodeIndex;
  float    parentTMin;
  float    parentTMax;
  float3   parentCenter;
} /*__attribute__ ( ( aligned ( 32 ) ) )*/ StackElement;


//
typedef struct {
  bool     hit;
  unsigned nodeIndex;
  unsigned attribIndex;
  unsigned depth;
  float    t;
  float    quality;
  unsigned numWhileLoops;
  float3   nodeCenter;
} __attribute__ ( ( aligned ( 64 ) ) ) SampleResult;



///////////////////////////////////////////////////////////////////////////////



inline void swapToIncreasing(float* value1, float* value2)
{
  if (*value1 > *value2) {
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


inline unsigned getNthchildIdx( const unsigned masks,
                                const unsigned firstchildIdx,
                                const unsigned childIdx)
{
  unsigned index = firstchildIdx;

  for (unsigned i=0; i!=childIdx; ++i) {
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
  const float rayOriginArray[3]    = {rayOrigin.x, rayOrigin.y, rayOrigin.z};
  const float rayDirectionArray[3] = {rayDirection.x, rayDirection.y, rayDirection.z};


  // parallel to plane 0..1
  for (int i=0; i!=3; ++i) {
    if (rayDirectionArray[i] == 0 && (-boxRadius > rayOriginArray[i] || boxRadius < rayOriginArray[i])) {
      return false;
    }

    float t1 = (-boxRadius - rayOriginArray[i]) / rayDirectionArray[i];
    float t2 = (boxRadius - rayOriginArray[i]) / rayDirectionArray[i];

    // swap
    if (t1 > t2) {
      float temp = t1;
      t1 = t2;
      t2 = temp;
    }

    if (t1 > tNear) {
      tNear = t1;
    }

    if (t2 < tFar) {
      tFar = t2;
    }

    if (tNear > tFar) {
      return false;
    }

    if (tFar < 0.0f) {
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
  if (!intersectAABB( rayOrigin, rayDirection, &tMin, &tMax)) {
    return false;
  }

  const int   scaleMax   = MAX_STACK_SIZE;
  int         scale      = scaleMax-1;
  float       scale_exp2 = 0.5f;// exp2f(scale - s_max)
  const float minNormal  = 0.0001f;
  const float epsilon    = 0.0001f;

  private StackElement stack[MAX_STACK_SIZE];

  // init stack with root node
  stack[scale].parentNodeIndex = 0;
  stack[scale].parentTMin      = tMin;
  stack[scale].parentTMax      = tMax;
  stack[scale].parentCenter    = (float3)(0.0f,0.0f,0.0f);

  if ( fabs(rayDirection.x) < minNormal) {
    rayDirection.x = minNormal * sign(rayDirection.x);
  }
  if ( fabs(rayDirection.y) < minNormal) {
    rayDirection.y = minNormal * sign(rayDirection.y);
  }
  if ( fabs(rayDirection.z) < minNormal) {
    rayDirection.z = minNormal * sign(rayDirection.z);
  }
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
  const unsigned maxLoops     = (scaleMax)*(scaleMax)*(scaleMax);

/////////////////// LOOP ///////////////////////////////XS

  while (scale < scaleMax)
  {

    parent           = &stack[scale];
    scale_exp2       = pow(2.0f, scale - scaleMax);
    childSizeHalf    = scale_exp2*0.5f;

    ++whileCounter;
    if (whileCounter == maxLoops)
    {
      result->hit           = true;
      result->nodeIndex     = parent->parentNodeIndex;
      result->depth         = scaleMax-scale;
      result->t             = parent->parentTMin;
      result->numWhileLoops = whileCounter;
      result->nodeCenter    = parent->parentCenter;
      result->quality       = scale_exp2-(tScaleRatio*parent->parentTMin);
      return true;
    }


    // ### POP if parent is behind the camera
    if ( parent->parentTMax < 0.0f) {
      ++scale;
      continue;
    }

    if (fabs(parent->parentTMin - parent->parentTMax) > epsilon) {
      // childEntryPoint in parent voxel coordinates
      float3 childEntryPoint = (rayOrigin + (parent->parentTMin + epsilon) * rayDirection) - parent->parentCenter;
      int childIdx           =  (int) (   4*(childEntryPoint.x > 0.0f)
                                        + 2*(childEntryPoint.y > 0.0f)
                                        +   (childEntryPoint.z > 0.0f));

      // childCenter in world coordinates
      float3 childCenter = (float3)(-0.5f + (bool)(childIdx & 4),
                                    -0.5f + (bool)(childIdx & 2),
                                    -0.5f + (bool)(childIdx & 1))*scale_exp2 + parent->parentCenter;

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
      if (getValidMaskFlag(svo[parent->parentNodeIndex]._masks, childIdx)) {
        // TERMINATE if voxel is a leaf
        if (getLeafMaskFlag(svo[parent->parentNodeIndex]._masks, childIdx)) {
          unsigned leafIndex = parent->parentNodeIndex + getNthchildIdx( svo[parent->parentNodeIndex]._masks,
                               svo[parent->parentNodeIndex]._firstchildIndex,
                               childIdx);
//          // check if leaf points to another treelet
////          if (svo[leafIndex]._masks)
//          {
//            // update parent befor push
//            parent->parentTMin = tcMax;
//
//            // ### PUSH
//            --scale;
////            stack[scale].parentNodeIndex = svo[leafIndex]._masks;
//            stack[scale].parentNodeIndex = 0;
//            stack[scale].parentTMin      = tcMin;
//            stack[scale].parentTMax      = tcMax;
//            stack[scale].parentCenter    = childCenter;
//
//            parent = 0;
//            continue;
//          }

          // else: return leaf
          result->hit           = true;
          result->nodeIndex     = leafIndex;
          result->depth         = scaleMax-scale;
          result->t             = parent->parentTMin;
          result->numWhileLoops = whileCounter;
          result->nodeCenter    = childCenter;
          result->quality       = scale_exp2-(tScaleRatio*tcMin);
          return true;
        } else {
          // TERMINATE if voxel is small enough
          if (tScaleRatio*tcMin > scale_exp2 || scaleMax-scale == MAX_SVO_RAYCAST_DEPTH) {
            unsigned returnchildIdx = parent->parentNodeIndex + getNthchildIdx(svo[parent->parentNodeIndex]._masks,
                                      svo[parent->parentNodeIndex]._firstchildIndex,
                                      childIdx);
            result->hit           = true;
            result->nodeIndex     = returnchildIdx;
            result->depth         = scaleMax-scale+1;
            result->t             = parent->parentTMin;
            result->numWhileLoops = whileCounter;
            result->nodeCenter    = childCenter;
            result->quality       = 0.0f;
            return true;
          }

          // update parent befor push
          parent->parentTMin = tcMax;

          // ### PUSH
          --scale;
          stack[scale].parentNodeIndex = parent->parentNodeIndex + getNthchildIdx(svo[parent->parentNodeIndex]._masks, // <- relative indexing
                                         svo[parent->parentNodeIndex]._firstchildIndex,
                                         childIdx);
          stack[scale].parentTMin      = tcMin;
          stack[scale].parentTMax      = tcMax;
          stack[scale].parentCenter    = childCenter;
          continue;
        }
      } else {
        // ### ADVANCE
        parent->parentTMin = tcMax;
      }
    } else {
      // POP
      ++scale;
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
  const float value =  1.0f - result->numWhileLoops/200.0f;

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
  const float value = result->depth/(float)(MAX_STACK_SIZE);
  return (float4)( value*result->hit,
                   value*result->hit,
                   value*result->hit, 1.0f);
}


///////////////////////////////////////////////////////////////////////////////


inline float4
shade_phong(const SampleResult* result,
            float3              rayDirection,
            __global const SvoNode* svo,
            __global Attribs* attribs)
{
  if (result->hit) {
    float3 normal = getNormal(result->nodeIndex, attribs);
    normal = normalize(normal);
    float4 color  = getColor(result->nodeIndex, attribs);

    // light
    const float3 lightDirection = normalize((float3)(25.5f, 55.0f, 25.0f) - result->nodeCenter);

    // diffuse
    float nDotL = max(0.0f, dot(normal, lightDirection));

    // specular
    float3 reflectionDir = normalize( ( ( 2.0 * normal ) * nDotL ) - lightDirection );
    float reflectDotView = max( 0.0, dot( reflectionDir,  -rayDirection) );
    float specular = max(0.0f, pow(reflectDotView, 100.0f));



#if 0
    SampleResult resultShadow;
    float3 shadowRayOrigin = result->nodeCenter + normal*0.01f;
    float shadow = sample( svo,
                           shadowRayOrigin, lightDirection,
                           0.001f,
                           &resultShadow);
#else
    float shadow = 0.0f;
#endif

    color =   color*0.1
            + color*nDotL*(1.0f-shadow)
            + color*specular;

    color.w = 1.0f;

    return color;
  }

  return (float4)( 0.2f,
                   0.3f,
                   0.2f,
                   1.0f);
}


///////////////////////////////////////////////////////////////////////////////

//
//inline float4
//shade_diffuse_color_reflection(const SampleResult* result,
//                               __global const SvoNode* svo,
//                               __global unsigned* attribIndices,
//                               __global Attribs* attribs,
//                               const float3 rayDirection,
//                               int          numReflections)
//{
//
//  if (result->hit) {
//    SampleResult refectResult = *result;
//
//    float4 resultcolor   = (float4)(0.0f,0.0f,0.0f,0.0f);
//    float3 viewDirection = rayDirection;
//
//    int reflectCounter = numReflections;
//
//    float strength = 0.75f;
//
//    while (true) {
//      // color and normal for current sample
//      float3 normal = getNormal(refectResult.attribIndex, attribs);
//
//      resultcolor = resultcolor * (1.0f-strength) + shade_diffuse_color_shadow(&refectResult,
//                    svo,
//                    attribIndices,
//                    attribs)*strength;
//
//      strength *= 0.75f;
//
//      if (reflectCounter == 0 || !refectResult.hit) {
//        break;
//      } else {
//
//        const float3 reflectDir    = normalize(viewDirection + ( -2.0f * normal * dot( normal, viewDirection )));
//        float3       reflectOrigin = refectResult.nodeCenter  + normal*0.01f;
//
//        sample( svo,
//                reflectOrigin, reflectDir,
//                0.001f,
//                &refectResult);
//
//        viewDirection = reflectDir;
//
//        --reflectCounter;
//      }
//    }
//    return resultcolor;
//  }
//  return (float4)( 0.2f,
//                   0.3f,
//                   0.2f,
//                   1.0f);
//}


///////////////////////////////////////////////////////////////////////////////


#define Gamma          0.80
#define IntensityMax 255.0f

//double round(double d)
//{
//  return floor(d + 0.5);
//}

unsigned
Adjust(const float Color, const float Factor)
{
  if (Color == 0.0f) {
    return 0;
  } else {
    int res = round(IntensityMax * pow(Color * Factor, Gamma));
    return min(255, max(0, res));
  }
}



float3
WavelengthToRGB(const float Wavelength)
{
  float Blue;
  float factor;
  float Green;
  float Red;

  if(380.0f <= Wavelength && Wavelength <= 440.0f) {
    Red   = -(Wavelength - 440.0f) / (440.0f - 380.0f);
    Green = 0.0f;
    Blue  = 1.0f;
  } else if(440.0f < Wavelength && Wavelength <= 490.0f) {
    Red   = 0.0f;
    Green = (Wavelength - 440.0f) / (490.0f - 440.0f);
    Blue  = 1.0f;
  } else if(490.0f < Wavelength && Wavelength <= 510.0f) {
    Red   = 0.0f;
    Green = 1.0f;
    Blue  = -(Wavelength - 510.0f) / (510.0f - 490.0f);
  } else if(510.0f < Wavelength && Wavelength <= 580.0f) {
    Red   = (Wavelength - 510.0f) / (580.0f - 510.0f);
    Green = 1.0f;
    Blue  = 0.0f;
  } else if(580.0f < Wavelength && Wavelength <= 645.0f) {
    Red   = 1.0f;
    Green = -(Wavelength - 645.0f) / (645.0f - 580.0f);
    Blue  = 0.0;
  } else if(645.0f < Wavelength && Wavelength <= 780.0f) {
    Red   = 1.0f;
    Green = 0.0f;
    Blue  = 0.0f;
  } else {
    Red   = 0.0f;
    Green = 0.0f;
    Blue  = 0.0f;
  }
  if(380.0f <= Wavelength && Wavelength <= 420.0f) {
    factor = 0.3f + 0.7f*(Wavelength - 380.0f) / (420.0f - 380.0f);
  } else if(420.0f < Wavelength && Wavelength <= 701.0f) {
    factor = 1.0f;
  } else if(701.0f < Wavelength && Wavelength <= 780.0f) {
    factor = 0.3f + 0.7f*(780.0f - Wavelength) / (780.0f - 701.0f);
  } else {
    factor = 0.0f;
  }

  return (float3)(Adjust(Red,   factor), Adjust(Green, factor), Adjust(Blue,  factor));
}




float GetWaveLengthFromDataPoint(float Value, float MinValue, float MaxValue)
{
  const float MinVisibleWavelength = 380.0f;//350.0;
  const float MaxVisibleWavelength = 780.0f;//650.0;
  //Convert data value in the range of MinValues..MaxValues to the
  //range 350..650
  return (Value - MinValue) / (MaxValue-MinValue) * (MaxVisibleWavelength - MinVisibleWavelength) + MinVisibleWavelength;
}


float3
DataPointToColor(float Value, float MinValue, float MaxValue)
{
  float Wavelength = GetWaveLengthFromDataPoint(Value, MinValue, MaxValue);
  float3 rgb       = WavelengthToRGB(Wavelength);

  const float fac = 1.0f/255.0f;
  return (float3)(rgb.x * fac, rgb.y * fac, rgb.z * fac);
}




///////////////////////////////////////////////////////////////////////////////


__kernel void
renderToBuffer ( __write_only image2d_t renderbuffer,
                 __global SvoNode*      svo,
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
  result.quality = 0.0f;

  // primary ray
  sample( svo,
          cameraPosition.xyz, rayDirection.xyz,
          frameBufferSize.z,
          &result); // wich is the tScaleRatio

  switch ((int)otherParams.x) { // rendermode select
  case 0:
    if (result.hit)
    {
      write_imagef ( renderbuffer,
                     (int2)(x,y),
                     (float4)(getColor(result.nodeIndex, attribs).xyz, 1.0f));
      return;
    }
    write_imagef (renderbuffer,
                  (int2)(x,y),
                  (float4)(0.2f,0.3f,0.2f,1.0f));
    break;
    case 1:

          if (result.hit)
          {
            write_imagef ( renderbuffer,
                          (int2)(x,y),
                          (float4)(getNormal(result.nodeIndex, attribs), 1.0f));
            return;
          }

           write_imagef ( renderbuffer,
                          (int2)(x,y),
                          (float4)(0.2f,0.3f,0.2f,1.0f));
        break;

  case 2:
    {
      float3 color = DataPointToColor(result.numWhileLoops, 0.0f, MAX_STACK_SIZE*30);
      write_imagef ( renderbuffer,
                     (int2)(x,y),
                     (float4)(color.x, color.y, color.z, 1.0f));
    }
    break;

  case 3:
    write_imagef ( renderbuffer,
                   (int2)(x,y),
                   shade_svoDepth(&result));
    break;

  case 4:
    write_imagef ( renderbuffer,
                   (int2)(x,y),
                   shade_pixelDepth(&result));
    break;

  case 5:
    write_imagef ( renderbuffer,
                   (int2)(x,y),
                   shade_iterationDepth(&result));
    break;

  case 6:
    write_imagef ( renderbuffer,
                   (int2)(x,y),
                   shade_phong(&result,
                               rayDirection.xyz,
                               svo,
                               attribs));
    break;

  case 7:
    {
      if (result.hit)
      {
//        if (result.quality > 0.0f)
//        {
//          float3 color = DataPointToColor(result.quality, 0.0f, 0.01);
          result.quality = sqrt(sqrt(result.quality));
          write_imagef ( renderbuffer,
                         (int2)(x,y),
                         (float4)(result.quality,
                                  result.quality,
                                  result.quality,
                                  1.0f));
          return;
//        }
//        else // no quality
//        {
//          write_imagef ( renderbuffer,
//                   (int2)(x,y),
//                   (float4)(0.4,0.4,0.4, 1.0f));
//        }

      }
      else // bg
      {
        write_imagef ( renderbuffer,
                 (int2)(x,y),
                 (float4)(0.6f,0.3f,0.3f, 1.0f));
      }
    }
    break;
  }
}


///////////////////////////////////////////////////////////////////////////////




















///////////////////////////////////////////////////////////////////////////////

//
typedef struct
{
  int   _nodePosOrTreeletGid;
  int   _isLeafeWithSubtreelet;
  float _errorIfLeafe;
  float _quality2;
} __attribute__ ( ( aligned ( 16 ) ) ) FeedBackDataElement;


//
typedef struct
{
  int      _isLeafeWithSubtreelet;
  int      _nodePosOrTreeletGid;
  float    _errorIfLeafe;
  float    _quality2;
}/*__attribute__ ( ( aligned ( 16 ) ) )*/ FeedBackDataSample;


///////////////////////////////////////////////////////////////////////////////



bool
sampleAnalyse( __global const SvoNode* svo,
               float3 rayOrigin,
               float3 rayDirection,
               const float tScaleRatio,
               FeedBackDataSample* sampleResult)
{
  float tMin,tMax;
  if (!intersectAABB( rayOrigin, rayDirection, &tMin, &tMax)) {
    return false;
  }

  const int   scaleMax   = MAX_STACK_SIZE;
  int         scale      = scaleMax-1;
  float       scale_exp2 = 0.5f;// exp2f(scale - s_max)
  const float minNormal  = 0.0001f;
  const float epsilon    = 0.0001f;

  private StackElement stack[MAX_STACK_SIZE];

  // init stack with root node
  stack[scale].parentNodeIndex = 0;
  stack[scale].parentTMin      = tMin;
  stack[scale].parentTMax      = tMax;
  stack[scale].parentCenter    = (float3)(0.0f,0.0f,0.0f);

  if ( fabs(rayDirection.x) < minNormal) {
    rayDirection.x = minNormal * sign(rayDirection.x);
  }
  if ( fabs(rayDirection.y) < minNormal) {
    rayDirection.y = minNormal * sign(rayDirection.y);
  }
  if ( fabs(rayDirection.z) < minNormal) {
    rayDirection.z = minNormal * sign(rayDirection.z);
  }
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
  const unsigned maxLoops     = (scaleMax+1)*(scaleMax+1)*5;

/////////////////// LOOP ///////////////////////////////XS

  while (scale < scaleMax) {
    ++whileCounter;

    if (whileCounter == maxLoops) {
      break;
    }

//    if (!parent)
//    {
    parent = &stack[scale];
    scale_exp2       = pow(2.0f, scale - scaleMax);
    childSizeHalf    = scale_exp2*0.5f;


//    }

    // ### POP if parent is behind the camera
    if ( parent->parentTMax < 0.0f) {
      ++scale;
      parent = 0;
      continue;
    }


    if (fabs(parent->parentTMin - parent->parentTMax) > epsilon)
    {
      // childEntryPoint in parent voxel coordinates
      float3 childEntryPoint = (rayOrigin + (parent->parentTMin + epsilon) * rayDirection) - parent->parentCenter;
      int childIdx           =  (int) (     4*(childEntryPoint.x > 0.0f)
                                          + 2*(childEntryPoint.y > 0.0f)
                                          +   (childEntryPoint.z > 0.0f));

      // childCenter in world coordinates
      float3 childCenter = (float3)(-0.5f + (bool)(childIdx & 4),
                                    -0.5f + (bool)(childIdx & 2),
                                    -0.5f + (bool)(childIdx & 1))*scale_exp2 + parent->parentCenter;

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
      if (getValidMaskFlag(svo[parent->parentNodeIndex]._masks, childIdx))
      {
        // TERMINATE if voxel is a leaf
        if (getLeafMaskFlag(svo[parent->parentNodeIndex]._masks, childIdx))
        {
          unsigned leafIndex = parent->parentNodeIndex + getNthchildIdx( svo[parent->parentNodeIndex]._masks,
                               svo[parent->parentNodeIndex]._firstchildIndex,
                               childIdx);

          /*
            Two possibilities:
            1. This "leafe" has a firstChildIndex than it has a subtreelet and
               the firtChildIndex is the Gid of that subtreelet
            2. This leafe is a leafe of ther whole svo and has no subtreelet
          */

          sampleResult->_errorIfLeafe = scale_exp2-(tScaleRatio*tcMin);

          if (svo[leafIndex]._firstchildIndex)
          {
            // ### WRITE required Treelet Gid encoded in the first child
            sampleResult->_isLeafeWithSubtreelet = true;
            sampleResult->_nodePosOrTreeletGid   = svo[leafIndex]._firstchildIndex; // <- which is the Treelet Gid of the child Treelet
          }
          else
          {
            sampleResult->_isLeafeWithSubtreelet = false;
            sampleResult->_nodePosOrTreeletGid   = leafIndex;
          }

          return true;
        }
        else
        {
          // TERMINATE if voxel is small enough
          if (tScaleRatio*tcMin > scale_exp2 || scaleMax-scale == MAX_SVO_RAYCAST_DEPTH)
          {
            unsigned returnchildIndex = parent->parentNodeIndex + getNthchildIdx(svo[parent->parentNodeIndex]._masks,
                                                                                 svo[parent->parentNodeIndex]._firstchildIndex,
                                                                                 childIdx);
            sampleResult->_isLeafeWithSubtreelet            = false;
            sampleResult->_nodePosOrTreeletGid = returnchildIndex;
//            sampleResult->_errorIfLeafe      = 0.0f;//tScaleRatio*tcMin / scale_exp2;
            return true;
          }

          // update parent befor push
          parent->parentTMin = tcMax;

          // ### PUSH
          --scale;
          stack[scale].parentNodeIndex = parent->parentNodeIndex + getNthchildIdx(svo[parent->parentNodeIndex]._masks, // <- relative indexing
                                         svo[parent->parentNodeIndex]._firstchildIndex,
                                         childIdx);
          stack[scale].parentTMin      = tcMin;
          stack[scale].parentTMax      = tcMax;
          stack[scale].parentCenter    = childCenter;

          parent = 0;
          continue;
        }
      } else {
        // ### ADVANCE
        parent->parentTMin = tcMax;
      }
    } else {
      // POP
      ++scale;
      parent = 0;
      continue;
    }

  } // while


  return false;
}


///////////////////////////////////////////////////////////////////////////////



__kernel void
renderToFeedbackBuffer ( __global FeedBackDataElement* feedbackBuffer,
                         __global SvoNode*     svo,
                         const float4          frameBufferSize,
                         const float4          pixelFarVectorH,
                         const float4          pixelFarVectorV,
                         const float4          far_lower_left,
                         const float4          cameraPosition)
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

  FeedBackDataSample result;
  result._isLeafeWithSubtreelet = 0;
  result._nodePosOrTreeletGid   = 0;
  result._errorIfLeafe          = 0.0f;


  // primary ray
  sampleAnalyse(  svo,
                  cameraPosition.xyz, rayDirection.xyz,
                  frameBufferSize.z,
                  &result);

  FeedBackDataElement feedBackElemet;
  feedBackElemet._nodePosOrTreeletGid    = result._nodePosOrTreeletGid;
  feedBackElemet._isLeafeWithSubtreelet  = result._isLeafeWithSubtreelet;
  feedBackElemet._errorIfLeafe           = result._errorIfLeafe;
  feedBackElemet._quality2               = 1.0f;

  const unsigned frameBufferPosition = (unsigned)(x + frameBufferSize.x*y);
  feedbackBuffer[frameBufferPosition] = feedBackElemet;
}



///////////////////////////////////////////////////////////////////////////////


