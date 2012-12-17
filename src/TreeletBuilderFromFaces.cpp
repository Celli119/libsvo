
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



// svo system includes
#include <TreeletBuilderFromFaces.h>
#include <Treelet.h>
//#include <BuilderTriangleFace.h>


// gloost system includes
#include <gloost/TextureManager.h>
#include <gloost/MatrixStack.h>


// cpp includes
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

//////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

TreeletBuilderFromFaces::TreeletBuilderFromFaces(unsigned maxDepth,
                                                 unsigned numThreads):
    _maxDepth(maxDepth),
    _treelet(0),
    _mesh(0)
{
	// insert your code here
}


//////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

TreeletBuilderFromFaces::~TreeletBuilderFromFaces()
{
	// insert your code here
}


//////////////////////////////////////////////////////


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
  std::cerr << std::endl << "               max size           " << _treelet->getMemSize() << " byte";
  std::cerr << std::endl << "               triangles.size():  " << triangles.size();
  std::cerr << std::endl;

  // choose all primitives as relevant

  Treelet::QueueElement queueElement;
  queueElement._aabbTransform = gloost::Matrix::createIdentity();
  queueElement._primitiveIds.resize(triangles.size());
  queueElement._localLeafIndex = 0;

  for (unsigned i=0; i!=queueElement._primitiveIds.size(); ++i)
  {
    queueElement._primitiveIds[i] = i;
  }

  // push root
  _queue.push(queueElement);

  buildFromQueue();
}


//////////////////////////////////////////////////////


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


#ifdef SVO_BUILDING_VERBOSE
  std::cerr << std::endl;
  std::cerr << std::endl << "Message from TreeletBuilderFromFaces::build(SvoBranch* svo, gloost::Mesh* mesh):";
  std::cerr << std::endl << "             Building Octree from triangle faces:";
  std::cerr << std::endl << "               max size       " << _treelet->getMemSize() << " byte";
  std::cerr << std::endl << "               num triangles: " << initialQueueElement._primitiveIds.size();
  std::cerr << std::endl;
#endif

  // push root
  _queue.push(initialQueueElement);
  _queue.front()._localLeafIndex = 0;

  buildFromQueue();
}

//////////////////////////////////////////////////////


/**
  \brief builds the svo recursive from triangle faces
  \param ...
  \remarks ...
*/

void
TreeletBuilderFromFaces::buildFromQueue()
{

  static const float offset               = 0.25f;
  static const gloost::Matrix scaleToHalf = gloost::Matrix::createScaleMatrix(0.5f);
  float childOffsetX;
  float childOffsetY;
  float childOffsetZ;

  unsigned currentNodeIndex = 1u;
  unsigned whileCounter     = 0u;


  // as long there is space within this treelet
  while (_queue.size() && (currentNodeIndex < _treelet->getNodes().size()-7) )
  {

//#ifdef SVO_BUILDING_VERBOSE
    whileCounter++;
    if (whileCounter % 100000 == 0)
    {
      std::cerr << std::endl << "  Build progress: " << (unsigned) (((float)currentNodeIndex/_treelet->getNodes().size())*100) << " %"
                             << " ( leaf count: " << _queue.size()  << " )";
    }
//#endif

    const Treelet::QueueElement& parentQueuedElement = _queue.front();
    std::vector<Treelet::QueueElement> childQueueElements(8);

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
            childOffsetX = offset;
          }
          else
          {
            childOffsetX = -offset;
          }

          if (y)
          {
            childOffsetY = offset;
          }
          else
          {
            childOffsetY = -offset;
          }

          if (z)
          {
            childOffsetZ = offset;
          }
          else
          {
            childOffsetZ = -offset;
          }

          // child voxel transformation
          gloost::Matrix aabbTransform = parentQueuedElement._aabbTransform
                                         * gloost::Matrix::createTransMatrix(childOffsetX, childOffsetY, childOffsetZ);
          aabbTransform = aabbTransform * gloost::Matrix::createScaleMatrix(0.5);

          childQueueElements[childIndex]._aabbTransform = aabbTransform;
          childQueueElements[childIndex]._depth         = parentQueuedElement._depth +1;

          gloost::BoundingBox bbox(gloost::Point3(-0.5,-0.5,-0.5), gloost::Point3(0.5,0.5,0.5));
          bbox.transform(aabbTransform);

          // test this possible leafe with all triagles
          for (unsigned tId=0; tId!=parentQueuedElement._primitiveIds.size(); ++tId)
          {
            const BuilderTriangleFace triangle(_mesh, parentQueuedElement._primitiveIds[tId]);

            if (triangle.intersectAABB(bbox))
            {
              childQueueElements[childIndex]._primitiveIds.push_back(parentQueuedElement._primitiveIds[tId]);
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
      if (childQueueElements[childIndex]._primitiveIds.size())
      {
        // update existing parent node so that the firstChildIndex is pointing to this node
        if (isFirstValidChild)
        {
          // set first child index of the parent, this is the relative distance within this Treelet
          _treelet->getNodes()[parentQueuedElement._localLeafIndex].setFirstChildIndex((int)currentNodeIndex-(int)parentQueuedElement._localLeafIndex);
          isFirstValidChild = false;
        }

        // set child valid (Note: It's unknown for now if this child is a leaf)
        _treelet->getNodes()[parentQueuedElement._localLeafIndex].setValidMaskFlag(childIndex, true);

        // set the location within the serialized svo to the queueElement
        childQueueElements[childIndex]._localLeafIndex       = currentNodeIndex;
        childQueueElements[childIndex]._idx                  = childIndex;
        childQueueElements[childIndex]._parentLocalNodeIndex = parentQueuedElement._localLeafIndex;

        // queue children
        // queue children
        {
          _queue.push(childQueueElements[childIndex]);
        }

        ++currentNodeIndex;
      }
      else
      {
        // set child NOT valid (Note: It's unknown for now if this child is a leaf)
        _treelet->getNodes()[parentQueuedElement._localLeafIndex].setValidMaskFlag(childIndex, false);
      }
    }

    // unqueue parent element
    _queue.pop();

  } // while (_queue.size())


  /* Finishing treelet by setting leaf node flags of parents of leafes to TRUE.
     Copy all QueueElements to the Treelets _leafQueueElements so that sub-Treelets
     can be build from them :-)
  */

#ifdef SVO_BUILDING_VERBOSE
  std::cerr << std::endl;
  std::cerr << std::endl << "  -> Finishing Treelet with leaf count: " << _queue.size();
#endif

  _treelet->setNumNodes((int)currentNodeIndex-1);
  _treelet->setNumLeaves(_queue.size());



#ifdef SVO_BUILDING_VERBOSE
  unsigned maxLeafDepth = 0;
  unsigned minLeafDepth = 1000;
#endif


  while (_queue.size())
  {
    const Treelet::QueueElement& leafQueuedElement = _queue.front();

    // set the leaf flag within leafes parent node
    _treelet->getNodes()[leafQueuedElement._parentLocalNodeIndex].setLeafMaskFlag(leafQueuedElement._idx, true);

#ifdef SVO_BUILDING_VERBOSE
    maxLeafDepth = gloost::max(maxLeafDepth, leafQueuedElement._depth);
    minLeafDepth = gloost::min(minLeafDepth, leafQueuedElement._depth);
#endif


#if 1
    if (leafQueuedElement._depth < _maxDepth)
    {
      _treelet->getIncompleteLeafQueueElements().push_back(leafQueuedElement);
    }
    else
    {
      _treelet->getFinalLeafQueueElements().push_back(leafQueuedElement);
    }
#endif
    _queue.pop();
  }

#ifdef SVO_BUILDING_VERBOSE
  std::cerr << std::endl << "       leafes with depth < maxDepth: " << _treelet->getLeafQueueElements().size();
  std::cerr << std::endl << "       min leaf depth                " << minLeafDepth;
  std::cerr << std::endl << "       max leaf depth                " << maxLeafDepth;
#endif
}



/////////////////////////////////////////////////////









} // namespace svo


