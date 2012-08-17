
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



/// gloost system includes
#include <CpuRaycasterSingleRay3.h>
#include <SvoVisualizer.h>



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
  _max_stack_size(0),
  _stack(),
  _tScaleRatio(1.0),
  _tMin(0),
  _tMax(0),
  _svo(0),
  _idToPositionLookUp(8),
  _verboseMode(verboseMode)
{
  _idToPositionLookUp[0] = gloost::Point3(-0.5, -0.5, -0.5);
  _idToPositionLookUp[1] = gloost::Point3(-0.5, -0.5,  0.5);
  _idToPositionLookUp[2] = gloost::Point3(-0.5,  0.5, -0.5);
  _idToPositionLookUp[3] = gloost::Point3(-0.5,  0.5,  0.5);
  _idToPositionLookUp[4] = gloost::Point3( 0.5, -0.5, -0.5);
  _idToPositionLookUp[5] = gloost::Point3( 0.5, -0.5,  0.5);
  _idToPositionLookUp[6] = gloost::Point3( 0.5,  0.5, -0.5);
  _idToPositionLookUp[7] = gloost::Point3( 0.5,  0.5,  0.5);
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



/**
  \brief traverses a Svo with a single ray
  \param ...
  \remarks ...
*/

bool
CpuRaycasterSingleRay3::start(const gloost::Ray& ray,
                              float tScaleRatio,
                              Svo* svo,
                              ResultStruct& result)
{
  _tScaleRatio = tScaleRatio;
  _svo         = svo;

  static gloost::BoundingBox svoBoundingBox(gloost::Point3(-0.5,-0.5,-0.5),
                                            gloost::Point3(0.5,0.5,0.5));

//  std::cerr << std::endl << "CpuRaycastStackElement: " << sizeof(CpuRaycastStackElement);

  gloost::mathType tMin, tMax;

  if (svoBoundingBox.intersect(ray, tMin, tMax))
  {
    return traversSvo(ray.getOrigin(),
                      ray.getDirection(),
                      tMin, tMax,
                      result);
  }
  else
  {
	  return false;
  }
	return false;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief traverses the Svo and returns the last valid hit child
  \param ...
  \remarks ...
*/

bool
CpuRaycasterSingleRay3::traversSvo( gloost::Point3 origin,
                                    gloost::Vector3 direction,
                                    float tMin,
                                    float tMax,
                                    ResultStruct& result)
{
  // STACK INIT
  static const int scaleMax   = _svo->getMaxDepth();
  int              scale      = scaleMax-1;
  float            scale_exp2 = 0.5f;// exp2f(scale - s_max)

  _stack.resize(scaleMax+1);

  CpuRaycastStackElement element;
  element.parentNodeIndex = 0;
  element.parentTMin      = tMin;
  element.parentTMax      = tMax;
  element.parentCenter    = gloost::Point3(0.0, 0.0, 0.0);
//  element.nextChild    = 0;

  _stack[scale] = element;


  // precalculate ray coefficients, tx(x) = "(1/dx)"x + "(-px/dx)"
  static float epsilon = 0.00001;
//  static float epsilon = pow(2, -(scaleMax+2));
//  static float epsilon = pow(2, (_svo->getMaxDepth()+1.0)*-1.0);

  if ( gloost::abs(direction[0]) < epsilon) direction[0] = epsilon * gloost::sign(direction[0]);
  if ( gloost::abs(direction[1]) < epsilon) direction[1] = epsilon * gloost::sign(direction[1]);
  if ( gloost::abs(direction[2]) < epsilon) direction[2] = epsilon * gloost::sign(direction[2]);


  gloost::mathType dxReziprok = 1.0/direction[0];
  gloost::mathType minusPx_dx = -origin[0]*dxReziprok;

  gloost::mathType dyReziprok = 1.0/direction[1];
  gloost::mathType minusPy_dy = -origin[1]*dyReziprok;

  gloost::mathType dzReziprok = 1.0/direction[2];
  gloost::mathType minusPz_dz = -origin[2]*dzReziprok;


  gloost::mathType x0;
  gloost::mathType x1;
  gloost::mathType y0;
  gloost::mathType y1;
  gloost::mathType z0;
  gloost::mathType z1;

  gloost::mathType tx0;
  gloost::mathType tx1;

  gloost::mathType ty0;
  gloost::mathType ty1;

  gloost::mathType tz0;
  gloost::mathType tz1;

  gloost::mathType tcMin;
  gloost::mathType tcMax;

  gloost::Point3 childEntryPoint;
  unsigned childIndex;

  gloost::mathType childSizeHalf;

  CpuRaycastStackElement* parent = 0;
  bool refetchParent             = true;

  unsigned         parentNodeIndex;
  gloost::mathType parentTMin;
  gloost::mathType parentTMax;
  gloost::Point3   parentCenter;

  unsigned        whileCounter = 0;
  static unsigned maxLoops     = (_svo->getMaxDepth()+1)*(_svo->getMaxDepth()+1)*5.0;

  std::vector<CpuSvoNode>& svoSerialized =  _svo->getSerializedNodes();


  /////////////////// LOOP ///////////////////////////////
  while (scale < scaleMax)
  {
    ++whileCounter;

    if (whileCounter == maxLoops)
    {
      std::cerr << std::endl << "ERROR MAX LOOPS REACHED: " << maxLoops;
      std::cerr << std::endl << "refetchParent:           " << refetchParent;
      std::cerr << std::endl;
      return 0;
    }

    if (refetchParent)
    {
      parent = &_stack[scale];

      parentNodeIndex = parent->parentNodeIndex;
      parentTMin      = parent->parentTMin;
      parentTMax      = parent->parentTMax;
      parentCenter    = parent->parentCenter;

      /// hier den Punkt tMin vom parent benutzen um einstiegskind zu bekommen
      /// x0,x1,y0,y1,z0,z1 für das einstiegskind setzen.
      /// dann mit tx1, ty1 und tz1 < tcmax die folgekinder bestimmen
      scale_exp2       = pow(2, scale - scaleMax);
//      childOffsetScale = scale_exp2;
      childSizeHalf    = scale_exp2*0.5;
      refetchParent    = false;
    }


    if ( parentTMax < 0.0)
    {
      // POP
      ++scale;
      refetchParent = true;
      continue;
    }

    if ( std::abs(parentTMin - parentTMax) < epsilon)
//    if ( tcMin >= parentTMax)
    {
      // POP
      ++scale;
      refetchParent = true;
      continue;
    }


    if (parentTMin < parentTMax)
    {
      // childEntryPoint in parent voxel coordinates
      childEntryPoint = (origin + (parentTMin + epsilon) * direction) - parentCenter;
      childIndex      =   4*(childEntryPoint[0] > 0)
                        + 2*(childEntryPoint[1] > 0)
                        +   (childEntryPoint[2] > 0);


      // childCenter in world coordinates
      gloost::Point3 childCenter = _idToPositionLookUp[childIndex]*scale_exp2 + parentCenter;

      x0 = childCenter[0] - childSizeHalf;
      x1 = childCenter[0] + childSizeHalf;
      y0 = childCenter[1] - childSizeHalf;
      y1 = childCenter[1] + childSizeHalf;
      z0 = childCenter[2] - childSizeHalf;
      z1 = childCenter[2] + childSizeHalf;


      // tx(x) = (1/dx)x + (-px/dx)
      // dx...Direction of the Ray in x
      // px...Origin of the Ray in x
      tx0 = dxReziprok*x0 + minusPx_dx;
      tx1 = dxReziprok*x1 + minusPx_dx;
      gloost::swapToIncreasing(tx0, tx1);

      ty0 = dyReziprok*y0 + minusPy_dy;
      ty1 = dyReziprok*y1 + minusPy_dy;
      gloost::swapToIncreasing(ty0, ty1);

      tz0 = dzReziprok*z0 + minusPz_dz;
      tz1 = dzReziprok*z1 + minusPz_dz;
      gloost::swapToIncreasing(tz0, tz1);


      tcMin = gloost::max(tx0, ty0, tz0); // <- you can only enter once
      tcMax = gloost::min(tx1, ty1, tz1); // <- you can only leave once


//      if (childIndex>7)
//      {
//        std::cerr << std::endl << "!!!!!! childIndex > 7: ";
//        return 0;
//      }


      // handle children
      if (svoSerialized[parentNodeIndex].getValidMask().getFlag(childIndex))
      {
        if (svoSerialized[parentNodeIndex].getLeafMask().getFlag(childIndex))
        {
          unsigned returnChildIndex = svoSerialized[parentNodeIndex].getNthChildIndex(childIndex);

          result.hit         = true;
          result.nodeIndex   = returnChildIndex;
          result.attribIndex = svoSerialized[returnChildIndex].getAttribPosition();
          result.depth       = scaleMax-scale;
          result.t           = parentTMin;

          return true;
        }
        else
        {
          if (_tScaleRatio*tcMax > scale_exp2)
          {
            unsigned returnChildIndex = svoSerialized[parentNodeIndex].getNthChildIndex(childIndex);

            result.hit         = true;
            result.nodeIndex   = returnChildIndex;
            result.attribIndex = svoSerialized[returnChildIndex].getAttribPosition();
            result.depth       = scaleMax-scale;
            result.t           = parentTMin;

            return true;
          }

          // update parent befor push
          _stack[scale].parentTMin = tcMax;

          // PUSH
          CpuRaycastStackElement newStackElement;
          newStackElement.parentNodeIndex = svoSerialized[parentNodeIndex].getNthChildIndex(childIndex);
          newStackElement.parentTMin      = tcMin;
          newStackElement.parentTMax      = tcMax;
          newStackElement.parentCenter    = childCenter;

          --scale;
          _stack[scale] = newStackElement;

          refetchParent = true;
          continue;
        }
      }
      else
      {
        // ADVANCE
        parentTMin = tcMax;
      }
    }
    else
    {
      // POP
      ++scale;
      refetchParent = true;
      continue;
    }

  }


  return 0;
}



////////////////////////////////////////////////////////////////////////////////





} // namespace svo


