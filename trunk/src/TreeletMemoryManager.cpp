
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



/// libsvo system includes
#include <TreeletMemoryManager.h>
#include <Treelet.h>
#include <TreeletBuilderFromFaces.h>
#include <attribute_generators/Ag_colorAndNormalsTriangles.h>


// gloost system includes
#include <gloost/Mesh.h>
#include <gloost/BinaryFile.h>


/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   TreeletMemoryManager

  \brief   TreeletMemoryManager, a subtree of the whole svo

  \author  Felix Weiszig
  \date    october 2012
  \remarks ...
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

TreeletMemoryManager::TreeletMemoryManager(const std::string& svoFilePath, unsigned incoreBufferSizeInByte):
  _treeletSizeInByte(0),
  _numNodesPerTreelet(0),
  _treelets(),
  _incoreBuffer(),
  _incoreBufferSizeInByte(incoreBufferSizeInByte),
  _treeletGidToSlotGidMap(),
  _freeIncoreSlots(),
  _incoreSlotsToUpload()
{

  // load svo file
  if (!loadFromFile(svoFilePath))
  {
    return;
  }

  if (_treelets[0])
  {
    _treeletSizeInByte = _treelets[0]->getMemSize();
  }

  initIncoreBuffer();


  // fill up incoreBuffer
  unsigned treeletGid = 1;
  while (treeletGid < _treelets.size() && insertTreeletIntoIncoreBuffer(treeletGid))
  {
    ++treeletGid;
  }

}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

TreeletMemoryManager::~TreeletMemoryManager()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns a Treelet for a given Gid
  \param   ...
  \remarks ...
*/

Treelet*
TreeletMemoryManager::getTreelet(gloost::gloostId id)
{
  return _treelets[id];
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns the Treelet size in bytes
  \param   ...
  \remarks ...
*/

unsigned
TreeletMemoryManager::getTreeletSizeInByte() const
{
  return _treeletSizeInByte;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns the incore buffer
  \param   ...
  \remarks ...
*/

std::vector<CpuSvoNode>&
TreeletMemoryManager::getIncoreBuffer()
{
  return _incoreBuffer;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   loads svo/Treelet structure from file
  \param   ...
  \remarks ...
*/

bool
TreeletMemoryManager::loadFromFile(const std::string& filePath)
{
  std::cerr << std::endl;
  std::cerr << std::endl << "Message from TreeletMemoryManager::loadFromFile():";
  std::cerr << std::endl << "             Reading *.svo file";
  std::cerr << std::endl << "             " << filePath;

  gloost::BinaryFile inFile;
  if (!inFile.openAndLoad(filePath))
  {
    std::cerr << std::endl;
    std::cerr << std::endl << "ERROR in : TreeletMemoryManager::loadFromFile()";
    std::cerr << std::endl << "           Could NOT open file" << filePath;
    std::cerr << std::endl;
    return false;
  }

  unsigned numTreelets = inFile.readUInt32();

  _treeletSizeInByte   = inFile.readUInt32();
  _numNodesPerTreelet  = _treeletSizeInByte/sizeof(CpuSvoNode);

  std::cerr << std::endl << "             number of Treelets:           " << numTreelets;
  std::cerr << std::endl << "             number of nodes per Treelets: " << _numNodesPerTreelet;
  std::cerr << std::endl << "             Treelet size:                 " << _treeletSizeInByte << " (" << _treeletSizeInByte/1024 << " KB)";

  _treelets.resize(numTreelets);

  for (unsigned i=0; i!=numTreelets; ++i)
  {
    _treelets[i] = new Treelet();
    _treelets[i]->loadFromFile(inFile);
  }

  inFile.unload();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   inits the incoreBuffer
  \param   ...
  \remarks ...
*/

void
TreeletMemoryManager::initIncoreBuffer()
{

  std::cerr << std::endl;
  std::cerr << std::endl << "Message from TreeletMemoryManager::initIncoreBuffer(): ";

  // allocate incoreBuffer
  _incoreBuffer.resize(_incoreBufferSizeInByte/SVO_CPUSVONODE_NODE_SIZE);

  std::cerr << std::endl << "             incore buffer size:      " << _incoreBufferSizeInByte/1024/1024;
  std::cerr << std::endl << "             incore num nodes:        " << _incoreBuffer.size();


  // memcpy root treelet to incoreBuffer
  memcpy( (char*)&_incoreBuffer.front(),
          (char*)&_treelets[0]->getNodes().front(),
          _treeletSizeInByte);

  // write down where to find root Treelet in the incoreBuffer
  _treeletGidToSlotGidMap[0u] = 0u;

  unsigned maxNumTreeletsInIncoreBuffer = _incoreBufferSizeInByte/_treeletSizeInByte;
  std::cerr << std::endl << "             max num treelets incore: " << maxNumTreeletsInIncoreBuffer;

  for (unsigned i=1; i!=maxNumTreeletsInIncoreBuffer; ++i)
  {
    _freeIncoreSlots.push((int)maxNumTreeletsInIncoreBuffer - (int)i);
  }
  std::cerr << std::endl;

}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   inserts a Treelet into the gpu buffer
  \param   ...
  \remarks ...
*/

bool
TreeletMemoryManager::insertTreeletIntoIncoreBuffer(gloost::gloostId treeletGid)
{
  if (!_freeIncoreSlots.size() || treeletGid >= _treelets.size())
  {
//    std::cerr << std::endl << "Warning from TreeletMemoryManager::insertTreeletIntoGpuBuffer: ";
//    std::cerr << std::endl << "             Out of incore slots!";

    return false;
    // make place ...
  }

  unsigned freeSlotGid = _freeIncoreSlots.top();
  _freeIncoreSlots.pop();

  // update lookup the treeletGid -> incoreBufferSlot
  _treeletGidToSlotGidMap[treeletGid] = freeSlotGid;

  // set slot Gid to Treelet
  _treelets[treeletGid]->setIncoreSlotPosition(freeSlotGid);

  // calculate where to put the Treelet
  unsigned incoreNodePosition = freeSlotGid*_numNodesPerTreelet;

#if 0
  std::cerr << std::endl << "Adding: " << treeletGid
            << " into slot " << freeSlotGid
            << ", at node " << incoreNodePosition;
#endif

  // memcpy root treelet to incoreBuffer
  memcpy( (char*)&_incoreBuffer[incoreNodePosition],
          (char*)&_treelets[treeletGid]->getNodes().front(),
          _treeletSizeInByte);

  // updating corresponding parent treelets leaf
  gloost::gloostId parentTreeletGid                = _treelets[treeletGid]->getParentTreeletGid();
  gloost::gloostId parentTreeletLeafPosition       = _treelets[treeletGid]->getParentTreeletLeafPosition();
  gloost::gloostId parentTreeletLeafParentPosition = _treelets[treeletGid]->getParentTreeletLeafsParentPosition();
  gloost::gloostId parentTreeletLeafIdx            = _treelets[treeletGid]->getParentTreeletLeafIdx();

  unsigned leafPositionInIncoreBuffer       = _treeletGidToSlotGidMap[parentTreeletGid]*_numNodesPerTreelet+parentTreeletLeafPosition;
  unsigned leafParentPositionInIncoreBuffer = _treeletGidToSlotGidMap[parentTreeletGid]*_numNodesPerTreelet+parentTreeletLeafParentPosition;

  // copy root node of new Treelet to the leaf of parents Treelet
  _incoreBuffer[leafPositionInIncoreBuffer] = _incoreBuffer[incoreNodePosition];

  // update first child position within leaf/root (this is a relative value within the incore buffer)
  _incoreBuffer[leafPositionInIncoreBuffer].setFirstChildIndex( (int)(incoreNodePosition+1) - (int)leafPositionInIncoreBuffer);

  // update leaf mask of leafs parent so that the leaf is no leaf anymore
  _incoreBuffer[leafParentPositionInIncoreBuffer].setLeafMaskFlag(parentTreeletLeafIdx, false);


  // mark incore slot of parent to be uploaded to device memory
  markIncoreSlotForUpload(_treeletGidToSlotGidMap[parentTreeletGid]);

  // mark incore slot of new Treelet to be uploaded to device memory
  markIncoreSlotForUpload(_treeletGidToSlotGidMap[treeletGid]);


  return true;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   marks a incore slot to be uploaded to device memory
  \param   ...
  \remarks ...
*/

void
TreeletMemoryManager::markIncoreSlotForUpload(gloost::gloostId slotGid)
{
  _incoreSlotsToUpload.insert(slotGid);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   updates device memory by uploading incore buffer slots
  \param   ...
  \remarks ...
*/

void
TreeletMemoryManager::updateDeviceMemory()
{
//  while(_incoreSlotsToUpload.size())
//  {
//    _incoreSlotsToUpload.
//  }
}


////////////////////////////////////////////////////////////////////////////////





} // namespace svo

