
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
#include <CpuRaycasterSingleRay.h>
#include <SvoVisualizer.h>



/// cpp includes
#include <string>
#include <iostream>
#include <map>
#include <list>



namespace svo
{

/**
  \class CpuRaycasterSingleRay

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

CpuRaycasterSingleRay::CpuRaycasterSingleRay(bool verboseMode):
  _pushCounter(0),
  _popCounter(0),
  _whileCounter(0),
  _maxStackDepth(0),
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

CpuRaycasterSingleRay::~CpuRaycasterSingleRay()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


//#define TEXTOUTPUTWHILETRAVERSAL

///**
//  \brief traverses a Svo with a single ray
//  \param ...
//  \remarks ...
//*/
//
//SvoNode*
//CpuRaycasterSingleRay::start(const gloost::Ray& ray, Svo* svo)
//{
//
//  static int frameCounter = 0;
//  ++frameCounter;
//
//#ifdef TEXTOUTPUTWHILETRAVERSAL
//  std::cerr << std::endl;
//  std::cerr << std::endl;
//  std::cerr << std::endl << "Traversal: " << frameCounter << "##########################################";
//#endif
//
//  _svo = svo;
//
//
//  gloost::BoundingBox svoBoundingBox(gloost::Point3(-0.5,-0.5,-0.5),
//                                     gloost::Point3(0.5,0.5,0.5));
//
//  gloost::Ray rayCopy(ray);
//  rayCopy.normalize();
//
//  if (svoBoundingBox.intersect(rayCopy, _tMax))
//  {
//    _tMin = rayCopy.getT();
//    return traversSvo(rayCopy);
//  }
//  else
//  {
//#ifdef TEXTOUTPUTWHILETRAVERSAL
//    std::cerr << std::endl << "missed looser!!!: ";
//#endif
//	  return false;
//  }
//
//
//	return false;
//}
//
//
//////////////////////////////////////////////////////////////////////////////////
//
//
///**
//  \brief traverses the Svo and returns the last valid hit child
//  \param ...
//  \remarks ...
//*/
//
//SvoNode*
//CpuRaycasterSingleRay::traversSvo(const gloost::Ray& ray)
//{
//  CpuRaycastStackElement element;
//  element.parentNode       = _svo->getRootNode();
//  element.parentTMin       = _tMin;
//  element.parentTMax       = _tMax;
//  element.parentCenter     = gloost::Point3(0.0, 0.0, 0.0);
//  element.parentDepth      = 0;
//
//  _stack.push(element);
//
//  // precalculate ray coefficients, tx(x) = "(1/dx)"x + "(-px/dx)"
//  // TODO: Chack ray direction component to be non zero!
//  gloost::mathType dxReziprok = 1.0/ray.getDirection()[0];
//  gloost::mathType minusPx_dx = -ray.getOrigin()[0]/ray.getDirection()[0];
//
//  gloost::mathType dyReziprok = 1.0/ray.getDirection()[1];
//  gloost::mathType minusPy_dy = -ray.getOrigin()[1]/ray.getDirection()[1];
//
//  gloost::mathType dzReziprok = 1.0/ray.getDirection()[2];
//  gloost::mathType minusPz_dz = -ray.getOrigin()[2]/ray.getDirection()[2];
//
//
//  while (_stack.size())
//  {
//    SvoNode*         parentNode         = _stack.top().parentNode;
//    gloost::mathType parentTMin         = _stack.top().parentTMin;
//    gloost::mathType parentTMax         = _stack.top().parentTMax;
//    gloost::Point3   parentCenter       = _stack.top().parentCenter;
//    int              depth              = _stack.top().parentDepth + 1;
//
//    _stack.pop();
//
//    gloost::Point3 parententryPoint = (ray.getOrigin() + parentTMin * ray.getDirection()) - parentCenter;
//    unsigned int   firstChildIndex = 4*(parententryPoint[0] >= 0) + 2*(parententryPoint[1] >= 0) + (parententryPoint[2] >= 0);
//
//
//#ifdef TEXTOUTPUTWHILETRAVERSAL
//    std::cerr << std::endl ;
//    std::cerr << std::endl << "#######  depth: " << depth;
//    std::cerr << std::endl << "first child id: " << firstChildIndex;
//#endif
//
//    /// hier den Punkt tMin vom parent benutzen um einstiegskind zu bekommen
//    /// x0,x1,y0,y1,z0,z1 für das einstiegskind setzen.
//    /// dann mit tx1, ty1 und tz1 < tcmax die folgekinder bestimmen
//    gloost::mathType scale     = pow(2, -depth+1);
//    gloost::mathType scaleHalf = pow(2, -depth)*0.5;
//
//    gloost::Point3 firstChildCenter = parentCenter + _idToPositionLookUp[firstChildIndex]*scale;
//
//    gloost::mathType x0 = firstChildCenter[0] - scaleHalf;
//    gloost::mathType x1 = firstChildCenter[0] + scaleHalf;
//    gloost::mathType y0 = firstChildCenter[1] - scaleHalf;
//    gloost::mathType y1 = firstChildCenter[1] + scaleHalf;
//    gloost::mathType z0 = firstChildCenter[2] - scaleHalf;
//    gloost::mathType z1 = firstChildCenter[2] + scaleHalf;
//
//
//    // tx(x) = (1/dx)x + (-px/dx)
//    // dx...Direction of the Ray in x
//    // px...Origin of the Ray in x
//
//    gloost::mathType tx0 = dxReziprok*x0 + minusPx_dx;
//    gloost::mathType tx1 = dxReziprok*x1 + minusPx_dx;
//    gloost::swapToIncreasing(tx0, tx1);
//
//    gloost::mathType ty0 = dyReziprok*y0 + minusPy_dy;
//    gloost::mathType ty1 = dyReziprok*y1 + minusPy_dy;
//    gloost::swapToIncreasing(ty0, ty1);
//
//    gloost::mathType tz0 = dzReziprok*z0 + minusPz_dz;
//    gloost::mathType tz1 = dzReziprok*z1 + minusPz_dz;
//    gloost::swapToIncreasing(tz0, tz1);
//
//
//    gloost::mathType tcMin = gloost::max(tx0, ty0, tz0);
//    gloost::mathType tcMax = gloost::min(tx1, ty1, tz1);
//
//    gloost::Point3 PtcMin = ray.getOrigin() + tcMin * ray.getDirection();
//    gloost::Point3 PtcMax = ray.getOrigin() + tcMax * ray.getDirection();
//
//
//
//    // tx1, ty1, tz1 sortieren und nacheinander switchen !
//
//    std::map<gloost::mathType, unsigned int> sortTiValues;
//    sortTiValues[tx1] = 0;
//    sortTiValues[ty1] = 1;
//    sortTiValues[tz1] = 2;
//
//    std::map<gloost::mathType, unsigned int>::iterator t1It    = sortTiValues.begin();
//    std::map<gloost::mathType, unsigned int>::iterator t1EndIt = sortTiValues.end();
//
//
//    if (parentNode->getValidMask().getFlag(firstChildIndex))
//    {
//      if (parentNode->getLeafMask().getFlag(firstChildIndex))
//      {
//
//
//#ifdef TEXTOUTPUTWHILETRAVERSAL
//        std::cerr << "   ----> hit a leaf! " << firstChildIndex;
//#endif
//        return parentNode->getChild(firstChildIndex);
//      }
//      else
//      {
//
//#ifdef TEXTOUTPUTWHILETRAVERSAL
//        std::cerr << "   ----> valid ... " << firstChildIndex;
//#endif
//
//        CpuRaycastStackElement firstNodeElement;
//        firstNodeElement.parentNode   = parentNode->getChild(firstChildIndex);
//        firstNodeElement.parentTMin   = tcMin;
//        firstNodeElement.parentTMax   = (*t1It).first;
//        firstNodeElement.parentCenter = firstChildCenter;
//        firstNodeElement.parentDepth  = depth;
//
//        _stack.push(firstNodeElement);
//      }
//    }
//
//    gloost::Vector3 indexBits(parententryPoint[0] >= 0, parententryPoint[1] >= 0, parententryPoint[2] >= 0);
//    unsigned int childCount = 0;
//    for (; t1It!=t1EndIt; ++t1It)
//    {
//      gloost::mathType t = t1It->first;
//      unsigned int     i = t1It->second;
//
//      if (t < parentTMax)
//      {
//        indexBits[i] = !indexBits[i];
//        unsigned int childIndex = (unsigned int) (4*(indexBits[0]) + 2*(indexBits[1]) + (indexBits[2]));
//
//#ifdef TEXTOUTPUTWHILETRAVERSAL
//        std::cerr << std::endl << "next child id:  " << childIndex;
//#endif
//        if (parentNode->getValidMask().getFlag(childIndex))
//        {
//          // found a leaf
//          if (parentNode->getLeafMask().getFlag(childIndex))
//          {
//
//#ifdef TEXTOUTPUTWHILETRAVERSAL
//            std::cerr << "   ----> hit a leaf! " << childIndex;
//#endif
//            return parentNode->getChild(childIndex);
//          }
//          else
//          {
//
//#ifdef TEXTOUTPUTWHILETRAVERSAL
//            std::cerr << "   ----> valid ... " << childIndex;
//#endif
//            // push child as a new parent node
//            std::map<gloost::mathType, unsigned int>::iterator nextIt = t1It;
//
//            CpuRaycastStackElement childElement;
//            childElement.parentNode = parentNode->getChild(childIndex);
////            childElement.parentTMin = t;
//
//            if (childCount < sortTiValues.size())
//            {
//              ++nextIt;
//              childElement.parentTMax = (nextIt)->first;
//            }
//            else
//            {
//              childElement.parentTMax = parentTMax;
//            }
//
//            childElement.parentCenter = parentCenter + _idToPositionLookUp[childIndex]*scale;
//            childElement.parentDepth  = depth;
//
//            _stack.push(childElement);
//          }
//
//        }
//      }
//
//    }
//
//  }
//
//
//  return 0;
//}


////////////////////////////////////////////////////////////////////////////////


//#define TEXTOUTPUTWHILETRAVERSAL

/**
  \brief traverses a Svo with a single ray
  \param ...
  \remarks ...
*/

SvoNode*
CpuRaycasterSingleRay::start(const gloost::Ray& ray, Svo* svo)
{
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
CpuRaycasterSingleRay::traversSvo(gloost::Ray ray, float tMin, float tMax)
{
  CpuRaycastStackElement element;
  element.parentNode       = _svo->getRootNode();
  element.parentTMin       = tMin;
  element.parentTMax       = tMax;
  element.parentCenter     = gloost::Point3(0.0, 0.0, 0.0);
  element.parentDepth      = 0;


  _stack.push(element);

  // precalculate ray coefficients, tx(x) = "(1/dx)"x + "(-px/dx)"
  static float epsilon = 0.00001;

  if ( gloost::abs(ray.getDirection()[0]) < epsilon) ray.getDirection()[0] = epsilon;
  if ( gloost::abs(ray.getDirection()[1]) < epsilon) ray.getDirection()[1] = epsilon;
  if ( gloost::abs(ray.getDirection()[2]) < epsilon) ray.getDirection()[2] = epsilon;



  gloost::mathType dxReziprok = 1.0/ray.getDirection()[0];
  gloost::mathType minusPx_dx = -ray.getOrigin()[0]/ray.getDirection()[0];

  gloost::mathType dyReziprok = 1.0/ray.getDirection()[1];
  gloost::mathType minusPy_dy = -ray.getOrigin()[1]/ray.getDirection()[1];

  gloost::mathType dzReziprok = 1.0/ray.getDirection()[2];
  gloost::mathType minusPz_dz = -ray.getOrigin()[2]/ray.getDirection()[2];



  CpuRaycastStackElement newStackElements[4];
  unsigned               newElementsCounter = 0;

  /////////////////// LOOP ///////////////////////////////
  while (_stack.size())
  {
    ++_whileCounter;

    newElementsCounter = 0;

    SvoNode*         parentNode         = _stack.top().parentNode;
    gloost::mathType parentTMin         = _stack.top().parentTMin;
    gloost::mathType parentTMax         = _stack.top().parentTMax;
    gloost::Point3   parentCenter       = _stack.top().parentCenter;
    int              depth              = _stack.top().parentDepth + 1;

    _stack.pop();
    ++_popCounter;

    if (parentTMax < 0.0)
    {
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

    // in parent voxel coordinates
    gloost::Point3 firstChildEntryPoint = (ray.getOrigin() + parentTMin * ray.getDirection()) - parentCenter;
    unsigned int   firstChildIndex = 4*(firstChildEntryPoint[0] >= 0)
                                   + 2*(firstChildEntryPoint[1] >= 0)
                                   +   (firstChildEntryPoint[2] >= 0);




    // in world coordinates
    gloost::Point3 firstChildCenter = _idToPositionLookUp[firstChildIndex]*parentScale + parentCenter;

    gloost::mathType x0 = firstChildCenter[0] - childSizeHalf;
    gloost::mathType x1 = firstChildCenter[0] + childSizeHalf;
    gloost::mathType y0 = firstChildCenter[1] - childSizeHalf;
    gloost::mathType y1 = firstChildCenter[1] + childSizeHalf;
    gloost::mathType z0 = firstChildCenter[2] - childSizeHalf;
    gloost::mathType z1 = firstChildCenter[2] + childSizeHalf;


    // tx(x) = (1/dx)x + (-px/dx)
    // dx...Direction of the Ray in x
    // px...Origin of the Ray in x
    gloost::mathType tx0 = dxReziprok*x0 + minusPx_dx;
    gloost::mathType tx1 = dxReziprok*x1 + minusPx_dx;
    gloost::swapToIncreasing(tx0, tx1);

    gloost::mathType ty0 = dyReziprok*y0 + minusPy_dy;
    gloost::mathType ty1 = dyReziprok*y1 + minusPy_dy;
    gloost::swapToIncreasing(ty0, ty1);

    gloost::mathType tz0 = dzReziprok*z0 + minusPz_dz;
    gloost::mathType tz1 = dzReziprok*z1 + minusPz_dz;
    gloost::swapToIncreasing(tz0, tz1);


    gloost::mathType tcMin = gloost::max(tx0, ty0, tz0); // <- you can only enter once
    gloost::mathType tcMax = gloost::min(tx1, ty1, tz1); // <- you can only leave once



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
        firstNodeElement.parentTMax   = tcMax;
        firstNodeElement.parentCenter = firstChildCenter;
        firstNodeElement.parentDepth  = depth;

        newStackElements[newElementsCounter++] = firstNodeElement;
      }
    }


    //////////////////////////////////// SECOND CHILD of FOUR ////////////////////
    if(tcMax < parentTMax)
    {
      // in parent voxel coordinates
      gloost::Point3 secondChildEntryPoint = (ray.getOrigin() + (tcMax + 0.0001) * ray.getDirection()) - parentCenter;
      unsigned int   secondChildIndex = 4*(secondChildEntryPoint[0] >= 0)
                                      + 2*(secondChildEntryPoint[1] >= 0)
                                      +   (secondChildEntryPoint[2] >= 0);




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
      tcMax = gloost::min(tx1, ty1, tz1);


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
          secondNodeElement.parentTMax   = tcMax;
          secondNodeElement.parentCenter = secondChildCenter;
          secondNodeElement.parentDepth  = depth;

          newStackElements[newElementsCounter++] = (secondNodeElement);
        }
      }
    } // if still in parentNode

    //////////////////////////////////// THIRD CHILD of FOUR ////////////////////
    if(tcMax < parentTMax)
    {
      // in parent voxel coordinates
      gloost::Point3 thirdChildEntryPoint = (ray.getOrigin() + (tcMax + 0.0001) * ray.getDirection()) - parentCenter;
      unsigned int   thirdChildIndex =  4*(thirdChildEntryPoint[0] >= 0)
                                      + 2*(thirdChildEntryPoint[1] >= 0)
                                      +   (thirdChildEntryPoint[2] >= 0);




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
      tcMax = gloost::min(tx1, ty1, tz1);


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
          thirdNodeElement.parentTMax   = tcMax;
          thirdNodeElement.parentCenter = thirdChildCenter;
          thirdNodeElement.parentDepth  = depth;

          newStackElements[newElementsCounter++] = (thirdNodeElement);
        }
      }
    } // if still in parentNode



    //////////////////////////////////// FOURTH CHILD of FOUR ////////////////////
    if(tcMax < parentTMax){
      // in parent voxel coordinates
      gloost::Point3 fourthChildEntryPoint = (ray.getOrigin() + (tcMax + 0.0001) * ray.getDirection()) - parentCenter;
      unsigned int   fourthChildIndex =  4*(fourthChildEntryPoint[0] >= 0)
                                       + 2*(fourthChildEntryPoint[1] >= 0)
                                       +   (fourthChildEntryPoint[2] >= 0);




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
      tcMax = gloost::min(tx1, ty1, tz1);


      // handle firstChild
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
          fourthNodeElement.parentTMax   = tcMax;
          fourthNodeElement.parentCenter = fourthChildCenter;
          fourthNodeElement.parentDepth  = depth;

          newStackElements[newElementsCounter++] = (fourthNodeElement);
        }
      }
    } // if still in parentNode

    --newElementsCounter;

    for (int i = newElementsCounter; i!=-1; --i)
    {
      _stack.push(newStackElements[i]);
      ++_pushCounter;
    }

    if (_maxStackDepth < _stack.size())
    {
      _maxStackDepth = _stack.size();
    }

  }


  return 0;
}


////////////////////////////////////////////////////////////////////////////////



void
CpuRaycasterSingleRay::sortAxisAndTs(AxisAndTValue* axisAndTs)
{
  AxisAndTValue tmp;

  if (axisAndTs[0]._t > axisAndTs[1]._t)
  {
    tmp = axisAndTs[0];
    axisAndTs[0] = axisAndTs[1];
    axisAndTs[1] = tmp;
  }

  if (axisAndTs[1]._t > axisAndTs[2]._t)
  {
    tmp = axisAndTs[1];
    axisAndTs[1] = axisAndTs[2];
    axisAndTs[2] = tmp;
  }

  if (axisAndTs[0]._t > axisAndTs[1]._t)
  {
    tmp = axisAndTs[0];
    axisAndTs[0] = axisAndTs[1];
    axisAndTs[1] = tmp;
  }

//  for (unsigned i=0; i!=3; ++i)
//  {
//    std::cerr << std::endl << axisAndTs[i]._axis << " : " << axisAndTs[i]._t;
//  }
//  std::cerr << std::endl << "-----------------------------------------";

}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief intersects the Svo BoundingBox
  \param ...
  \remarks ...
*/

bool
CpuRaycasterSingleRay::intersectBoundingBox(const gloost::Ray& ray)
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


