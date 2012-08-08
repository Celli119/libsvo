
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
#include <CpuRaycasterSingleRay2.h>
#include <SvoVisualizer.h>



/// cpp includes
#include <string>
#include <iostream>
#include <map>
#include <list>



namespace svo
{

/**
  \class CpuRaycasterSingleRay2

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

CpuRaycasterSingleRay2::CpuRaycasterSingleRay2(bool verboseMode):
  _max_stack_size(0),
  _stack(),
  _currentDepth(),
  _tMin(0),
  _tMax(0),
  _svo(0),
  _idToPositionLookUp(8),
  _verboseMode(verboseMode)
{
  _idToPositionLookUp[0] = gloost::Point3(-0.25, -0.25, -0.25);
  _idToPositionLookUp[1] = gloost::Point3(-0.25, -0.25,  0.25);
  _idToPositionLookUp[2] = gloost::Point3(-0.25,  0.25, -0.25);
  _idToPositionLookUp[3] = gloost::Point3(-0.25,  0.25,  0.25);
  _idToPositionLookUp[4] = gloost::Point3( 0.25, -0.25, -0.25);
  _idToPositionLookUp[5] = gloost::Point3( 0.25, -0.25,  0.25);
  _idToPositionLookUp[6] = gloost::Point3( 0.25,  0.25, -0.25);
  _idToPositionLookUp[7] = gloost::Point3( 0.25,  0.25,  0.25);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

CpuRaycasterSingleRay2::~CpuRaycasterSingleRay2()
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

SvoNode*
CpuRaycasterSingleRay2::start(const gloost::Ray& ray, Svo* svo)
{
  _svo = svo;

  _stack.clear();

  static gloost::BoundingBox svoBoundingBox(gloost::Point3(-0.5,-0.5,-0.5),
                                            gloost::Point3(0.5,0.5,0.5));

//  std::cerr << std::endl << "CpuRaycastStackElement: " << sizeof(CpuRaycastStackElement);

  gloost::mathType tMin, tMax;

  if (svoBoundingBox.intersect(ray, tMin, tMax))
  {
    return traversSvo(ray.getOrigin(),
                      ray.getDirection(),
                      tMin, tMax);
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

SvoNode*
CpuRaycasterSingleRay2::traversSvo( gloost::Point3 origin,
                                    gloost::Vector3 direction,
                                    float tMin,
                                    float tMax)
{
  CpuRaycastStackElement element;
  element.parentNode   = _svo->getRootNode();
  element.parentTMin   = tMin;
  element.parentTMax   = tMax;
  element.parentCenter = gloost::Point3(0.0, 0.0, 0.0);

  _stack.push_back(element);


  _currentDepth = 0;


  // precalculate ray coefficients, tx(x) = "(1/dx)"x + "(-px/dx)"
  static float epsilon = 0.00001;

  if ( gloost::abs(direction[0]) < epsilon) direction[0] = epsilon;
  if ( gloost::abs(direction[1]) < epsilon) direction[1] = epsilon;
  if ( gloost::abs(direction[2]) < epsilon) direction[2] = epsilon;


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



  /////////////////// LOOP ///////////////////////////////
  while (_stack.size())
  {
//    _max_stack_size = gloost::max(_max_stack_size, (int)_stack.size());

    CpuRaycastStackElement parent = _stack[_currentDepth];

    SvoNode*         parentNode   = parent.parentNode;
    gloost::mathType parentTMin   = parent.parentTMin;
    gloost::mathType parentTMax   = parent.parentTMax;
    gloost::Point3   parentCenter = parent.parentCenter;
    int              depth        = _currentDepth + 1;
    unsigned         nextChild    = parent.nextChild;


    if (nextChild == 4 || parentTMax < 0.0)
    {
      _stack.pop_back();
      --_currentDepth;
      continue;
    }


    /// hier den Punkt tMin vom parent benutzen um einstiegskind zu bekommen
    /// x0,x1,y0,y1,z0,z1 für das einstiegskind setzen.
    /// dann mit tx1, ty1 und tz1 < tcmax die folgekinder bestimmen
    gloost::mathType parentScale   = pow(2, -depth+1);
    gloost::mathType childSizeHalf = pow(2, -depth-1);

    /* Depth = 1 (child of root)
        parentScale = 1.0;
       Depth = 2:
        parentScale = 0.5;
       Depth = 3:
        parentScale = 0.25;
    */

    //////////////////////////////////// FIRST CHILD of FOUR ////////////////////

    if (nextChild == 0 && parentTMin < parentTMax)
    {
      // in parent voxel coordinates
      childEntryPoint = (origin + parentTMin * direction) - parentCenter;
      childIndex      =   4*(childEntryPoint[0] > 0)
                        + 2*(childEntryPoint[1] > 0)
                        +   (childEntryPoint[2] > 0);


      // in world coordinates
      gloost::Point3 childCenter = _idToPositionLookUp[childIndex]*parentScale + parentCenter;

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



      // handle firstChild
      if (parentNode->getValidMask().getFlag(childIndex))
      {
        if (parentNode->getLeafMask().getFlag(childIndex))
        {
          return parentNode->getChild(childIndex);
        }
        else
        {
          CpuRaycastStackElement newStackElement;
          newStackElement.parentNode   = parentNode->getChild(childIndex);
          newStackElement.parentTMin   = tcMin;
          newStackElement.parentTMax   = tcMax;
          newStackElement.parentCenter = childCenter;
          newStackElement.nextChild    = 0;

          _stack[_currentDepth].nextChild  = 1;
          _stack[_currentDepth].parentTMin = tcMax;

          ++_currentDepth;
          _stack.push_back(newStackElement);

//          if (_currentDepth != _stack.size()-1) std::cerr << std::endl << "AHHHHH 1: " << _currentDepth << " != " << _stack.size()-1;

          continue;
        }
      }
      else
      {
        ++nextChild;
      }
    }

    //////////////////////////////////// SECOND CHILD of FOUR ////////////////////
    if(nextChild == 1 && tcMax < parentTMax)
    {
      // in parent voxel coordinates
      childEntryPoint = (origin + (tcMax + epsilon) * direction) - parentCenter;
      childIndex      =   4*(childEntryPoint[0] > 0)
                        + 2*(childEntryPoint[1] > 0)
                        +   (childEntryPoint[2] > 0);


      // in world coordinates
      gloost::Point3 childCenter = _idToPositionLookUp[childIndex]*parentScale + parentCenter;

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

      tcMin = gloost::max(tx0, ty0, tz0);
      tcMax = gloost::min(tx1, ty1, tz1);


      // handle second child
      if (parentNode->getValidMask().getFlag(childIndex))
      {
        if (parentNode->getLeafMask().getFlag(childIndex))
        {
          return parentNode->getChild(childIndex);
        }
        else
        {
          CpuRaycastStackElement newStackElement;
          newStackElement.parentNode   = parentNode->getChild(childIndex);
          newStackElement.parentTMin   = tcMin;
          newStackElement.parentTMax   = tcMax;
          newStackElement.parentCenter = childCenter;

          _stack[_currentDepth].nextChild  = 2;
          _stack[_currentDepth].parentTMin = tcMax;

          ++_currentDepth;
          _stack.push_back(newStackElement);

//          if (_currentDepth != _stack.size()-1) std::cerr << std::endl << "AHHHHH 2: " << _currentDepth << " != " << _stack.size()-1;

          continue;
        }
      }
      else
      {
        ++nextChild;
      }
    } // if still in parentNode



    //////////////////////////////////// THIRD CHILD of FOUR ////////////////////
    if(nextChild == 2 && tcMax < parentTMax)
    {
      // in parent voxel coordinates
      childEntryPoint = (origin + (tcMax + epsilon) * direction) - parentCenter;
      childIndex      =   4*(childEntryPoint[0] > 0)
                        + 2*(childEntryPoint[1] > 0)
                        +   (childEntryPoint[2] > 0);


      // in world coordinates
      gloost::Point3 childCenter = _idToPositionLookUp[childIndex]*parentScale + parentCenter;

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

      tcMin = gloost::max(tx0, ty0, tz0);
      tcMax = gloost::min(tx1, ty1, tz1);


      // handle firstChild
      if (parentNode->getValidMask().getFlag(childIndex))
      {
        if (parentNode->getLeafMask().getFlag(childIndex))
        {
          return parentNode->getChild(childIndex);
        }
        else
        {
          CpuRaycastStackElement newStackElement;
          newStackElement.parentNode   = parentNode->getChild(childIndex);
          newStackElement.parentTMin   = tcMin;
          newStackElement.parentTMax   = tcMax;
          newStackElement.parentCenter = childCenter;

          _stack[_currentDepth].nextChild  = 3;
          _stack[_currentDepth].parentTMin = tcMax;

          ++_currentDepth;
          _stack.push_back(newStackElement);

//          if (_currentDepth != _stack.size()-1) std::cerr << std::endl << "AHHHHH 3: " << _currentDepth << " != " << _stack.size()-1;

          continue;
        }
      }
      else
      {
        ++nextChild;
      }
    } // if still in parentNode


    //////////////////////////////////// FOURTH CHILD of FOUR ////////////////////
    if(nextChild == 3 && tcMax < parentTMax)
    {
      // in parent voxel coordinates
      childEntryPoint = (origin + (tcMax + epsilon) * direction) - parentCenter;
      childIndex      =   4*(childEntryPoint[0] > 0)
                        + 2*(childEntryPoint[1] > 0)
                        +   (childEntryPoint[2] > 0);

      // in world coordinates
      gloost::Point3 childCenter = _idToPositionLookUp[childIndex]*parentScale + parentCenter;

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


      tcMin = gloost::max(tx0, ty0, tz0);
      tcMax = gloost::min(tx1, ty1, tz1);


      // handle
      if (parentNode->getValidMask().getFlag(childIndex))
      {
        if (parentNode->getLeafMask().getFlag(childIndex))
        {
          return parentNode->getChild(childIndex);
        }
        else
        {
          CpuRaycastStackElement newStackElement;
          newStackElement.parentNode   = parentNode->getChild(childIndex);
          newStackElement.parentTMin   = tcMin;
          newStackElement.parentTMax   = tcMax;
          newStackElement.parentCenter = childCenter;

          if (_stack.size())
          {
            _stack[_currentDepth].nextChild  = 4;
            _stack[_currentDepth].parentTMin = tcMax;
          }

          ++_currentDepth;
          _stack.push_back(newStackElement);

//          if (_currentDepth != _stack.size()-1) std::cerr << std::endl << "AHHHHH 4: " << _currentDepth << " != " << _stack.size()-1;

          continue;
        }
      }
    } // if still in parentNode


    _stack.pop_back();
    --_currentDepth;
  }


  return 0;
}


////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////





} // namespace svo


