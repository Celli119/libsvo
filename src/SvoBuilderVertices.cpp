
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
#include <SvoBuilderVertices.h>
#include <Svo.h>


/// gloost system includes
#include <TextureManager.h>


/// cpp includes
#include <string>
#include <iostream>
#include <chrono>



namespace svo
{

/**
  \class SvoBuilderVertices

  \brief generates the svo structure from a mesh using the vertices

  \author Felix Weiszig
  \date   March 2011
  \remarks
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

SvoBuilderVertices::SvoBuilderVertices()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

SvoBuilderVertices::~SvoBuilderVertices()
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
SvoBuilderVertices::build(Svo* svo, gloost::Mesh* mesh)
{

  std::vector<gloost::Point3>&       vertices  = mesh->getVertices();
  std::vector<gloost::Vector3>&      normals   = mesh->getNormals();
  std::vector<gloost::vec4>&         colors    = mesh->getColors();

#ifndef GLOOST_SYSTEM_DISABLE_OUTPUT_MESSAGES
    std::cerr << std::endl;
    std::cerr << std::endl << "Message from Svo::buildFromMeshVertices(gloost::Mesh* mesh):";
    std::cerr << std::endl << "             Building Octree from mesh vertices:";
    std::cerr << std::endl << "               max depth                  " << svo->getMaxDepth();
    std::cerr << std::endl << "               min voxelsize              " << pow(2, -(int)svo->getMaxDepth());
    std::cerr << std::endl << "               resolution                 " << pow(2, (int)svo->getMaxDepth());
    std::cerr << std::endl << "               vertices.size():           " << vertices.size();
#endif



  auto t0 = std::chrono::high_resolution_clock::now();



  const gloost::BoundingBox& svoBoundingBox = svo->getBoundingBox();

  // for seperate component data
  for (unsigned int i=0; i!=vertices.size(); ++i)
  {

    bool inside = true;

    for (unsigned int c=0; c!=3; ++c)
    {
      if (vertices[i][c] < svoBoundingBox.getPMin()[c] || vertices[i][c] > svoBoundingBox.getPMax()[c])
      {
        inside = false;
        break;
      }
    }

    if (inside)
    {
      svo::SvoNode* newNode = svo->insert(vertices[i]);

      // push new leaf attributes to the attribute buffer
      if (newNode)
      {
//        newNode->setAttribPosition(svo->getCurrentAttribPosition());
//
//        svo->pushAttributeComponent(normals[i][0]);
//        svo->pushAttributeComponent(normals[i][1]);
//        svo->pushAttributeComponent(normals[i][2]);
//
//        svo->pushAttributeComponent(colors[i].r);
//        svo->pushAttributeComponent(colors[i].g);
//        svo->pushAttributeComponent(colors[i].b);
      }
    }



  }


  // for interleaved data
  std::vector<float>& interleavedComponents = mesh->getInterleavedAttributes();

  if (interleavedComponents.size())
  {
    // size of one interleaved vertex + attributes
    int indexStepWidth = mesh->getInterleavedInfo().interleavedPackageSize; // add size of vertex
    int vertexPos      = mesh->getInterleavedInfo().interleavedVertexOffset;
    int normalPos      = mesh->getInterleavedInfo().interleavedNormalOffset;
    int colorPos       = mesh->getInterleavedInfo().interleavedColorOffset;


    // positions
    for (unsigned int i=vertexPos; i<interleavedComponents.size(); )
    {

      gloost::Point3 vertex(interleavedComponents[i],    // x
                            interleavedComponents[i+1],  // y
                            interleavedComponents[i+2]); // z


      svo::SvoNode* newNode = svo->insert(vertex);

      // push new leaf attributes to the attribute buffer
      if (newNode)
      {
//        newNode->setAttribPosition(svo->getCurrentAttribPosition());
//
//        gloost::Vector3 normal(interleavedComponents[i+3],  // x
//                               interleavedComponents[i+4],  // y
//                               interleavedComponents[i+5]); // z
//
//        svo->pushAttributeComponent(normal[0]);
//        svo->pushAttributeComponent(normal[1]);
//        svo->pushAttributeComponent(normal[2]);
//
//
//        gloost::Vector3 color(interleavedComponents[i+6],  // x
//                              interleavedComponents[i+7],  // y
//                              interleavedComponents[i+8]); // z
//
//        svo->pushAttributeComponent(color[0]);
//        svo->pushAttributeComponent(color[1]);
//        svo->pushAttributeComponent(color[2]);
      }

      i += indexStepWidth;
    }

  }

  svo->generateInnerNodesAttributes(svo->getRootNode());


  auto t1 = std::chrono::high_resolution_clock::now();
  std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);

#ifndef GLOOST_SYSTEM_DISABLE_OUTPUT_MESSAGES
  std::cerr << std::endl << "               Number of leaves:          " << svo->getNumLeaves();
  std::cerr << std::endl << "               Number of nodes:           " << svo->getNumNodes();
  std::cerr << std::endl << "               Number of OOB Points:      " << svo->getNumOutOfBoundPoints();
  std::cerr << std::endl << "               Number of double Points:   " << svo->getNumDoublePoints() << " ( " << (100.0f*svo->getNumDoublePoints())/(float)vertices.size() << " % )";
  std::cerr << std::endl << "               Octree memory real CPU:    " << svo->getNumNodes()*sizeof(svo::SvoNode)/1024.0/1024.0 << " MB";
  std::cerr << std::endl << "               Build time:                " << duration.count()/1000.0 << " sec";
  std::cerr << std::endl;
  std::cerr << std::endl << "             Creating attributes for inner nodes: ";
#endif


#ifndef GLOOST_SYSTEM_DISABLE_OUTPUT_MESSAGES
  std::cerr << std::endl << "               Octree memory serialized:  " << svo->getNumNodes()*svo::SvoNode::getSerializedNodeSize()/1024.0/1024.0 << " MB";
//  std::cerr << std::endl << "               Attribs memory serialized: " << svo->getCurrentAttribPosition()*sizeof(float)/1024.0/1024.0 << " MB";
  std::cerr << std::endl << "               Number of one-child-nodes: " << svo->getNumOneChildNodes() << " ( " << (100.0f*svo->getNumOneChildNodes())/(float)svo->getNumNodes() << " % )";
  std::cerr << std::endl;

#endif


}




////////////////////////////////////////////////////////////////////////////////





} // namespace svo


