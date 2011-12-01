
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



#ifndef H_SVO_CPU_RAYCASTER_SINGLE_RAY
#define H_SVO_CPU_RAYCASTER_SINGLE_RAY



/// svo system includes
#include <SvoNode.h>
#include <Svo.h>


/// gloost system includes
#include <gloostConfig.h>
#include <gloostMath.h>
#include <Ray.h>


/// cpp includes
#include <string>
#include <stack>



namespace svo
{

  class SvoVisualizer;


  //  CpuRaycasterSingleRay

class CpuRaycasterSingleRay
{
	public:

    // class constructor
    CpuRaycasterSingleRay();

    // class destructor
	  ~CpuRaycasterSingleRay();


    // traverses a Svo with a single ray
	  SvoNode* start(const gloost::Ray& ray, Svo* svo);

    // traverses a Svo with a single ray
	  SvoNode* traversSvo(const gloost::Ray&    ray);


	  // intersects the Svo BoundingBox and writes to _tMin and _tMax
	  bool intersectBoundingBox(const gloost::Ray& ray);


    // returns the visualzer
    SvoVisualizer* getVisualizer();


    struct CpuRaycastStackElement
    {
      CpuRaycastStackElement():
        parentNode(0),
        parentTMin(0),
        parentCenter(),
        parentDepth(0)
      {}

      SvoNode*         parentNode;

      gloost::mathType parentTMin;

      gloost::Point3   parentCenter;
      int              parentDepth;
    };



  unsigned _db_maxDepth;


	private:

   std::stack<CpuRaycastStackElement> _stack;

   gloost::mathType _tMin;
   gloost::mathType _tMax;

   gloost::Point3   _entryPoint;
   gloost::Point3   _exitPoint;

   Svo*             _svo;

   SvoVisualizer*   _visualizer;

};


} // namespace svo


#endif // H_SVO_CPU_RAYCASTER_SINGLE_RAY


