
/*
                       ___                            __
                      /\_ \                          /\ \__
                   __ \//\ \     ___     ___     ____\ \  _\
                 /'_ `\ \ \ \   / __`\  / __`\  /  __\\ \ \/
                /\ \ \ \ \_\ \_/\ \ \ \/\ \ \ \/\__   \\ \ \_
                \ \____ \/\____\ \____/\ \____/\/\____/ \ \__\
                 \/___/\ \/____/\/___/  \/___/  \/___/   \/__/
                   /\____/
                   \_/__/

                   OpenGL framework for fast demo programming

                             http://www.gloost.org

    This file is part of the gloost framework. You can use it in parts or as
       whole under the terms of the GPL (http://www.gnu.org/licenses/#GPL).

            gloost is being created by Felix Weißig and Stephan Beck

     Felix Weißig (thesleeper@gmx.net), Stephan Beck (stephan@pixelstars.de)
*/



/// svo system includes
#include <CpuRaycasterSingleRay3.h>
#include <SvoVisualizer.h>
#include <CpuSvoNode.h>


/// gloost system includes
#include <gloost/gloostHelper.h>



/// cpp includes
#include <string>
#include <iostream>
#include <map>
#include <list>



namespace svo
{

/**
  \class CpuRaycasterSingleRay3

  \brief Cpu raycaster performs raycasting on an svo::Svo with a single ray

  \author Felix Weiszig
  \date   December 2009
  \remarks
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

CpuRaycasterSingleRay3::CpuRaycasterSingleRay3(bool verboseMode):
  _svo(0),
  _pushCounter(0),
  _popCounter(0),
  _whileCounter(0)
//  _stack(),
//  _tMin(0),
//  _tMax(0),
//  _svo(0),
//  _idToPositionLookUp(8),
//  _verboseMode(verboseMode)
{

}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

CpuRaycasterSingleRay3::~CpuRaycasterSingleRay3()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


//#define TEXTOUTPUTWHILETRAVERSAL

/**
  \brief traverses a Svo with a single ray
  \param ...
  \remarks ...
*/

CpuSvoNode
CpuRaycasterSingleRay3::start(const gloost::Ray& ray, Svo* svo)
{
  _svo = svo;

//  while (_stack.size())
//  {
//    _stack.pop();
//  }


  return traversSvo(svo->getRootNode(),
                    gloost::Point3(ray.getOrigin()),
                    gloost::Vector3(ray.getDirection()));
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief traverses the Svo and returns the last valid hit child
  \param ...
  \remarks ...
*/

CpuSvoNode
CpuRaycasterSingleRay3::traversSvo(SvoNode* rootNode, gloost::Point3 p, gloost::Vector3 d)
{
  const int   s_max   = 23;               // Maximum scale
  const float epsilon = pow(2.0, -s_max);

  Stack stack(s_max+1);

  // Get rid of small ray direction components to avoid division by zero.
//  if (std::abs(d[0]) < epsilon) d[0] = copysignf(epsilon, d[0]);
//  if (std::abs(d[1]) < epsilon) d[1] = copysignf(epsilon, d[1]);
//  if (std::abs(d[2]) < epsilon) d[2] = copysignf(epsilon, d[2]);

  if (std::abs(d[0]) < epsilon) d[0] = epsilon * gloost::sign(d[0]);
  if (std::abs(d[1]) < epsilon) d[1] = epsilon * gloost::sign(d[1]);
  if (std::abs(d[2]) < epsilon) d[2] = epsilon * gloost::sign(d[2]);


  // Precompute the coefficients of tx(x), ty(y), and tz(z).
  // The octree is assumed to reside at coordinates [1, 2].

  float tx_coef = 1.0f / -std::abs(d[0]);
  float ty_coef = 1.0f / -std::abs(d[1]);
  float tz_coef = 1.0f / -std::abs(d[2]);

  float tx_bias = tx_coef * p[0];
  float ty_bias = ty_coef * p[1];
  float tz_bias = tz_coef * p[2];


  // Select octant mask to mirror the coordinate system so
  // that ray direction is negative along each axis.

  int octant_mask = 7;
  if (d[0] > 0.0f) octant_mask ^= 1, tx_bias = 3.0f * tx_coef - tx_bias;
  if (d[1] > 0.0f) octant_mask ^= 2, ty_bias = 3.0f * ty_coef - ty_bias;
  if (d[2] > 0.0f) octant_mask ^= 4, tz_bias = 3.0f * tz_coef - tz_bias;

//  std::cerr << std::endl << "word: " << gloost::unsignedToBinaryString(octant_mask);


  // Initialize the active span of t-values.

  float t_min = gloost::max(gloost::max(2.0f * tx_coef - tx_bias, 2.0f * ty_coef - ty_bias), 2.0f * tz_coef - tz_bias);
  float t_max = gloost::min(gloost::min(tx_coef - tx_bias, ty_coef - ty_bias), tz_coef - tz_bias);
  float h     = t_max;
  t_min       = gloost::max(t_min, 0.0f);
  t_max       = gloost::min(t_max, 1.0f);


  // Initialize the current voxel to the first child of the root.

  CpuSvoNode parent   = _svo->getSerializedNodeForIndex(0);
  // int2 childDescriptor = make_int2(0, 0); // invalid until fetched
  int idx             = 0;
  gloost::Vector3 pos = gloost::Vector3(1.0,1.0,1.0);
  int scale           = s_max - 1;

//  std::cerr << std::endl << "scale: " << scale;

  float scale_exp2    = 0.5f;// exp2f(scale - s_max)

  if (1.5f * tx_coef - tx_bias > t_min) idx ^= 1, pos[0] = 1.5f;
  if (1.5f * ty_coef - ty_bias > t_min) idx ^= 2, pos[1] = 1.5f;
  if (1.5f * tz_coef - tz_bias > t_min) idx ^= 4, pos[2] = 1.5f;

  unsigned itCounter = 0;


  while (scale < s_max/* && itCounter == 0*/)
  {

    // Fetch child descriptor unless it is already valid.
//    if (child_descriptor.x == 0)
//    {
//      child_descriptor = *(int2*)parent;   /// !!!!!!!!!!
//    }

    // Determine maximum t-value of the cube by evaluating
    // tx(), ty(), and tz() at its corner.

    float tx_corner = pos[0] * tx_coef - tx_bias;
    float ty_corner = pos[1] * ty_coef - ty_bias;
    float tz_corner = pos[2] * tz_coef - tz_bias;
    float tc_max = gloost::min(gloost::min(tx_corner, ty_corner), tz_corner);


    // Process voxel if the corresponding bit in valid mask is set
    // and the active t-span is non-empty.

    int child_shift = idx ^ octant_mask; // permute child slots based on the mirroring
//    int child_masks = child_descriptor.x << child_shift;

    int child_masks = parent.getValidMask().getValue();
    child_masks = child_masks << 8;
    child_masks |= parent.getLeafMask().getValue();
//    child_masks = child_masks << child_shift;

//    std::cerr << std::endl << "t_min: " << t_min;
//    std::cerr << std::endl << "t_max: " << t_max;
//    std::cerr << std::endl;

    std::cerr << std::endl << "wanna PUSH0: ";
    std::cerr << std::endl << "t_min: " << t_min;
    std::cerr << std::endl << "t_max: " << t_max;
//    std::cerr << std::endl << gloost::unsignedToBinaryString(child_masks);

    if ((child_masks & 0x8000) != 0 && t_min <= t_max)
    {

      // Terminate if the voxel is small enough.

//      updateCounter(PerfCounter_Instructions, 3);
//      if (tc_max * ray.dir_sz + ray_orig_sz >= scale_exp2)
//          break; // at t_min


      // INTERSECT
      // Intersect active t-span with the cube and evaluate
      // tx(), ty(), and tz() at the center of the voxel.

      float tv_max = gloost::min(t_max, tc_max);
      float half = scale_exp2 * 0.5f;
      float tx_center = half * tx_coef + tx_corner;
      float ty_center = half * ty_coef + ty_corner;
      float tz_center = half * tz_coef + tz_corner;


        std::cerr << std::endl << "wanna PUSH1: ";

      // Descend to the first child if the resulting t-span is non-empty.
      if (t_min <= tv_max)
      {
        std::cerr << std::endl << "wanna PUSH2: ";

          // Terminate if the corresponding bit in the non-leaf mask is not set.

          if ((child_masks & 0x0080) == 0)
              break; // at t_min (overridden with tv_min).

          // PUSH
          // Write current parent to the stack.

//#ifndef DISABLE_PUSH_OPTIMIZATION
//          if (tc_max < h)
//#endif
          {
            std::cerr << std::endl << "PUSH: ";
            stack.write(scale, parent, t_max);
          }
          h = tc_max;


//          // Find child descriptor corresponding to the current voxel.
//
//          int ofs = (unsigned int)child_descriptor.x >> 17; // child pointer
////          if ((child_descriptor.x & 0x10000) != 0) // far
////          {
////              ofs = parent[ofs * 2]; // far pointer
////              updateCountersForGlobalAccess(2, &parent[ofs * 2]);
////          }
//          ofs    += popc8(child_masks & 0x7F);
//          parent += ofs * 2;
        parent = _svo->getSerializedNodeForIndex(parent.getFirstChildIndex());

        // Select child voxel that the ray enters first.

        idx = 0;
        scale--;
        scale_exp2 = half;

        if (tx_center > t_min) idx ^= 1, pos[0] += scale_exp2;
        if (ty_center > t_min) idx ^= 2, pos[1] += scale_exp2;
        if (tz_center > t_min) idx ^= 4, pos[2] += scale_exp2;

        // Update active t-span and invalidate cached child descriptor.
        t_max = tv_max;
        continue;
      }
    }



    // ADVANCE
    // Step along the ray.

    int step_mask = 0;
    if (tx_corner <= tc_max) step_mask ^= 1, pos[0] -= scale_exp2;
    if (ty_corner <= tc_max) step_mask ^= 2, pos[1] -= scale_exp2;
    if (tz_corner <= tc_max) step_mask ^= 4, pos[2] -= scale_exp2;

    // Update active t-span and flip bits of the child slot index.

    t_min = tc_max;
    idx  ^= step_mask;

    // Proceed with pop if the bit flips disagree with the ray direction.

    if ((idx & step_mask) != 0)
    {
      // POP
      // Find the highest differing bit between the two positions.

      unsigned int differing_bits = 0;
      if ((step_mask & 1) != 0) differing_bits |= gloost::float_as_int(pos[0]) ^ gloost::float_as_int(pos[0] + scale_exp2);
      if ((step_mask & 2) != 0) differing_bits |= gloost::float_as_int(pos[1]) ^ gloost::float_as_int(pos[1] + scale_exp2);
      if ((step_mask & 4) != 0) differing_bits |= gloost::float_as_int(pos[2]) ^ gloost::float_as_int(pos[2] + scale_exp2);
      scale = (gloost::float_as_int((float)differing_bits) >> 23) - 127; // position of the highest bit
      scale_exp2 = gloost::int_as_float((scale - s_max + 127) << 23); // exp2f(scale - s_max)

      // Restore parent voxel from the stack.

      parent = stack.read(scale, t_max);

      // Round cube position and extract child slot index.

      int shx = gloost::float_as_int(pos[0]) >> scale;
      int shy = gloost::float_as_int(pos[1]) >> scale;
      int shz = gloost::float_as_int(pos[2]) >> scale;
      pos[0]  = gloost::int_as_float(shx << scale);
      pos[1]  = gloost::int_as_float(shy << scale);
      pos[2]  = gloost::int_as_float(shz << scale);
      idx     = (shx & 1) | ((shy & 1) << 1) | ((shz & 1) << 2);

      // Prevent same parent from being stored again and invalidate cached child descriptor.

      h = 0.0f;
//      child_descriptor.x = 0;
    }
  }

    // Indicate miss if we are outside the octree.

//#if (MAX_RAYCAST_ITERATIONS > 0)
//    if (scale >= s_max || iter > MAX_RAYCAST_ITERATIONS)
//#else
    if (scale >= s_max)
//#endif
    {
        t_min = 2.0f;
    }

    // Undo mirroring of the coordinate system.

    if ((octant_mask & 1) == 0) pos[0] = 3.0f - scale_exp2 - pos[0];
    if ((octant_mask & 2) == 0) pos[1] = 3.0f - scale_exp2 - pos[1];
    if ((octant_mask & 4) == 0) pos[2] = 3.0f - scale_exp2 - pos[2];

    // Output results.

//    res.t = t_min;
//    res.iter = iter;
//    res.pos.x = fminf(fmaxf(ray.orig.x + t_min * ray.dir.x, pos.x + epsilon), pos.x + scale_exp2 - epsilon);
//    res.pos.y = fminf(fmaxf(ray.orig.y + t_min * ray.dir.y, pos.y + epsilon), pos.y + scale_exp2 - epsilon);
//    res.pos.z = fminf(fmaxf(ray.orig.z + t_min * ray.dir.z, pos.z + epsilon), pos.z + scale_exp2 - epsilon);
//    res.node = parent;
//    res.childIdx = idx ^ octant_mask ^ 7;
//    res.stackPtr = scale;



  return parent;
}

////////////////////////////////////////////////////////////////////////////////





























} // namespace svo


