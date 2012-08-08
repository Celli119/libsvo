
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
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>
#include <gloost/Ray.h>


/// cpp includes
#include <string>
#include <stack>



namespace svo
{


struct AxisAndTValue
{
  float _t;
  int   _axis;

  bool operator<(const AxisAndTValue& a)
  {
    return a._t < this->_t;
  }

};



  //  CpuRaycasterSingleRay

class CpuRaycasterSingleRay
{
	public:

    // class constructor
    CpuRaycasterSingleRay(bool verboseMode = false);

    // class destructor
	  virtual ~CpuRaycasterSingleRay();


    // traverses a Svo with a single ray
//	  SvoNode* start(const gloost::Ray& ray, Svo* svo);
	  virtual SvoNode* start(const gloost::Ray& ray, Svo* svo);

    // traverses a Svo with a single ray
//	  SvoNode* traversSvo(const gloost::Ray& ray);
	  virtual SvoNode* traversSvo(gloost::Ray ray, float tMin, float tMax);


	  // intersects the Svo BoundingBox and writes to _tMin and _tMax
	  virtual bool intersectBoundingBox(const gloost::Ray& ray);


    struct CpuRaycastStackElement
    {
      CpuRaycastStackElement():
        parentNode(0),
        parentTMin(0),
        parentTMax(0),
        parentCenter(),
        parentDepth(0)
      {}

      SvoNode*         parentNode;

      gloost::mathType parentTMin;
      gloost::mathType parentTMax;

      gloost::Point3   parentCenter;
      int              parentDepth;
    };


    unsigned _pushCounter;
    unsigned _popCounter;
    unsigned _whileCounter;
    unsigned _maxStackDepth;


	private:


	 void sortAxisAndTs(AxisAndTValue* axisAndTs);

   std::stack<CpuRaycastStackElement> _stack;


   gloost::mathType _tMin;
   gloost::mathType _tMax;

   Svo*             _svo;

   std::vector<gloost::Point3> _idToPositionLookUp;

   bool _verboseMode;

};


} // namespace svo


#endif // H_SVO_CPU_RAYCASTER_SINGLE_RAY


