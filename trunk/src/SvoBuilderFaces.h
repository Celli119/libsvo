
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



#ifndef H_SVO_SVO_BUILDER_FACES
#define H_SVO_SVO_BUILDER_FACES



/// svo system includes
#include <BuilderTriangleFace.h>


/// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>
#include <gloost/Mesh.h>
#include <gloost/Vector3.h>
#include <gloost/Point3.h>
#include <gloost/MatrixStack.h>


/// cpp includes
#include <string>
#include <boost/thread/mutex.hpp>

namespace svo
{

  class Svo;
  class SvoNode;


  //  generates the svo structure by discretising faces

class SvoBuilderFaces
{
	public:

    // class constructor
    SvoBuilderFaces();

    // class destructor
	  virtual ~SvoBuilderFaces();


    // generates the svo by discretising faces
	  Svo* build(Svo* svo, gloost::Mesh* mesh);

	protected:

	  void runThreadOnRange(unsigned threadId,
                          unsigned startIndex,
                          unsigned endIndex);

    // builds the svo recursive from triangle faces
    void buildRecursive(unsigned                   threadId,
                        unsigned                   currentDepth,
                        const BuilderTriangleFace& triangle);


    Svo*                _svo;
    gloost::Mesh*       _mesh;

    unsigned     _numBuildThreads;
    boost::mutex _modifySvoMutex;;

    std::vector<gloost::MatrixStack> _matrixStacks;
};


} // namespace svo


#endif // H_SVO_SVO_BUILDER_FACES


