
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



/// svo system includes
#include <SvoBuilderFaces.h>
#include <Svo.h>
//#include <BuilderTriangleFace.h>


/// gloost system includes
#include <gloost/TextureManager.h>
#include <gloost/MatrixStack.h>


/// cpp includes
#include <string>
#include <iostream>
#include <boost/thread.hpp>
//#include <chrono>



namespace svo
{

/**
  \class SvoBuilderFaces

  \brief generates the svo structure by discretising faces

  \author Felix Weiszig
  \date   March 2011
  \remarks
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

SvoBuilderFaces::SvoBuilderFaces(unsigned numThreads):
    _svo(0),
    _mesh(0),
    _numBuildThreads(numThreads),
    _modifySvoMutex(),
    _matrixStacks(_numBuildThreads, gloost::MatrixStack())
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

SvoBuilderFaces::~SvoBuilderFaces()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief builds the svo structure from a mesh using the vertices
  \param mesh gloost::Mesh with unique normal and color for each vertice
  \remarks ...
*/

Svo*
SvoBuilderFaces::build(Svo* svo, gloost::Mesh* mesh)
{
  _svo  = svo;
  _mesh = mesh;

  std::vector<gloost::TriangleFace>& triangles = mesh->getTriangles();
  std::vector<gloost::Point3>&       vertices  = mesh->getVertices();
  std::vector<gloost::Vector3>&      normals   = mesh->getNormals();
  std::vector<gloost::vec4>&         colors    = mesh->getColors();

//#ifndef GLOOST_SYSTEM_DISABLE_OUTPUT_MESSAGES
    std::cerr << std::endl;
    std::cerr << std::endl << "Message from SvoBuilderFaces::build(Svo* svo, gloost::Mesh* mesh):";
    std::cerr << std::endl << "             Building Octree from triangle faces:";
    std::cerr << std::endl << "               max depth                  " << svo->getMaxDepth();
    std::cerr << std::endl << "               min voxelsize              " << pow(2, -(int)_svo->getMaxDepth());
    std::cerr << std::endl << "               resolution                 " << pow(2, (int)_svo->getMaxDepth());
    std::cerr << std::endl << "               triangles.size():          " << triangles.size();
//#endif

  // take start time
//  auto t0 = std::chrono::high_resolution_clock::now();


  unsigned range = triangles.size()/_numBuildThreads;

  boost::thread_group threadGroup;

  // seperate component data
  for (unsigned int t=0; t!=_numBuildThreads; ++t)
  {
    threadGroup.create_thread(boost::bind(&SvoBuilderFaces::runThreadOnRange, this, t, t*range, (t+1)*range ));
  }

  std::cerr << std::endl;
  std::cerr << std::endl << "starting build threads: [" << gloost::repeatString(" ", _numBuildThreads) << "]";
  std::cerr << std::endl << "              patience: [";

  threadGroup.join_all();

//  auto t1 = std::chrono::high_resolution_clock::now();
//  std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);

//#ifndef GLOOST_SYSTEM_DISABLE_OUTPUT_MESSAGES
  std::cerr << std::endl;
  std::cerr << std::endl << "               Number of leaves:          " << _svo->getNumLeaves();
  std::cerr << std::endl << "               Number of nodes:           " << _svo->getNumNodes();
  std::cerr << std::endl << "               Number of OOB Points:      " << _svo->getNumOutOfBoundPoints();
  std::cerr << std::endl << "               Number of double Points:   " << _svo->getNumDoublePoints();
  std::cerr << std::endl << "               Octree memory real CPU:    " << _svo->getNumNodes()*sizeof(svo::SvoNode)/1024.0/1024.0 << " MB";
  std::cerr << std::endl << "               Discrete samples count:    " << _svo->getNumDiscreteSamples();
  std::cerr << std::endl << "               Discrete samples memory:   " << _svo->getNumDiscreteSamples()*sizeof(DiscreteSample)/1024.0/1024.0 << " MB";
  std::cerr << std::endl << "               Octree memory serialized:  " << _svo->getNumNodes()*svo::SvoNode::getSerializedNodeSize()/1024.0/1024.0 << " MB";
//  std::cerr << std::endl << "               Attribs memory serialized: " << _svo->getCurrentAttribPosition()*sizeof(float)/1024.0/1024.0 << " MB";
  std::cerr << std::endl << "               Number of one-child-nodes: " << _svo->getNumOneChildNodes() << " ( " << (100.0f*_svo->getNumOneChildNodes())/(float)_svo->getNumNodes() << " % )";
  std::cerr << std::endl;
//#endif


}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief ...
  \param ...
  \remarks ...
*/

void
SvoBuilderFaces::runThreadOnRange(unsigned threadId,
                                  unsigned startIndex,
                                  unsigned endIndex)
{
  // seperate component data
  for (unsigned i = startIndex; i!=endIndex; ++i)
  {
    const BuilderTriangleFace triFace(_mesh, i);
//    if (triFace.intersectAABB(_svo->getBoundingBox()))
    {
      buildRecursive(threadId, 0, triFace);
    }
  }

  std::cerr << '|';

}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief builds the svo recursive from triangle faces
  \param ...
  \remarks ...
*/

void
SvoBuilderFaces::buildRecursive(unsigned                   threadId,
                                unsigned                   currentDepth,
                                const BuilderTriangleFace& triangle)
{

  // max depth reached
  if ( currentDepth  == _svo->getMaxDepth())
  {
    gloost::BoundingBox bbox(gloost::Point3(-0.5,-0.5,-0.5),
                             gloost::Point3( 0.5, 0.5, 0.5));

    bbox.transform(_matrixStacks[threadId].top());

    // validate voxel by firing a ray in the direction of the triangles normal through the center
    // of the AABB. Only if it hits the triangle (and get a normal and a color)
    // the voxel will be inserted
    float u   = 0.0;
    float v   = 0.0;

    SvoNode* leafNode = 0;

    if (triangle.calculateUAndV( bbox.getCenter(), u, v))
    {
      _modifySvoMutex.lock();
      {
        leafNode = _svo->insert(bbox.getCenter());

        // push attribs for a new leaf node
        if (leafNode->getAttribPosition() == SVO_EMPTY_ATTRIB_POS)
        {
          leafNode->setAttribPosition(_svo->createDiscreteSampleList());
        }

        _svo->getDiscreteSampleList(leafNode->getAttribPosition()).push_back(DiscreteSample(triangle._id, u, v));
      }
      _modifySvoMutex.unlock();
    }

    return;
  }


  static const float offset = 0.25;

  for (int z=0; z!=2; ++z)
  {
    for (int y=0; y!=2; ++y)
    {
      for (int x=0; x!=2; ++x)
      {
        int choosenChildIndex = 4*x+2*y+z;

        gloost::Vector3 childOffset(0.0,0.0,0.0);

        if (x)
        {
          childOffset[0] += offset;
        }
        else
        {
          childOffset[0] -= offset;
        }

        if (y)
        {
          childOffset[1] += offset;
        }
        else
        {
          childOffset[1] -= offset;
        }

        if (z)
        {
          childOffset[2] += offset;
        }
        else
        {
          childOffset[2] -= offset;
        }

        _matrixStacks[threadId].push();
        {
          _matrixStacks[threadId].translate(childOffset[0], childOffset[1], childOffset[2] );
          _matrixStacks[threadId].scale(0.5);

          gloost::BoundingBox bbox(gloost::Point3(-0.5,-0.5,-0.5), gloost::Point3(0.5,0.5,0.5));

          bbox.transform(_matrixStacks[threadId].top());

          if (triangle.intersectAABB(bbox))
          {
            buildRecursive(threadId,
                           currentDepth+1,
                           triangle);
          }
        }
        _matrixStacks[threadId].pop();

      }
    }
  }

}


///////////////////////////////////////////////////////////////////////////////









} // namespace svo


