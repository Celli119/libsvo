
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



#ifndef H_SVO_CPU_RAYCASTER_SINGLE_RAY4
#define H_SVO_CPU_RAYCASTER_SINGLE_RAY4



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

  //  CpuRaycasterSingleRay4

class CpuRaycasterSingleRay4
{
	public:

    // class constructor
    CpuRaycasterSingleRay4(bool verboseMode = false);

    // class destructor
	  virtual ~CpuRaycasterSingleRay4();


    // traverses a Svo with a single ray
//	  SvoNode* start(const gloost::Ray& ray, Svo* svo);
	  virtual CpuSvoNode start(const gloost::Ray& ray, Svo* svo);

    // traverses a Svo with a single ray
//	  SvoNode* traversSvo(const gloost::Ray& ray);
	  virtual CpuSvoNode traversSvo(SvoNode* rootNode, gloost::Point3 p, gloost::Vector3 d);




    unsigned _pushCounter;
    unsigned _popCounter;
    unsigned _whileCounter;


    struct StackElement
    {
      StackElement()
      {
        _parent = CpuSvoNode();
        _t_max  = 0;
      }
      StackElement(CpuSvoNode parent, float t_max)
      {
        _parent = parent;
        _t_max  = t_max;
      }

      CpuSvoNode _parent;
      float      _t_max;

    };


    struct Stack
    {
      Stack(unsigned maxDepth)
      {
        _stack.resize(maxDepth);
      }

      void write(unsigned scale, CpuSvoNode parent, float t_max)
      {
        std::cerr << std::endl << "Stack::write at: " << scale;
        std::cerr << std::endl;
        _stack[scale]._parent = parent;
        _stack[scale]._t_max  = t_max;
      }


      CpuSvoNode read(unsigned scale, float& t_max)
      {
        std::cerr << std::endl << "Stack::read at:  " << scale;
        std::cerr << std::endl;

        t_max = _stack[scale]._t_max;
        return _stack[scale]._parent;
      }

      std::vector<StackElement> _stack;


    };


	private:

	  Svo* _svo;


};


} // namespace svo


#endif // H_SVO_CPU_RAYCASTER_SINGLE_RAY4


