
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
#include <SvoBranch.h>
#include <CpuSvoNode.h>


/// gloost system includes
#include <gloost/TextureManager.h>
#include <gloost/BitMask.h>
#include <gloost/MatrixStack.h>
#include <gloost/BinaryFile.h>
#include <gloost/BinaryBundle.h>
#include <gloost/serializers.h>

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

SvoBranch::SvoBranch(int maxDepth):
  _root(new SvoNode()),
  _maxDepth(maxDepth),
  _boundingBox(gloost::Point3(-0.5, -0.5, -0.5), gloost::Point3(0.5, 0.5, 0.5)),
  _numNodes(1),
  _numLeaves(0),
  _numOutOfBoundPoints(0),
  _numDoublePoints(0),
  _numOneChildNodes(0),
  _discreteSamples(),
  _discreteSampleIndex(0),
  _serializedCpuSvoNodes()
{
//	createDiscreteSampleList();
}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

SvoBranch::SvoBranch(const std::string svoFilePath):
  _root(new SvoNode()),
  _maxDepth(0),
  _boundingBox(gloost::Point3(-0.5, -0.5, -0.5), gloost::Point3(0.5, 0.5, 0.5)),
  _numNodes(1),
  _numLeaves(0),
  _numOutOfBoundPoints(0),
  _numDoublePoints(0),
  _numOneChildNodes(0),
  _discreteSamples(),
  _discreteSampleIndex(0),
  _serializedCpuSvoNodes()
{
  loadSerializedSvoFromFile(svoFilePath);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

SvoBranch::~SvoBranch()
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
SvoBranch::insert(const gloost::Point3& point)
{

  // check point with bounding box of svo
//  for (unsigned i=0; i!=3; ++i)
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
SvoBranch::insertAndBuild(SvoNode*        currentParent,
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

  unsigned choosenChildIndex = 4*x+2*y+z;

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
  \brief serializes the svo in Breadth-first order
  \param ...
  \remarks ...
*/

unsigned
SvoBranch::serializeSvo()
{

  std::cerr << std::endl << "Message from SvoBranch::serializeSvo(): ";
  std::cerr << std::endl << "             serializing svo nodes structure";
  std::cerr << std::endl << "           " << getNumNodes() << " nodes to serialize";

  _serializedCpuSvoNodes.resize(getNumNodes());
  _serializedAttributeIndices.resize(getNumNodes());
  unsigned currentNodeIndex = 0;


  std::queue<QueuedNode> nodeQueue;

  QueuedNode queuedNode;
  queuedNode.node  = _root;
  queuedNode.index = currentNodeIndex;
  nodeQueue.push(queuedNode);


  // push root node to position 0
  CpuSvoNode cpuNode(0,
                     _root->getValidMask(),
                     _root->getLeafMask(),
                     _root->getAttribPosition());

  _serializedCpuSvoNodes[currentNodeIndex]      = cpuNode;  // child pointer (unknown for now)
  _serializedAttributeIndices[currentNodeIndex] = _root->getAttribPosition();
  ++currentNodeIndex;


  // add all nodes to the queue in a width search style
  while (nodeQueue.size())
  {
    // take a node from the queue
    QueuedNode currentParent = nodeQueue.front();
    nodeQueue.pop();

    unsigned childCount = 0;

    for (unsigned i=0; i!=8; ++i)
    {
      if (currentParent.node->getValidMask().getFlag(i))
      {
        ++childCount;

        // if this is the first child, put its position into parents childPointer field in the buffer
        if (childCount == 1)
        {
          _serializedCpuSvoNodes[currentParent.index].setFirstChildIndex(currentNodeIndex);
        }

        // queue current child
        SvoNode* child = currentParent.node->getChild(i);

        QueuedNode queuedChild;
        queuedChild.node  = child;
        queuedChild.index = currentNodeIndex;
        nodeQueue.push(queuedChild);

        // serialize current child
        CpuSvoNode cpuChildNode(0,                       // <- first child position unknown for now
                                child->getValidMask(),
                                child->getLeafMask(),
                                child->getAttribPosition());

        _serializedCpuSvoNodes[currentNodeIndex]      = cpuChildNode;
        _serializedAttributeIndices[currentNodeIndex] = child->getAttribPosition();
        ++currentNodeIndex;
      }
    }
  }
  std::cerr << std::endl;
  std::cerr << std::endl << "           number of nodes serialized: " << currentNodeIndex;
  std::cerr << std::endl;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief writes the serialized svo and attributes to a file
  \param ...
  \remarks ...
*/

bool
SvoBranch::writeSerializedSvoToFile(const std::string& filePath)
{

  if (!_serializedCpuSvoNodes.size())
  {
    serializeSvo();
  }

  std::cerr << std::endl;
  std::cerr << std::endl << "Message from SvoBranch::writeSerializedSvoToFile():";
  std::cerr << std::endl << "  Writing SVO to:";
  std::cerr << std::endl << "    " << filePath;

//  return gloost::BinaryFile::write(filePath, *_serializedSvoBundle);

  gloost::BinaryFile outfile;
  if (!outfile.openToWrite(filePath))
  {
    return false;
  }

  outfile.writeString(gloost::toString("v1") + " ");                      // write version
  outfile.writeString(gloost::toString(_maxDepth) + " ");                 // write maxDepth
  outfile.writeString(gloost::toString(_numNodes) + " ");                 // write numNodes
  outfile.writeBuffer((unsigned char*)&_serializedCpuSvoNodes.front(),    // write Nodes
                      _serializedCpuSvoNodes.size()*sizeof(CpuSvoNode));
  outfile.writeBuffer((unsigned char*)&_serializedAttributeIndices.front(),    // write attribute indices
                      _serializedAttributeIndices.size()*sizeof(unsigned));

  outfile.close();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief reads a serialized svo and attribute files
  \param ...
  \remarks ...
*/

bool
SvoBranch::loadSerializedSvoFromFile(const std::string& filePath)
{
  std::cerr << std::endl;
  std::cerr << std::endl << "Message from SvoBranch::loadSerializedSvoToFile():";
  std::cerr << std::endl << "             Reading *.svo file";
  std::cerr << std::endl << "             " << filePath;

  gloost::BinaryFile infile;
  infile.openAndLoad(filePath);

  std::string versionString = infile.readWord();
  std::cerr << std::endl << "             version: " << versionString;

  _maxDepth = atoi(infile.readWord().c_str());
  std::cerr << std::endl << "             depth:   " << _maxDepth;

  _numNodes = atoi(infile.readWord().c_str());
  std::cerr << std::endl << "             nodes:   " << _numNodes;

  _serializedCpuSvoNodes.resize(_numNodes);
  _serializedAttributeIndices.resize(_numNodes);

  gloost::unserialize((unsigned char*)&_serializedCpuSvoNodes.front(),
                      _numNodes*sizeof(CpuSvoNode),
                      infile);
  gloost::unserialize((unsigned char*)&_serializedAttributeIndices.front(),
                      _numNodes*sizeof(unsigned),
                      infile);

  infile.unload();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns a vector with serialized nodes
  \param ...
  \remarks ...
*/

std::vector<CpuSvoNode>&
SvoBranch::getSerializedNodes()
{
  return _serializedCpuSvoNodes;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns a serialized SvoNode for an index
  \param ...
  \remarks ...
*/

CpuSvoNode
SvoBranch::getSerializedNodeForIndex(unsigned index)
{
  return _serializedCpuSvoNodes[index];
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns a vector with serialized attribute indices
  \param ...
  \remarks ...
*/

std::vector<unsigned>&
SvoBranch::getSerializedAttributeIndices()
{
  return _serializedAttributeIndices;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief creates new set of samples for a leaf node and returns an id
  \param ...
  \remarks ...
*/

unsigned
SvoBranch::createDiscreteSampleList()
{
  _discreteSamples.push_back(std::vector<DiscreteSample>());
  return _discreteSamples.size()-1;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the vector of lists of DiscreteSamples
  \param ...
  \remarks ...
*/

std::vector< SvoBranch::SampleList >&
SvoBranch::getDiscreteSampleLists()
{
  return _discreteSamples;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns a lists of DiscreteSamples
  \param ...
  \remarks ...
*/

SvoBranch::SampleList&
SvoBranch::getDiscreteSampleList(unsigned id)
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
SvoBranch::getNumDiscreteSamples() const
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
SvoBranch::clearDiscreteSamples()
{
  unsigned freedMemory = 0;

  for (unsigned i=0; i!=_discreteSamples.size(); ++i)
  {
    freedMemory += _discreteSamples[i].size()*sizeof(DiscreteSample);
  }
  _discreteSamples.clear();
  _discreteSamples.resize(1);

  std::cerr << std::endl;
  std::cerr << std::endl << "Message from SvoBranch::clearDiscreteSamples(): ";
  std::cerr << std::endl << "  " << freedMemory/1024.0/1024.0 << " MB freed.";
}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the root node
  \param ...
  \remarks ...
*/

SvoNode*
SvoBranch::getRootNode()
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
SvoBranch::getBoundingBox() const
{
  return _boundingBox;
}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the depth of the svo
  \param ...
  \remarks ...
*/

unsigned
SvoBranch::getMaxDepth() const
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
SvoBranch::getNumNodes() const
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
SvoBranch::getNumLeaves() const
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
SvoBranch::getNumOutOfBoundPoints() const
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
SvoBranch::getNumDoublePoints() const
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
SvoBranch::getNumOneChildNodes() const
{
  return _numOneChildNodes;
}


////////////////////////////////////////////////////////////////////////////////

} // namespace svo

