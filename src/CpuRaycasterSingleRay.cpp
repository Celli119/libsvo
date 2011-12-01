
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

CpuRaycasterSingleRay::CpuRaycasterSingleRay():
  _db_maxDepth(2),
  _stack(),
  _tMin(0),
  _tMax(10000),
  _entryPoint(),
  _exitPoint(),
  _svo(0),
  _visualizer(new SvoVisualizer(100))
{
	// insert your code here
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


/**
  \brief traverses a Svo with a single ray
  \param ...
  \remarks ...
*/

SvoNode*
CpuRaycasterSingleRay::start(const gloost::Ray& ray, Svo* svo)
{


  static int frameCounter = 0;
  ++frameCounter;

  std::cerr << std::endl;
  std::cerr << std::endl;
  std::cerr << std::endl << "Traversal: " << frameCounter << "##########################################";


  _svo = svo;

  if (_visualizer)
  {
    delete _visualizer;
    _visualizer = new SvoVisualizer(100, SVO_VISUALIZER_MODE_BOXED_LEAFES);
  }


  gloost::BoundingBox svoBoundingBox(gloost::Point3(-0.5,-0.5,-0.5),
                                     gloost::Point3(0.5,0.5,0.5));

  gloost::Ray rayCopy(ray);
  rayCopy.normalize();


  if (svoBoundingBox.intersect(rayCopy))
  {

    _tMin = rayCopy.getT();

    _entryPoint = ray.getOrigin() + rayCopy.getT() * rayCopy.getDirection();
    _exitPoint  = ray.getOrigin() + rayCopy.getT() * rayCopy.getDirection();

    return traversSvo(ray);
  }
  else
  {
    std::cerr << std::endl << "missed looser!!!: ";
  }


	return 0;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief traverses the Svo and returns the last valid hit child
  \param ...
  \remarks ...
*/

SvoNode*
CpuRaycasterSingleRay::traversSvo(const gloost::Ray& ray)
{
  CpuRaycastStackElement element;
  element.parentNode       = _svo->getRootNode();
  element.parentTMin       = _tMin;
  element.parentCenter     = gloost::Point3(0.0, 0.0, 0.0);
  element.parentDepth      = 0;

  _stack.push(element);


  std::vector<gloost::Point3> idToPositionLookUp(8);
  idToPositionLookUp[0] = gloost::Point3(-0.25, -0.25, -0.25);
  idToPositionLookUp[1] = gloost::Point3(-0.25, -0.25,  0.25);
  idToPositionLookUp[2] = gloost::Point3(-0.25,  0.25, -0.25);
  idToPositionLookUp[3] = gloost::Point3(-0.25,  0.25,  0.25);
  idToPositionLookUp[4] = gloost::Point3( 0.25, -0.25, -0.25);
  idToPositionLookUp[5] = gloost::Point3( 0.25, -0.25,  0.25);
  idToPositionLookUp[6] = gloost::Point3( 0.25,  0.25, -0.25);
  idToPositionLookUp[7] = gloost::Point3( 0.25,  0.25,  0.25);

  std::vector<gloost::vec4> idToColors(8);

  idToColors[0] = gloost::vec4(1.0, 0.0, 0.0, 1.0);   // green
  idToColors[1] = gloost::vec4(1.0, 1.0, 1.0, 1.0);   // white
  idToColors[2] = gloost::vec4(1.0, 1.0, 1.0, 1.0);   // white
  idToColors[3] = gloost::vec4(1.0, 1.0, 1.0, 1.0);   // white
  idToColors[4] = gloost::vec4(1.0, 1.0, 1.0, 1.0);   // white
  idToColors[5] = gloost::vec4(1.0, 1.0, 1.0, 1.0);   // white
  idToColors[6] = gloost::vec4(1.0, 0.0, 1.0, 1.0);   // yellow
  idToColors[7] = gloost::vec4(1.0, 1.0, 1.0, 1.0);   // white




  // visualize root
  gloost::BoundingBox rootBb(gloost::Point3(-0.5,-0.5,-0.5),
                             gloost::Point3( 0.5, 0.5, 0.5));

  _visualizer->pushWireCubeToMesh(rootBb,
                                  0,
                                  gloost::vec4(0.1, 0.1, 0.1, 1.0));


  // precalculate ray coefficients, tx(x) = "(1/dx)"x + "(-px/dx)"
  // TODO: Chack ray direction component to be non zero!
  gloost::mathType dxReziprok = 1.0/ray.getDirection()[0];
  gloost::mathType minusPx_dx = -ray.getOrigin()[0]/ray.getDirection()[0];

  gloost::mathType dyReziprok = 1.0/ray.getDirection()[1];
  gloost::mathType minusPy_dy = -ray.getOrigin()[1]/ray.getDirection()[1];

  gloost::mathType dzReziprok = 1.0/ray.getDirection()[2];
  gloost::mathType minusPz_dz = -ray.getOrigin()[2]/ray.getDirection()[2];


  while (_stack.size())
  {
    SvoNode*         parentNode         = _stack.top().parentNode;
    gloost::mathType parentTMin         = _stack.top().parentTMin;
    gloost::Point3   parentCenter       = _stack.top().parentCenter;
    int              depth              = _stack.top().parentDepth + 1;


    gloost::Point3 parententryPoint = ray.getOrigin() + parentTMin * ray.getDirection();
    unsigned int   firstChildIndex = 4*(parententryPoint[0] >= 0) + 2*(parententryPoint[1] >= 0) + (parententryPoint[2] >= 0);

    std::cerr << std::endl ;
    std::cerr << std::endl << "#######  depth: " << depth;
    std::cerr << std::endl << "first child id: " << firstChildIndex;


    /// hier den Punkt tMin vom parent benutzen um einstiegskind zu bekommen
    /// x0,x1,y0,y1,z0,z1 für das einstiegskind setzen.
    /// dann mit tx1, ty1 und tz1 < tcmax die folgekinder bestimmen

    _stack.pop();

    gloost::Point3 firstchildLocalOffset = idToPositionLookUp[firstChildIndex];


    gloost::mathType scale     = pow(2, -depth+1);
    gloost::mathType scaleHalf = pow(2, -depth)*0.5;

    gloost::Point3 firstChildCenter = parentCenter + firstchildLocalOffset*scale;

    std::cerr << std::endl << "firstchildLocalOffset: " << firstchildLocalOffset;
    std::cerr << std::endl << "firstChildCenter:      " << firstChildCenter;



//    gloost::Ray tmp(ray);
//    tmp.setT(parentTMin);
//    std::cerr << std::endl << "tmp: " << tmp;


    gloost::mathType x0 = firstChildCenter[0] - scaleHalf;
    gloost::mathType x1 = firstChildCenter[0] + scaleHalf;
    gloost::mathType y0 = firstChildCenter[1] - scaleHalf;
    gloost::mathType y1 = firstChildCenter[1] + scaleHalf;
    gloost::mathType z0 = firstChildCenter[2] - scaleHalf;
    gloost::mathType z1 = firstChildCenter[2] + scaleHalf;


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


    gloost::mathType tcMin = gloost::max(tx0, ty0, tz0);
    gloost::mathType tcMax = gloost::min(tx1, ty1, tz1);

    gloost::Point3 PtcMin = ray.getOrigin() + tcMin * ray.getDirection();
    gloost::Point3 PtcMax = ray.getOrigin() + tcMax * ray.getDirection();



    // tx1, ty1, tz1 sortieren und nacheinander switchen !
    std::map<gloost::mathType, unsigned int> sortTiValues;
    sortTiValues[tx1] = 0;
    sortTiValues[ty1] = 1;
    sortTiValues[tz1] = 2;

    std::map<gloost::mathType, unsigned int>::iterator tIt    = sortTiValues.begin();
    std::map<gloost::mathType, unsigned int>::iterator tEndIt = sortTiValues.end();


    if (parentNode->getValidMask().getFlag(firstChildIndex))
    {
      if (parentNode->getLeafMask().getFlag(firstChildIndex))
      {
        std::cerr << "   ----> hit a leaf! " << firstChildIndex;
      }
      else
      {
        std::cerr << "   ----> valid ... " << firstChildIndex;


        /// VISUAL ###################################################
        gloost::BoundingBox nodeBb(gloost::Point3(x0,y0,z0),
                                   gloost::Point3(x1,y1,z1));



        _visualizer->pushWireCubeToMesh(nodeBb,
                                        depth,
                                        idToColors[firstChildIndex]);
        /// VISUAL ###################################################


        CpuRaycastStackElement firstNodeElement;
        firstNodeElement.parentNode   = parentNode->getChild(firstChildIndex);
        firstNodeElement.parentTMin   = tcMin;
        firstNodeElement.parentCenter = firstChildCenter;
        firstNodeElement.parentDepth  = depth;

        _stack.push(firstNodeElement);
      }
    }

//
//
//    gloost::Vector3 indexBits(parententryPoint[0] >= 0, parententryPoint[1] >= 0, parententryPoint[2] >= 0);
//
//    unsigned int childCount = 0;
//    for (; tIt!=tEndIt; ++tIt)
//    {
//      gloost::mathType t = tIt->first;
//      unsigned int     i = tIt->second;
//
//      if (t < parentTMax)
//      {
//        indexBits[i] = !indexBits[i];
//        unsigned int childIndex = (unsigned int) (4*(indexBits[0]) + 2*(indexBits[1]) + (indexBits[2]));
//
//        std::cerr << std::endl << "next child id:  " << childIndex;
//
//        if (parentNode->getValidMask().getFlag(childIndex))
//        {
//          // found a leaf
//          if (parentNode->getLeafMask().getFlag(childIndex))
//          {
//            std::cerr << "   ----> hit a leaf! " << childIndex;
//            return parentNode->getChild(childIndex);
//          }
//          else
//          {
//            std::cerr << "   ----> valid ... " << childIndex;
//
//            // push child as a new parent node
//            std::map<gloost::mathType, unsigned int>::iterator nextIt = tIt;
//
//            CpuRaycastStackElement childElement;
//            childElement.parentNode = parentNode->getChild(childIndex);
//            childElement.parentTMin = t;
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
//            childElement.parentCenter = parentCenter + idToPositionLookUp[childIndex]*scale;
//            childElement.parentDepth  = depth;
//
//            _stack.push(childElement);
//          }
//
//        }
//      }
//
//    }

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


/**
  \brief returns the visualzer
  \param ...
  \remarks ...
*/

SvoVisualizer*
CpuRaycasterSingleRay::getVisualizer()
{
  return _visualizer;
}


////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////





} // namespace svo


