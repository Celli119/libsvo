
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
#include <chrono>



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

SvoBuilderFaces::SvoBuilderFaces():
    _svo(0),
    _mesh(0),
    _matrixStack()
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
  auto t0 = std::chrono::high_resolution_clock::now();


  // seperate component data
  for (unsigned int i=0; i!=triangles.size(); ++i)
  {
    const BuilderTriangleFace triFace(_mesh, i);
//    if (triFace.getCenter()[0] > 0.0 && triFace.getCenter()[1] > 0.0)
    if (triFace.intersectAABB(svo->getBoundingBox()))
    {
      buildRecursive(0, triFace);
    }
  }

//  svo->normalizeLeafAttribs();
//  svo->generateInnerNodesAttributes(svo->getRootNode());


  auto t1 = std::chrono::high_resolution_clock::now();
  std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);

//#ifndef GLOOST_SYSTEM_DISABLE_OUTPUT_MESSAGES
  std::cerr << std::endl << "               Number of leaves:          " << _svo->getNumLeaves();
  std::cerr << std::endl << "               Number of nodes:           " << _svo->getNumNodes();
  std::cerr << std::endl << "               Number of OOB Points:      " << _svo->getNumOutOfBoundPoints();
  std::cerr << std::endl << "               Number of double Points:   " << _svo->getNumDoublePoints();
  std::cerr << std::endl << "               Octree memory real CPU:    " << _svo->getNumNodes()*sizeof(svo::SvoNode)/1024.0/1024.0 << " MB";
  std::cerr << std::endl << "               Build time:                " << duration.count()/1000.0 << " sec";
  std::cerr << std::endl;
  std::cerr << std::endl << "             Creating attributes for inner nodes: ";
  std::cerr << std::endl << "               Octree memory serialized:  " << _svo->getNumNodes()*svo::SvoNode::getSerializedNodeSize()/1024.0/1024.0 << " MB";
//  std::cerr << std::endl << "               Attribs memory serialized: " << _svo->getCurrentAttribPosition()*sizeof(float)/1024.0/1024.0 << " MB";
  std::cerr << std::endl << "               Number of one-child-nodes: " << _svo->getNumOneChildNodes() << " ( " << (100.0f*_svo->getNumOneChildNodes())/(float)_svo->getNumNodes() << " % )";
  std::cerr << std::endl << "               Number of one-child-nodes: " << _svo->getNumOneChildNodes() << " ( " << (100.0f*_svo->getNumOneChildNodes())/(float)_svo->getNumNodes() << " % )";
  std::cerr << std::endl;
//#endif


}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief builds the svo recursive from triangle faces
  \param ...
  \remarks ...
*/

void
SvoBuilderFaces::buildRecursive(unsigned int currentDepth, const BuilderTriangleFace& triangle)
{

  // max depth reached
  if ( currentDepth  == _svo->getMaxDepth())
  {
    gloost::BoundingBox bbox(gloost::Point3(-0.5,-0.5,-0.5),
                             gloost::Point3( 0.5, 0.5, 0.5));

    bbox.transform(_matrixStack.top());
    gloost::Point3 voxelCenter = _matrixStack.top() * gloost::Point3(0.0,0.0,0.0);


    // validate voxel by firing a ray in the direction of the triangles normal through the center
    // of the AABB. Only if it hits the triangle (and get a normal and a color)
    // the voxel will be inserted
    float u   = 0.0;
    float v   = 0.0;

    SvoNode* leafNode = 0;

    if (triangle.calculateUAndV( bbox.getCenter(), u, v))
    {
      leafNode = _svo->insert(voxelCenter);
    }

    if (leafNode)
    {
      // push attribs for a new leaf node
      if (leafNode->getAttribPosition() == SVO_SVONODE_EMPTY_ATTRIB_POS)
      {
        //
        leafNode->setAttribPosition(_svo->createDiscreteSampleList());
      }

//      if (_svo->getDiscreteSampleList(leafNode->getAttribPosition()).size() < 1) // allow only one sample
        _svo->getDiscreteSampleList(leafNode->getAttribPosition()).push_back(DiscreteSample(triangle._id, u, v));

    }
    return;
  }


  const float offset = 0.25;

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

        _matrixStack.push();
        {
          _matrixStack.translate(childOffset[0], childOffset[1], childOffset[2] );
          _matrixStack.scale(0.5);

          gloost::BoundingBox bbox(gloost::Point3(-0.5,-0.5,-0.5), gloost::Point3(0.5,0.5,0.5));

          bbox.transform(_matrixStack.top());

          if (triangle.intersectAABB(bbox))
          {
            buildRecursive(currentDepth+1,
                           triangle);
          }
        }
        _matrixStack.pop();

      }
    }
  }

}


///////////////////////////////////////////////////////////////////////////////









} // namespace svo

