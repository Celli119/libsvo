
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



#ifndef H_SVO_CPU_RAYCASTER_SINGLE_RAY3
#define H_SVO_CPU_RAYCASTER_SINGLE_RAY3



/// svo system includes
#include <SvoNode.h>
#include <Svo.h>


/// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>
#include <gloost/Ray.h>


/// cpp includes
#include <string>
#include <stack>



namespace svo
{

  //  CpuRaycasterSingleRay3

class CpuRaycasterSingleRay3
{
	public:


    struct CpuRaycastStackElement
    {
      CpuRaycastStackElement()
      {}

      unsigned         parentNodeIndex;
      gloost::mathType parentTMin;
      gloost::mathType parentTMax;

      gloost::Point3   parentCenter;

//      char nextChild;
    };


    struct ResultStruct
    {
      ResultStruct()
      {
        hit = 0;
      }

      bool           hit;
      gloost::Point3 nodeCenter;
      unsigned       nodeIndex;
      unsigned       attribIndex;
      unsigned       depth;
      float          t;
      unsigned       numWhileLoops;
    };




    // class constructor
    CpuRaycasterSingleRay3(bool verboseMode = false);

    // class destructor
	  ~CpuRaycasterSingleRay3();


    // traverses a Svo with a single ray
//	  SvoNode* start(const gloost::Ray& ray, Svo* svo);
	  bool start(const gloost::Ray& ray,
               float tScaleRatio,
               Svo* svo,
               ResultStruct& result);

    // traverses a Svo with a single ray
//	  SvoNode* traversSvo(const gloost::Ray& ray);
	  bool traversSvo(gloost::Point3 origin,
                    gloost::Vector3 direction,
                    float tMin,
                    float tMax,
                    ResultStruct& result);


    int _max_stack_size;


	private:


//	 void sortAxisAndTs(AxisAndTValue* axisAndTs);

   std::vector<CpuRaycastStackElement> _stack;

   float _tScaleRatio;

   gloost::mathType _tMin;
   gloost::mathType _tMax;

   Svo*             _svo;

   std::vector<gloost::Point3> _idToPositionLookUp;

   bool _verboseMode;

};


} // namespace svo


#endif // H_SVO_CPU_RAYCASTER_SINGLE_RAY3


