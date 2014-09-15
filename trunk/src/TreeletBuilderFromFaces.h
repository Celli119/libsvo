
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



#ifndef H_SVO_TREELET_BUILDER_FROM_FACES
#define H_SVO_TREELET_BUILDER_FROM_FACES



// svo system includes
#include <Treelet.h>
#include <BuilderTriangleFace.h>


// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>
#include <gloost/Mesh.h>
#include <gloost/Vector3.h>
#include <gloost/Point3.h>
#include <gloost/MatrixStack.h>


// cpp includes
#include <string>
#include <memory>
#include <queue>

namespace svo
{
  class SvoNode;


  //  generates the svo structure by discretising faces

class TreeletBuilderFromFaces
{
	public:

    // class constructor
    TreeletBuilderFromFaces(unsigned maxDepth,
                            unsigned numThreads = 6);

    // class destructor
	  virtual ~TreeletBuilderFromFaces();



    // builds a treelet by discretising triangles using all triangles of a mesh
	  void build(Treelet* treelet, const std::shared_ptr<gloost::Mesh>& mesh);

    // builds a treelet by discretising triangles using initial QueueElement
	  void build(Treelet* treelet, const std::shared_ptr<gloost::Mesh>& mesh, const Treelet::QueueElement& initialQueueElement);



	protected:

    // builds the svo recursive from triangle faces
    void buildFromQueue();


    unsigned          _maxDepth;

    Treelet*          _treelet;
    std::shared_ptr<gloost::Mesh> _mesh;

    unsigned          _numBuildThreads;

    std::queue<Treelet::QueueElement> _queue;

//    std::map




};


} // namespace svo


#endif // H_SVO_TREELET_BUILDER_FROM_FACES


