
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

Treelet::Treelet():
  _treeletGid(0),
  _parentTreeletGid(0),
  _parentTreeletLeafPosition(0),
  _memSize(0),
  _numNodes(0),
  _numLeaves(0),
  _serializedNodes(),
  _incoreSlotGid(0)
{

}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor
  \param treeletGid the Gid of this Treelet
  \param treeletGid the Gid of the parent Treelet
  \param treeletGid the index of the parents Treelets leaf,
         which correspons with this treelets root node
  \param memSizeInByte size in bytes for this treelet
  \remarks
*/

Treelet::Treelet( gloost::gloostId  treeletGid,
                  gloost::gloostId  parentTreeletGid,
                  gloost::gloostId  parentTreeletLeafPosition,
                  unsigned          memSizeInByte):
  _treeletGid(treeletGid),
  _parentTreeletGid(parentTreeletGid),
  _parentTreeletLeafPosition(parentTreeletLeafPosition),
  _memSize(memSizeInByte),
  _numNodes(0),
  _numLeaves(0),
  _serializedNodes(),
  _incoreSlotGid(0)
{
  unsigned maxNumNodes = _memSize/sizeof(CpuSvoNode);
  unsigned actualSize  = maxNumNodes*sizeof(CpuSvoNode);

  std::cerr << std::endl;
  std::cerr << std::endl << "Creating Treelet";
  std::cerr << std::endl << "    Gid        :           " << _treeletGid;
  std::cerr << std::endl << "    parent Gid :           " << _parentTreeletGid;
  std::cerr << std::endl << "    parent leaf position : " << parentTreeletLeafPosition;

  if (actualSize != _memSize)
  {
    std::cerr << std::endl << "    Size of    : " << _memSize << " Byte";
    std::cerr << std::endl << "    changed to : " << actualSize;
    _memSize = actualSize;
  }

  std::cerr << std::endl << "     size:                 " << _memSize << " (" << (float)_memSize/1024/1024 << " MB)";
  std::cerr << std::endl << "     max number of nodes:  " << maxNumNodes;

  _serializedNodes.resize(maxNumNodes);
}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

Treelet::Treelet(const std::string treeletFilePath):
  _treeletGid(0),
  _parentTreeletGid(0),
  _parentTreeletLeafPosition(0),
  _memSize(0),
  _numNodes(0),
  _numLeaves(0),
  _serializedNodes(),
  _incoreSlotGid(0)
{
  loadFromFile(treeletFilePath);
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
Treelet::getNodes()
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
Treelet::getNodeForIndex(unsigned index)
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
Treelet::getMemSize() const
{
  return _memSize;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns number of nodes
  \param ...
  \remarks ...
*/

unsigned
Treelet::getNumNodes() const
{
  return _numNodes;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief sets the number of nodes
  \param ...
  \remarks ...
*/

void
Treelet::setNumNodes(unsigned value)
{
  _numNodes = value;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns number of leafes
  \param ...
  \remarks ...
*/

unsigned
Treelet::getNumLeaves() const
{
  return _numLeaves;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief sets the number of leafes
  \param ...
  \remarks ...
*/

void
Treelet::setNumLeaves(unsigned value)
{
  _numLeaves = value;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the index of the first leaf node
  \param ...
  \remarks ...
*/

unsigned
Treelet::getFirstLeafIndex() const
{
  return (int)_numNodes-(int)_numLeaves;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns the absolute position of the root node of this treelet within the gpu buffer
  \param   ...
  \remarks ...
*/

unsigned
Treelet::getIncoreSlotPosition() const
{
  return _incoreSlotGid;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   sets the absolute position of the Treelet within the gpu buffer
  \param   ...
  \remarks ...
*/

void
Treelet::setIncoreSlotPosition(unsigned incoreSlotGid)
{
  _incoreSlotGid = incoreSlotGid;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief writes the serialized svo and attributes to a file
  \param ...
  \remarks ...
*/

bool
Treelet::writeToFile(const std::string& filePath)
{
  std::cerr << std::endl;
  std::cerr << std::endl << "Message from Treelet::writeToFile():";
  std::cerr << std::endl << "  Writing Treelet to:";
  std::cerr << std::endl << "    " << filePath;

  gloost::BinaryFile outfile;
  if (!outfile.openToWrite(filePath))
  {
    return false;
  }

  writeToFile(outfile);

  outfile.close();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief writes the serialized svo and attributes to a file
  \param ...
  \remarks ...
*/

bool
Treelet::writeToFile(gloost::BinaryFile& outFileToAppend)
{
  outFileToAppend.writeUInt32(_numNodes);                                   // write number of nodes
  outFileToAppend.writeUInt32(_numLeaves);                                  // write number of leaves
  outFileToAppend.writeUInt32(_memSize);                                    // write mem size
  outFileToAppend.writeUInt32(_treeletGid);                                 // write treeletGid
  outFileToAppend.writeUInt32(_parentTreeletGid);                           // write parents treeletGid
  outFileToAppend.writeUInt32(_parentTreeletLeafPosition);                  // write position of the leaf within the parent Treelet
  outFileToAppend.writeBuffer((unsigned char*)&_serializedNodes.front(),    // write Nodes
                              _serializedNodes.size()*sizeof(CpuSvoNode));
//  outfile.writeBuffer((unsigned char*)&_serializedAttributeIndices.front(),    // write attribute indices
//                      _serializedAttributeIndices.size()*sizeof(unsigned));
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief loads a *.treelet file
  \param ...
  \remarks ...
*/

bool
Treelet::loadFromFile(const std::string& filePath)
{
  std::cerr << std::endl;
  std::cerr << std::endl << "Message from Treelet::loadFromFile():";
  std::cerr << std::endl << "             Reading *.treelet file";
  std::cerr << std::endl << "             " << filePath;

  gloost::BinaryFile inFile;
  inFile.openAndLoad(filePath);

  loadFromFile(inFile);


  inFile.unload();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief loads a Treelet from a *.svo file
  \param ...
  \remarks ...
*/

bool
Treelet::loadFromFile(gloost::BinaryFile& inFile)
{
  _numNodes                  = inFile.readUInt32();
  _numLeaves                 = inFile.readUInt32();
  _memSize                   = inFile.readUInt32();
  _treeletGid                = inFile.readUInt32();
  _parentTreeletGid          = inFile.readUInt32();
  _parentTreeletLeafPosition = inFile.readUInt32();

//  std::cerr << std::endl << "Loaded Treelet with: ";
//  std::cerr << std::endl << "    _treeletGid:       " << _treeletGid;
//  std::cerr << std::endl << "    _parentTreeletGid: " << _parentTreeletGid;
//  std::cerr << std::endl << "    _numNodes:         " << _numNodes;
//  std::cerr << std::endl << "    _numLeaves:        " << _numLeaves;
//  std::cerr << std::endl << "    _memSize:          " << _memSize;

  _serializedNodes.resize(_memSize/sizeof(CpuSvoNode));

  gloost::unserialize((unsigned char*)&_serializedNodes.front(),
                      _memSize,
                      inFile);
//  gloost::unserialize((unsigned char*)&_serializedAttributeIndices.front(),
//                      _numNodes*sizeof(unsigned),
//                      infile);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns a map of QueueElement for each leafe assoziated with the leafes index within this Treelet
  \param   ...
  \remarks This map is only filled after building
*/

std::vector<Treelet::QueueElement>&
Treelet::getLeafQueueElements()
{
  return _leafQueueElements;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returs treeletId
  \param   ...
  \remarks
*/

gloost::gloostId
Treelet::getTreeletGid() const
{
  return _treeletGid;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returs treeletId
  \param   ...
  \remarks
*/

gloost::gloostId
Treelet::getParentTreeletGid() const
{
  return _parentTreeletGid;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returs the position of the leaf within the parent Treelet
  \param   ...
  \remarks
*/

gloost::gloostId
Treelet::getParentTreeletLeafPosition() const
{
  return _parentTreeletLeafPosition;
}


////////////////////////////////////////////////////////////////////////////////

} // namespace svo

