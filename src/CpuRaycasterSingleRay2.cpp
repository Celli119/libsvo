
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
  _pushCounter(0),
  _stack(),
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

//  std::cerr << std::endl << "CpuRaycasterSingleRay2: ";

  _svo = svo;

  while (_stack.size())
  {
    _stack.pop();
  }

  gloost::BoundingBox svoBoundingBox(gloost::Point3(-0.5,-0.5,-0.5),
                                     gloost::Point3(0.5,0.5,0.5));

  gloost::mathType tMin, tMax;

  if (svoBoundingBox.intersect(ray, tMin, tMax))
  {
    return traversSvo(ray, tMin, tMax);
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
CpuRaycasterSingleRay2::traversSvo(const gloost::Ray& ray, float tMin, float tMax)
{
  CpuRaycastStackElement element;
  element.parentNode       = _svo->getRootNode();
  element.parentTMin       = tMin;
  element.parentTMax       = tMax;
  element.parentCenter     = gloost::Point3(0.0, 0.0, 0.0);
  element.parentDepth      = 0;


  _stack.push(element);

  // precalculate ray coefficients, tx(x) = "(1/dx)"x + "(-px/dx)"
  const float epsilon = 0.0000001;

  if ( ( gloost::abs(ray.getDirection()[0]) < epsilon) ||
       ( gloost::abs(ray.getDirection()[1]) < epsilon) ||
       ( gloost::abs(ray.getDirection()[2]) < epsilon) )
      {
        return 0;
      }

  gloost::mathType dxReziprok = 1.0/ray.getDirection()[0];
  gloost::mathType minusPx_dx = -ray.getOrigin()[0]/ray.getDirection()[0];

  gloost::mathType dyReziprok = 1.0/ray.getDirection()[1];
  gloost::mathType minusPy_dy = -ray.getOrigin()[1]/ray.getDirection()[1];

  gloost::mathType dzReziprok = 1.0/ray.getDirection()[2];
  gloost::mathType minusPz_dz = -ray.getOrigin()[2]/ray.getDirection()[2];


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




  /////////////////// LOOP ///////////////////////////////
  while (_stack.size())
  {

    ++_whileCounter;

    CpuRaycastStackElement parent = _stack.top();

    SvoNode*         parentNode         = parent.parentNode;
    gloost::mathType parentTMin         = parent.parentTMin;
    gloost::mathType parentTMax         = parent.parentTMax;
    gloost::Point3   parentCenter       = parent.parentCenter;
    int              depth              = parent.parentDepth + 1;
    unsigned         nextChild          = parent.nextChild;
//    _stack.pop();

    if (parentTMax < 0.0)
    {
      _stack.pop();
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
      gloost::Point3 firstChildEntryPoint = (ray.getOrigin() + parentTMin * ray.getDirection()) - parentCenter;
      unsigned int   firstChildIndex = 4*(firstChildEntryPoint[0] > 0)
                                     + 2*(firstChildEntryPoint[1] > 0)
                                     +   (firstChildEntryPoint[2] > 0);




      // in world coordinates
      gloost::Point3 firstChildCenter = _idToPositionLookUp[firstChildIndex]*parentScale + parentCenter;

      x0 = firstChildCenter[0] - childSizeHalf;
      x1 = firstChildCenter[0] + childSizeHalf;
      y0 = firstChildCenter[1] - childSizeHalf;
      y1 = firstChildCenter[1] + childSizeHalf;
      z0 = firstChildCenter[2] - childSizeHalf;
      z1 = firstChildCenter[2] + childSizeHalf;


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
        //tcMax = gloost::min(tx1, ty1, tz1); // <- you can only leave once
        parentTMin = gloost::min(tx1, ty1, tz1); // <- you can only leave once



        // handle firstChild
        if (parentNode->getValidMask().getFlag(firstChildIndex))
        {
          if (parentNode->getLeafMask().getFlag(firstChildIndex))
          {
            return parentNode->getChild(firstChildIndex);
          }
          else
          {
            CpuRaycastStackElement firstNodeElement;
            firstNodeElement.parentNode   = parentNode->getChild(firstChildIndex);
            firstNodeElement.parentTMin   = tcMin;
            firstNodeElement.parentTMax   = parentTMin;
            firstNodeElement.parentCenter = firstChildCenter;
            firstNodeElement.parentDepth  = depth;
            firstNodeElement.nextChild    = 0;

  //          newStackElements[newElementsCounter++] = firstNodeElement;
            _stack.top().nextChild  = 1;
            _stack.top().parentTMin = parentTMin;

            _stack.push(firstNodeElement);


            ++_pushCounter;

            continue;
          }
        }
        else
        {
          ++nextChild;
        }
    }

    //////////////////////////////////// SECOND CHILD of FOUR ////////////////////
    if(nextChild == 1 && parentTMin < parentTMax)
    {
      // in parent voxel coordinates
      gloost::Point3 secondChildEntryPoint = (ray.getOrigin() + (parentTMin + 0.0001) * ray.getDirection()) - parentCenter;
      unsigned int   secondChildIndex = 4*(secondChildEntryPoint[0] > 0)
                                      + 2*(secondChildEntryPoint[1] > 0)
                                      +   (secondChildEntryPoint[2] > 0);




      // in world coordinates
      gloost::Point3 secondChildCenter = _idToPositionLookUp[secondChildIndex]*parentScale + parentCenter;

      x0 = secondChildCenter[0] - childSizeHalf;
      x1 = secondChildCenter[0] + childSizeHalf;
      y0 = secondChildCenter[1] - childSizeHalf;
      y1 = secondChildCenter[1] + childSizeHalf;
      z0 = secondChildCenter[2] - childSizeHalf;
      z1 = secondChildCenter[2] + childSizeHalf;


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

        //tcMax = gloost::min(tx1, ty1, tz1);
        parentTMin = gloost::min(tx1, ty1, tz1);


        // handle firstChild
        if (parentNode->getValidMask().getFlag(secondChildIndex))
        {
          if (parentNode->getLeafMask().getFlag(secondChildIndex))
          {
            return parentNode->getChild(secondChildIndex);
          }
          else
          {
            CpuRaycastStackElement secondNodeElement;
            secondNodeElement.parentNode   = parentNode->getChild(secondChildIndex);
            secondNodeElement.parentTMin   = tcMin;
            secondNodeElement.parentTMax   = parentTMin;
            secondNodeElement.parentCenter = secondChildCenter;
            secondNodeElement.parentDepth  = depth;

            //newStackElements[newElementsCounter++] = (secondNodeElement);
            _stack.top().nextChild  = 2;
            _stack.top().parentTMin = parentTMin;

            _stack.push(secondNodeElement);

            ++_pushCounter;

            continue;
          }
        }
        else{
          ++nextChild;
        }
    } // if still in parentNode

    //////////////////////////////////// THIRD CHILD of FOUR ////////////////////
    if(nextChild == 2 && parentTMin < parentTMax)
    {
      // in parent voxel coordinates
      gloost::Point3 thirdChildEntryPoint = (ray.getOrigin() + (parentTMin + 0.0001) * ray.getDirection()) - parentCenter;
      unsigned int   thirdChildIndex =  4*(thirdChildEntryPoint[0] > 0)
                                      + 2*(thirdChildEntryPoint[1] > 0)
                                      +   (thirdChildEntryPoint[2] > 0);




      // in world coordinates
      gloost::Point3 thirdChildCenter = _idToPositionLookUp[thirdChildIndex]*parentScale + parentCenter;

      x0 = thirdChildCenter[0] - childSizeHalf;
      x1 = thirdChildCenter[0] + childSizeHalf;
      y0 = thirdChildCenter[1] - childSizeHalf;
      y1 = thirdChildCenter[1] + childSizeHalf;
      z0 = thirdChildCenter[2] - childSizeHalf;
      z1 = thirdChildCenter[2] + childSizeHalf;


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

      //tcMax = gloost::min(tx1, ty1, tz1);
      parentTMin = gloost::min(tx1, ty1, tz1);


        // handle firstChild
        if (parentNode->getValidMask().getFlag(thirdChildIndex))
        {
          if (parentNode->getLeafMask().getFlag(thirdChildIndex))
          {
            return parentNode->getChild(thirdChildIndex);
          }
          else
          {
            CpuRaycastStackElement thirdNodeElement;
            thirdNodeElement.parentNode   = parentNode->getChild(thirdChildIndex);
            thirdNodeElement.parentTMin   = tcMin;
            thirdNodeElement.parentTMax   = parentTMin;
            thirdNodeElement.parentCenter = thirdChildCenter;
            thirdNodeElement.parentDepth  = depth;

            //newStackElements[newElementsCounter++] = (thirdNodeElement);
            _stack.top().nextChild  = 3;
            _stack.top().parentTMin = parentTMin;

            _stack.push(thirdNodeElement);

            ++_pushCounter;

            continue;
          }
        }
        else{
          ++nextChild;
        }
    } // if still in parentNode


    _stack.pop();
    ++_popCounter;

    //////////////////////////////////// FOURTH CHILD of FOUR ////////////////////
    if(nextChild == 3 && parentTMin < parentTMax)
    {
      // in parent voxel coordinates
      gloost::Point3 fourthChildEntryPoint = (ray.getOrigin() + (parentTMin + 0.0001) * ray.getDirection()) - parentCenter;
      unsigned int   fourthChildIndex =  4*(fourthChildEntryPoint[0] > 0)
                                       + 2*(fourthChildEntryPoint[1] > 0)
                                       +   (fourthChildEntryPoint[2] > 0);




      // in world coordinates
      gloost::Point3 fourthChildCenter = _idToPositionLookUp[fourthChildIndex]*parentScale + parentCenter;

      x0 = fourthChildCenter[0] - childSizeHalf;
      x1 = fourthChildCenter[0] + childSizeHalf;
      y0 = fourthChildCenter[1] - childSizeHalf;
      y1 = fourthChildCenter[1] + childSizeHalf;
      z0 = fourthChildCenter[2] - childSizeHalf;
      z1 = fourthChildCenter[2] + childSizeHalf;


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

        //tcMax = gloost::min(tx1, ty1, tz1);
        parentTMin = gloost::min(tx1, ty1, tz1);


        // handle
        if (parentNode->getValidMask().getFlag(fourthChildIndex))
        {
          if (parentNode->getLeafMask().getFlag(fourthChildIndex))
          {
            return parentNode->getChild(fourthChildIndex);
          }
          else
          {
            CpuRaycastStackElement fourthNodeElement;
            fourthNodeElement.parentNode   = parentNode->getChild(fourthChildIndex);
            fourthNodeElement.parentTMin   = tcMin;
            fourthNodeElement.parentTMax   = parentTMin;
            fourthNodeElement.parentCenter = fourthChildCenter;
            fourthNodeElement.parentDepth  = depth;


            //newStackElements[newElementsCounter++] = (fourthNodeElement);
            _stack.push(fourthNodeElement);

            ++_pushCounter;
  //          ++_pushCounter;
  //
  //          continue;
          }
        }
  //      else{
  //        ++nextChild;
  //      }
    } // if still in parentNode



  }


  return 0;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief intersects the Svo BoundingBox
  \param ...
  \remarks ...
*/

bool
CpuRaycasterSingleRay2::intersectBoundingBox(const gloost::Ray& ray)
{

  gloost::Point3 pMin(-0.5,-0.5,-0.5);
  gloost::Point3 pMax(0.5,0.5,0.5);


  // this is a bit unsave
	double tNear = -100000000;
	double tFar  =  100000000;

	// parallel to plane 0..1
	for (int i=0; i!=3; ++i)
	{
    if (ray.getDirection()[i] == 0)
    {
      if (!(pMin[i] < ray.getOrigin()[i] && pMax[i] > ray.getOrigin()[i]))
      {
        return false;
      }
    }

    double t1 = (pMin[i] - ray.getOrigin()[i]) / ray.getDirection()[i];
    double t2 = (pMax[i] - ray.getOrigin()[i]) / ray.getDirection()[i];

    // swap
    if (t1 > t2)
    {
      double temp = t1;
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

    if (tFar < 0)
    {
      return false;
    }
	}
//  ray.setT(tNear);

  _tMin = tNear;
  _tMax = tFar;



  return true;

}


////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////





} // namespace svo


