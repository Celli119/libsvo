
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



#ifndef H_SVO_CPU_RAYCASTER_SINGLE_RAY2
#define H_SVO_CPU_RAYCASTER_SINGLE_RAY2



/// svo system includes
#include <SvoNode.h>
#include <SvoBranch.h>


/// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>
#include <gloost/Ray.h>


/// cpp includes
#include <string>
#include <stack>



namespace svo
{

  //  CpuRaycasterSingleRay2

class CpuRaycasterSingleRay2
{
	public:

    // class constructor
    CpuRaycasterSingleRay2(bool verboseMode = false);

    // class destructor
	  ~CpuRaycasterSingleRay2();


    // traverses a Svo with a single ray
//	  SvoNode* start(const gloost::Ray& ray, SvoBranch* svo);
	  SvoNode* start(const gloost::Ray& ray, SvoBranch* svo);

    // traverses a Svo with a single ray
//	  SvoNode* traversSvo(const gloost::Ray& ray);
	  SvoNode* traversSvo(gloost::Point3 origin,
                        gloost::Vector3 direction,
                        float tMin,
                        float tMax);


    struct CpuRaycastStackElement
    {
      CpuRaycastStackElement()
      {}

      SvoNode*         parentNode;
      gloost::mathType parentTMin;
      gloost::mathType parentTMax;

      gloost::Point3   parentCenter;

//      char nextChild;
    };

    int _max_stack_size;


	private:


//	 void sortAxisAndTs(AxisAndTValue* axisAndTs);

   std::vector<CpuRaycastStackElement> _stack;

   gloost::mathType _tMin;
   gloost::mathType _tMax;

   SvoBranch*             _svo;

   std::vector<gloost::Point3> _idToPositionLookUp;

   bool _verboseMode;

};


} // namespace svo


#endif // H_SVO_CPU_RAYCASTER_SINGLE_RAY


