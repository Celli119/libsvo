
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
#include <TreeletBuilderFromFaces.h>
#include <Treelet.h>
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
  \class TreeletBuilderFromFaces

  \brief generates Treelets from triangle faces

  \author Felix Weiszig
  \date   March 2011
  \remarks
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

TreeletBuilderFromFaces::TreeletBuilderFromFaces(unsigned treeletSizeInBytes,
                                                 unsigned numThreads):
    _treelet(0),
    _mesh(0)
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

TreeletBuilderFromFaces::~TreeletBuilderFromFaces()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief builds a treelet by discretising triangles using all triangles of a mesh
  \param mesh gloost::Mesh with normal and color for each vertice
  \remarks ...
*/

void
TreeletBuilderFromFaces::build(Treelet* treelet, gloost::Mesh* mesh)
{
  _treelet  = treelet;
  _mesh     = mesh;

  std::vector<gloost::TriangleFace>& triangles = mesh->getTriangles();
  std::vector<gloost::Point3>&       vertices  = mesh->getVertices();
  std::vector<gloost::Vector3>&      normals   = mesh->getNormals();
  std::vector<gloost::vec4>&         colors    = mesh->getColors();

  std::cerr << std::endl;
  std::cerr << std::endl << "Message from TreeletBuilderFromFaces::build(SvoBranch* svo, gloost::Mesh* mesh):";
  std::cerr << std::endl << "             Building Octree from triangle faces:";
  std::cerr << std::endl << "               max size           " << _treelet->getMaxSize() << " byte";
  std::cerr << std::endl << "               triangles.size():  " << triangles.size();
  std::cerr << std::endl;

  // choose all primitives as relevant

  Treelet::QueueElement queueElement;
  queueElement._aabbTransform = gloost::Matrix::createIdentity();
  queueElement._primitiveIds.resize(triangles.size());
  queueElement._nodeIndex = 0;

  for (unsigned i=0; i!=queueElement._primitiveIds.size(); ++i)
  {
    queueElement._primitiveIds[i] = i;
  }

  // push root
  _queue.push(queueElement);

  buildFromQueue();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief builds a treelet by discretising triangles using initial QueueElement
  \param mesh gloost::Mesh with unique normal and color for each vertice
  \remarks ...
*/

void
TreeletBuilderFromFaces::build(Treelet* treelet, gloost::Mesh* mesh, const Treelet::QueueElement& initialQueueElement)
{
  _treelet  = treelet;
  _mesh     = mesh;

  std::vector<gloost::TriangleFace>& triangles = mesh->getTriangles();
  std::vector<gloost::Point3>&       vertices  = mesh->getVertices();
  std::vector<gloost::Vector3>&      normals   = mesh->getNormals();
  std::vector<gloost::vec4>&         colors    = mesh->getColors();

  std::cerr << std::endl;
  std::cerr << std::endl << "Message from TreeletBuilderFromFaces::build(SvoBranch* svo, gloost::Mesh* mesh):";
  std::cerr << std::endl << "             Building Octree from triangle faces:";
  std::cerr << std::endl << "               max size           " << _treelet->getMaxSize() << " byte";
  std::cerr << std::endl << "               triangles.size():  " << initialQueueElement._primitiveIds.size();
  std::cerr << std::endl;

  // choose all primitives as relevant

//  Treelet::QueueElement queueElement;
//  queueElement._aabbTransform = gloost::Matrix::createIdentity();
//  queueElement._primitiveIds.resize(triangles.size());
//  queueElement._nodeIndex = 0;
//
//  for (unsigned i=0; i!=queueElement._primitiveIds.size(); ++i)
//  {
//    queueElement._primitiveIds[i] = i;
//  }

  // push root
  _queue.push(initialQueueElement);
  _queue.front()._nodeIndex = 0;

  buildFromQueue();
}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief builds the svo recursive from triangle faces
  \param ...
  \remarks ...
*/

void
TreeletBuilderFromFaces::buildFromQueue()
{

  static const float offset = 0.25;

  unsigned currentNodeIndex = 1;

  unsigned whileCounter = 0;

  while (_queue.size() && (currentNodeIndex<_treelet->getSerializedNodes().size()-8) )
  {
    whileCounter++;
    if (whileCounter % 100000 == 0)
    {
      std::cerr << std::endl << "  Build progress: " << (unsigned) (((float)currentNodeIndex/_treelet->getSerializedNodes().size())*100) << " %"
                             << " ( leaf count: " << _queue.size() << " )";
    }


    const Treelet::QueueElement& parentQueuedElement = _queue.front();
    std::vector<Treelet::QueueElement> _childQueueElements(8);

    // 8 possible children
    for (unsigned z=0; z!=2u; ++z)
    {
      for (unsigned y=0; y!=2u; ++y)
      {
        for (unsigned x=0; x!=2u; ++x)
        {
          unsigned childIndex = 4*x+2*y+z;

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

          // child voxel transformation
          gloost::Matrix aabbTransform = parentQueuedElement._aabbTransform
                                         * gloost::Matrix::createTransMatrix(childOffset[0], childOffset[1], childOffset[2]);
          aabbTransform = aabbTransform * gloost::Matrix::createScaleMatrix(0.5);

          _childQueueElements[childIndex]._aabbTransform = aabbTransform;
          _childQueueElements[childIndex]._depth         = parentQueuedElement._depth +1;


          gloost::BoundingBox bbox(gloost::Point3(-0.5,-0.5,-0.5), gloost::Point3(0.5,0.5,0.5));
          bbox.transform(aabbTransform);

          // test this possible leafe with all trinagles
          for (unsigned tId=0; tId!=parentQueuedElement._primitiveIds.size(); ++tId)
          {
            const BuilderTriangleFace triangle(_mesh, parentQueuedElement._primitiveIds[tId]);

            if (triangle.intersectAABB(bbox))
            {
              _childQueueElements[childIndex]._primitiveIds.push_back(parentQueuedElement._primitiveIds[tId]);
            }
          }

        }
      }
    }

    // update parent node and build serial svo structure by checking content of new QueueElement
    bool isFirstValidChild = true;
    for (unsigned childIndex=0; childIndex!=8; ++childIndex)
    {
      // use only children with triangles
      if (_childQueueElements[childIndex]._primitiveIds.size())
      {
        // update existing parent node so that the firstChildIndex is pointing to this node
        if (isFirstValidChild)
        {
          // set first child index of the parent
          _treelet->getSerializedNodes()[parentQueuedElement._nodeIndex].setFirstChildIndex(currentNodeIndex);
          isFirstValidChild = false;
        }

        // set child valid (Note: It's unknown for now if this child is a leaf)
        _treelet->getSerializedNodes()[parentQueuedElement._nodeIndex].setValidMaskFlag(childIndex, true);

        // set the location within the serialized svo to the queueElement
        _childQueueElements[childIndex]._nodeIndex       = currentNodeIndex++;
        _childQueueElements[childIndex]._idx             = childIndex;
        _childQueueElements[childIndex]._parentNodeIndex = parentQueuedElement._nodeIndex;

        // queue element for this child
        _queue.push(_childQueueElements[childIndex]);
      }
      else
      {
        // set child NOT valid (Note: It's unknown for now if this child is a leaf)
        _treelet->getSerializedNodes()[parentQueuedElement._nodeIndex].setValidMaskFlag(childIndex, false);
      }
    }

    // unqueue parent element
    _queue.pop();

  } // while (_queue.size())


  /* Finishing treelet by setting leaf node flags of parents of leafes to TRUE.
     Copy all QueueElements to the Treelets _leafQueueElements so that sub-Treelets
     can be build from them :-)
  */
  std::cerr << std::endl;
  std::cerr << std::endl << "  -> Finishing treelet with leaf count: " << _queue.size();


  std::map<unsigned, Treelet::QueueElement>& treeletLeafQueueElements = _treelet->getLeafQueueElements();

  while (_queue.size())
  {
    const Treelet::QueueElement& leafQueuedElement = _queue.front();
    _treelet->getSerializedNodes()[leafQueuedElement._parentNodeIndex].setLeafMaskFlag(leafQueuedElement._idx, true);

    treeletLeafQueueElements[leafQueuedElement._nodeIndex] = leafQueuedElement;

    _queue.pop();
  }

}



///////////////////////////////////////////////////////////////////////////////









} // namespace svo

