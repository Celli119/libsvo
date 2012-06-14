
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
#include <Svo.h>
#include <CpuSvoNode.h>


/// gloost system includes
#include <gloost/TextureManager.h>
#include <gloost/BitMask.h>
#include <gloost/MatrixStack.h>
#include <gloost/BinaryFile.h>
#include <gloost/BinaryBundle.h>

/// cpp includes
#include <string>
#include <iostream>
#include <cmath>
#include <queue>



namespace svo
{

/**
  \class Svo

  \brief Sparse Voxel Octree with utility functions

  \author Felix Weiszig
  \date   Feb 2011
  \remarks ...
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

Svo::Svo(int maxDepth):
  _root(new SvoNode()),
  _maxDepth(maxDepth),
  _minVoxelSize(pow(2, -maxDepth)),
  _boundingBox(gloost::Point3(-0.5, -0.5, -0.5), gloost::Point3(0.5, 0.5, 0.5)),
  _numNodes(1),
  _numLeaves(0),
  _numOutOfBoundPoints(0),
  _numDoublePoints(0),
  _numOneChildNodes(0),
  _discreteSamples(),
  _discreteSampleIndex(0),
  _serializedCpuSvoNodes(),
  _serializedSvoBundle(0)
{
//	createDiscreteSampleList();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

Svo::~Svo()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief inserts a position in the octree and returns its leaf node
  \param ...
  \remarks point should be inside the SVOs bounding box
*/

SvoNode*
Svo::insert(const gloost::Point3& point)
{

  // check point with bounding box of svo
//  for (unsigned int i=0; i!=3; ++i)
//  {
//    if (point[i] < _boundingBox.getPMin()[i] || point[i] > _boundingBox.getPMax()[i])
//    {
//      ++_numOutOfBoundPoints;
//      return 0;
//    }
//  }


  return insertAndBuild(0,     // parent
                        0,     // current child index
                        _root, // current node
                        point, // point in current voxel space
                        0);    // depth within the svo
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief inserts a point in the svo, builds the tree
  \param ...
  \remarks ...
*/

SvoNode*
Svo::insertAndBuild(SvoNode*              currentParent,
                    int                   currentChildIndex,
                    SvoNode*              currentNode,
                    const gloost::Point3& localPos,
                    int                   currentDepth)
{

  // max depth reached
  if ( currentDepth  == _maxDepth)
  {
    if (currentParent->getLeafMask().getFlag(currentChildIndex) == 0)
    {
      ++_numLeaves;
      currentParent->getLeafMask().setFlag(currentChildIndex, true);
      return currentNode;
    }
    else
    {
      ++_numDoublePoints;
      return currentParent->getChild(currentChildIndex);
    }
  }

  bool x = (localPos[0] >= 0);
  bool y = (localPos[1] >= 0);
  bool z = (localPos[2] >= 0);

  unsigned int choosenChildIndex = 4*x+2*y+z;

  // add a child node on this index if there wasn't one
  if (!currentNode->getValidMask().getFlag(choosenChildIndex))
  {
    currentNode->getValidMask().setFlag(choosenChildIndex, true);
    currentNode->setChild(choosenChildIndex, new SvoNode());
    ++_numNodes;
  }

  float currentScale = pow(2, -(int)currentDepth);
  float offset       = 0.25*currentScale;


  /// calculate local position
  gloost::Point3 chosenChildLocalPos(localPos[0] - (x*(offset)) + (!x*(offset)),
                                     localPos[1] - (y*(offset)) + (!y*(offset)),
                                     localPos[2] - (z*(offset)) + (!z*(offset)));

  return insertAndBuild( currentNode,
                         choosenChildIndex,
                         currentNode->getChild(choosenChildIndex),
                         chosenChildLocalPos,
                         currentDepth+1);

}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief serializes the svo and returns TextureManager id of resulting texture
  \param ...
  \remarks ...
*/

unsigned int
Svo::serializeSvo()
{

  std::queue<QueuedNode> nodeQueue;

  QueuedNode queuedNode;
  queuedNode.node     = _root;
  queuedNode.position = _serializedCpuSvoNodes.size();
  nodeQueue.push(queuedNode);


  CpuSvoNode cpuNode(0,
                     _root->getValidMask(),
                     _root->getLeafMask(),
                     _root->getAttribPosition());

  _serializedCpuSvoNodes.push_back(cpuNode);                     // child pointer (unknown for now)



  // add all nodes to the queue in a width search style
  while (nodeQueue.size())
  {
    QueuedNode currentParent = nodeQueue.front();
    nodeQueue.pop();

    for (unsigned int i=0; i!=8; ++i)
    {
      int childCount = 0;

      if (currentParent.node->getValidMask().getFlag(i))
      {
        ++childCount;

        // if this is the first child, put its position into parents childPointer field in the buffer
        if (childCount == 1)
        {
          _serializedCpuSvoNodes[currentParent.position].setFirstChildIndex(_serializedCpuSvoNodes.size());
        }

        SvoNode* child = currentParent.node->getChild(i);

        QueuedNode queuedChild;
        queuedChild.node     = child;
        queuedChild.position = _serializedCpuSvoNodes.size();
        nodeQueue.push(queuedChild);

        CpuSvoNode cpuNode(0,
                           child->getValidMask(),
                           child->getLeafMask(),
                           child->getAttribPosition());

        _serializedCpuSvoNodes.push_back(cpuNode);

      }
    }
  }

  std::cerr << std::endl << "sizeof(CpuSvoNode): " << sizeof(CpuSvoNode);

  _serializedSvoBundle = new gloost::BinaryBundle((unsigned char*)&_serializedCpuSvoNodes.front(), _serializedCpuSvoNodes.size()*sizeof(CpuSvoNode));
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief writes the serialized svo and attributes to a file
  \param ...
  \remarks ...
*/

bool
Svo::writeSerializedSvoToFile(const std::string& filePath)
{

  if (!_serializedSvoBundle)
  {
    serializeSvo();
  }

  std::cerr << std::endl;
  std::cerr << std::endl << "Message from Svo::writeSerializedSvoToFile():";
  std::cerr << std::endl << "  Writing SVO to:";
  std::cerr << std::endl << "    " << filePath;

//  return gloost::BinaryFile::write(filePath, *_serializedSvoBundle);

  gloost::BinaryFile outfile;
  if (!outfile.openToWrite(filePath))
  {
    return false;
  }

  std::cerr << std::endl << "_maxDepth: " << gloost::toString(_maxDepth);

  outfile.writeString(gloost::toString(_maxDepth) + " ");                 // write maxDepth
  outfile.writeString(gloost::toString(_numNodes) + " ");                 // write numNodes
  outfile.writeBuffer((unsigned char*)&_serializedCpuSvoNodes.front(),    // write Nodes
                      _serializedCpuSvoNodes.size()*sizeof(CpuSvoNode));

  outfile.close();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief creates new set of samples for a leaf node and returns an id
  \param ...
  \remarks ...
*/

unsigned
Svo::createDiscreteSampleList()
{
  _discreteSamples.push_back(std::list<DiscreteSample>() );
  return _discreteSamples.size()-1;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the vector of lists of DiscreteSamples
  \param ...
  \remarks ...
*/

std::vector< Svo::SampleList >&
Svo::getDiscreteSampleLists()
{
  return _discreteSamples;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns a lists of DiscreteSamples
  \param ...
  \remarks ...
*/

Svo::SampleList&
Svo::getDiscreteSampleList(unsigned id)
{
  return _discreteSamples[id];
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the number of discrete samples currently stored
  \param ...
  \remarks ...
*/

unsigned
Svo::getNumDiscreteSamples() const
{

  unsigned count = 0;

  for (unsigned i=0; i!=_discreteSamples.size(); ++i)
  {
    count += _discreteSamples[i].size();
  }

  return count;
}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief clears all discrete samples to save some mem
  \param ...
  \remarks ...
*/

void
Svo::clearDiscreteSamples()
{
  unsigned freedMemory = 0;

  for (unsigned i=0; i!=_discreteSamples.size(); ++i)
  {
    freedMemory += _discreteSamples[i].size()*sizeof(DiscreteSample);
    _discreteSamples[i].clear();
  }
  _discreteSamples.clear();

  std::cerr << std::endl;
  std::cerr << std::endl << "Message from Svo::clearDiscreteSamples(): ";
  std::cerr << std::endl << "  " << freedMemory/1024.0/1024.0 << " MB freed.";
}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns current attribute index;
  \param ...
  \remarks ...
*/

//unsigned int
//Svo::getCurrentAttribPosition() const
//{
//  return _attributeBuffer.size();
//}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the attribute buffer
  \param ...
  \remarks ...
*/

//std::vector<float>&
//Svo::getAttributeBuffer()
//{
//  return _attributeBuffer;
//}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief adds a element to the nomralizer vector
  \param ...
  \remarks ...
*/

//void
//Svo::pushNormalizer()
//{
//  _attribNormalizers.push_back(1);
//}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief increases the node count within the normalizer to normalize attribs for double voxels
  \param ...
  \remarks ...
*/

//void
//Svo::addDoubleNodeToNormalizer(unsigned attribPos)
//{
//  ++_attribNormalizers[attribPos];
//}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the root node
  \param ...
  \remarks ...
*/

SvoNode*
Svo::getRootNode()
{
  return _root;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the BoundingBox of the Svo
  \param ...
  \remarks
*/

const gloost::BoundingBox&
Svo::getBoundingBox() const
{
  return _boundingBox;
}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the depth of the svo
  \param ...
  \remarks ...
*/

unsigned int
Svo::getMaxDepth() const
{
  return _maxDepth;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns number of nodes
  \param ...
  \remarks ...
*/

int
Svo::getNumNodes() const
{
  return _numNodes;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns number of leafes
  \param ...
  \remarks ...
*/

int
Svo::getNumLeaves() const
{
  return _numLeaves;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns number of points that didn't make it in the svo because they where out of bound
  \param ...
  \remarks ...
*/

int
Svo::getNumOutOfBoundPoints() const
{
  return _numOutOfBoundPoints;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns number of points that didn't make it in the svo because of there was allready one in the tree
  \param ...
  \remarks ...
*/

int
Svo::getNumDoublePoints() const
{
  return _numDoublePoints;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns number nodes with only one child
  \param ...
  \remarks ...
*/

int
Svo::getNumOneChildNodes() const
{
  return _numOneChildNodes;
}


////////////////////////////////////////////////////////////////////////////////

} // namespace svo


