
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
#include <Treelet.h>
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

Treelet::Treelet(unsigned maxSizeInByte):
  _maxSize(maxSizeInByte),
  _numNodes(1),
  _numLeaves(1),
  _serializedNodes()
{
  unsigned maxNumNodes = _maxSize/sizeof(CpuSvoNode);
  unsigned actualSize  = maxNumNodes*sizeof(CpuSvoNode);

  std::cerr << std::endl;
  std::cerr << std::endl << "Creating Treelet";

  if (actualSize != _maxSize)
  {
    std::cerr << std::endl << "    Size of    : " << _maxSize << " Byte";
    std::cerr << std::endl << "    changed to : " << actualSize;
    _maxSize = actualSize;
  }

  std::cerr << std::endl << "Creating Treelet of size: " << _maxSize;
  std::cerr << std::endl << "     max number of nodes: " << maxNumNodes;

  _serializedNodes.resize(maxNumNodes);
}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

Treelet::Treelet(const std::string treeletFilePath):
  _maxSize(0),
  _numNodes(1),
  _numLeaves(0),
  _serializedNodes()
{
  loadTreeletFromFile(treeletFilePath);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

Treelet::~Treelet()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns a vector with serialized nodes
  \param ...
  \remarks ...
*/

std::vector<CpuSvoNode>&
Treelet::getSerializedNodes()
{
  return _serializedNodes;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns a serialized SvoNode for an index
  \param ...
  \remarks ...
*/

CpuSvoNode
Treelet::getSerializedNodeForIndex(unsigned index)
{
  return _serializedNodes[index];
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the max size in Bytes
  \param ...
  \remarks ...
*/

unsigned
Treelet::getMaxSize() const
{
  return _maxSize;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns number of nodes
  \param ...
  \remarks ...
*/

int
Treelet::getNumNodes() const
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
Treelet::getNumLeaves() const
{
  return _numLeaves;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief writes the serialized svo and attributes to a file
  \param ...
  \remarks ...
*/

bool
Treelet::writeTreeletToFile(const std::string& filePath)
{
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

  outfile.writeString(gloost::toString("v1") + " ");                // write version
//  outfile.writeString(gloost::toString(_maxSize) + " ");          // write maxDepth
  outfile.writeString(gloost::toString(_serializedNodes.size()) + " ");           // write numNodes
  outfile.writeBuffer((unsigned char*)&_serializedNodes.front(),    // write Nodes
                      _serializedNodes.size()*sizeof(CpuSvoNode));
//  outfile.writeBuffer((unsigned char*)&_serializedAttributeIndices.front(),    // write attribute indices
//                      _serializedAttributeIndices.size()*sizeof(unsigned));

  outfile.close();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief reads a serialized svo and attribute files
  \param ...
  \remarks ...
*/

bool
Treelet::loadTreeletFromFile(const std::string& filePath)
{
  std::cerr << std::endl;
  std::cerr << std::endl << "Message from SvoBranch::loadSerializedSvoToFile():";
  std::cerr << std::endl << "             Reading *.svo file";
  std::cerr << std::endl << "             " << filePath;

  gloost::BinaryFile infile;
  infile.openAndLoad(filePath);

  std::string versionString = infile.readWord();
  std::cerr << std::endl << "             version: " << versionString;

//  _maxDepth = atoi(infile.readWord().c_str());
//  std::cerr << std::endl << "             depth:   " << _maxDepth;

  _numNodes = atoi(infile.readWord().c_str());
  std::cerr << std::endl << "             nodes:   " << _numNodes;

  _serializedNodes.resize(_numNodes);
//  _serializedAttributeIndices.resize(_numNodes);

  gloost::unserialize((unsigned char*)&_serializedNodes.front(),
                      _numNodes*sizeof(CpuSvoNode),
                      infile);
//  gloost::unserialize((unsigned char*)&_serializedAttributeIndices.front(),
//                      _numNodes*sizeof(unsigned),
//                      infile);

  infile.unload();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns a map of QueueElement for each leafe assoziated with the leafes index within this Treelet
  \param   ...
  \remarks This map is only filled after building
*/

std::map<unsigned, Treelet::QueueElement>&
Treelet::getLeafQueueElements()
{
  return _leafQueueElements;
}


////////////////////////////////////////////////////////////////////////////////

} // namespace svo

