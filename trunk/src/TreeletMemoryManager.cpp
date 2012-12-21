
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



// libsvo system includes
#include <TreeletMemoryManager.h>
#include <Treelet.h>
#include <TreeletBuilderFromFaces.h>
#include <attribute_generators/Ag_colorAndNormalsTriangles.h>


// gloost system includes
#include <gloost/Mesh.h>
#include <gloost/BinaryFile.h>


// cpp includes
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

//////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

TreeletMemoryManager::TreeletMemoryManager(const std::string& svoFilePath, unsigned incoreBufferSizeInByte):
  _treeletSizeInByte(0),
  _numNodesPerTreelet(0),
  _treelets(),
  _attributeBuffers(),
  _incoreBuffer(),
  _incoreAttributeBuffer(0),
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

  resetIncoreBuffer();


  // fill up incoreBuffer
//  unsigned treeletGid = 1;
//  while (treeletGid < _treelets.size() && insertTreeletIntoIncoreBuffer(treeletGid))
//  {
//    ++treeletGid;
//  }

}


//////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

TreeletMemoryManager::~TreeletMemoryManager()
{
	// insert your code here
}


//////////////////////////////////////////////////////


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


//////////////////////////////////////////////////////


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


//////////////////////////////////////////////////////


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


//////////////////////////////////////////////////////


/**
  \brief   returns the attribute incore buffer
  \param   ...
  \remarks ...
*/

gloost::InterleavedAttributes*
TreeletMemoryManager::getAttributeIncoreBuffer()
{
  return _incoreAttributeBuffer;
}

//////////////////////////////////////////////////////


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
  if (!inFile.openAndLoad(filePath + ".svo"))
  {
    std::cerr << std::endl;
    std::cerr << std::endl << "ERROR in : TreeletMemoryManager::loadFromFile()";
    std::cerr << std::endl << "           Could NOT open file" << filePath << ".svo";
    std::cerr << std::endl;
    return false;
  }

  unsigned numTreelets = inFile.readUInt32();

  _treeletSizeInByte   = inFile.readUInt32();
  _numNodesPerTreelet  = _treeletSizeInByte/sizeof(CpuSvoNode);

  std::cerr << std::endl << "             number of Treelets:           " << numTreelets;
  std::cerr << std::endl << "             number of nodes per Treelets: " << _numNodesPerTreelet;
  std::cerr << std::endl << "             Treelet size:                 " << _treeletSizeInByte << " (" << _treeletSizeInByte/1024 << " KB)";
//  std::cerr << std::endl << "> ";

  _treelets.resize(numTreelets);

  for (unsigned i=0; i!=numTreelets; ++i)
  {
//    std::cerr << i << ", ";
    _treelets[i] = new Treelet();
    _treelets[i]->loadFromFile(inFile);
  }

  inFile.unload();


  // attributes
  gloost::BinaryFile inFileAttributes;
  if (!inFileAttributes.openAndLoad(filePath+ ".ia"))
  {
    std::cerr << std::endl;
    std::cerr << std::endl << "ERROR in : TreeletMemoryManager::loadFromFile()";
    std::cerr << std::endl << "           Could NOT open file" << filePath << ".ia";
//    std::cerr << std::endl << "> ";
    return false;
  }

  unsigned numAttributeBuffers = inFileAttributes.readUInt32();
  _attributeBuffers.resize(numAttributeBuffers);

  for (unsigned i=0; i!=numTreelets; ++i)
  {
//    std::cerr << i << ", ";
    _attributeBuffers[i] = new gloost::InterleavedAttributes();
    _attributeBuffers[i]->loadFromFile(inFileAttributes);
  }

  inFileAttributes.unload();


  return true;
}


//////////////////////////////////////////////////////


/**
  \brief   inits the incoreBuffer
  \param   ...
  \remarks ...
*/

void
TreeletMemoryManager::resetIncoreBuffer()
{

  _freeIncoreSlots = std::stack<unsigned>();
  _treeletGidToSlotGidMap.clear();
  _incoreSlotsToUpload.clear();

  std::cerr << std::endl;
  std::cerr << std::endl << "Message from TreeletMemoryManager::initIncoreBuffer(): ";

  // allocate incoreBuffer
  unsigned numNodesInIncoreBuffer = _incoreBufferSizeInByte/SVO_CPUSVONODE_NODE_SIZE;
  _incoreBuffer.resize(numNodesInIncoreBuffer);

  std::cerr << std::endl << "             incore buffer size:      " << _incoreBufferSizeInByte/1024/1024 << " MB";
  std::cerr << std::endl << "             incore num nodes:        " << _incoreBuffer.size();

  unsigned maxNumTreeletsInIncoreBuffer = _incoreBufferSizeInByte/_treeletSizeInByte;
  for (unsigned i=0; i!=maxNumTreeletsInIncoreBuffer+1; ++i)
  {
    _freeIncoreSlots.push((int)maxNumTreeletsInIncoreBuffer - (int)i);
  }


  // create incore buffer for attributes
  std::cerr << std::endl << "             Attribute incore buffer: " << numNodesInIncoreBuffer * _attributeBuffers[0]->getPackageStride() / 1024.0 /1024.0 << " MB";
  if (_attributeBuffers.size())
  {
    _incoreAttributeBuffer = new gloost::InterleavedAttributes(_attributeBuffers[0]->getLayout(),
                                                               numNodesInIncoreBuffer);
  }
  std::cerr << std::endl << "             Attribute incore buffer size: " << _incoreAttributeBuffer->getVector().size();



  insertTreeletIntoIncoreBuffer(0);
}


//////////////////////////////////////////////////////


/**
  \brief   inserts a Treelet into the gpu buffer
  \param   ...
  \remarks ...
*/

bool
TreeletMemoryManager::insertTreeletIntoIncoreBuffer(gloost::gloostId treeletGid)
{

  // test if Treelet is allready in incore buffer

  std::map<gloost::gloostId, gloost::gloostId>::iterator pos = _treeletGidToSlotGidMap.find(treeletGid);
  if (pos != _treeletGidToSlotGidMap.end() &&  (*pos).second != 0)
  {
    return false;
  }

  bool messageOut = false;

  if (!_freeIncoreSlots.size() || treeletGid >= _treelets.size())
  {
    if (!messageOut)
    {
      std::cerr << std::endl << "Warning from TreeletMemoryManager::insertTreeletIntoGpuBuffer: ";
      std::cerr << std::endl << "             Out of incore slots!";
      messageOut = true;
    }

    return false;
    // make space available ...
  }

  unsigned freeSlotGid = _freeIncoreSlots.top();
  _freeIncoreSlots.pop();

  // update lookup the treeletGid -> incoreBufferSlot
  _treeletGidToSlotGidMap[treeletGid] = freeSlotGid;

  // set slot Gid to Treelet
  _treelets[treeletGid]->setIncoreSlotPosition(freeSlotGid);

  // calculate where to put the Treelet
  unsigned incoreNodePosition = freeSlotGid*_numNodesPerTreelet;


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



  // ### copy treelet attributes to incoreBuffer

  unsigned incorePackageIndex   = freeSlotGid*_numNodesPerTreelet;
  unsigned incoreAttribPosition = _attributeBuffers[treeletGid]->getPackageIndex(incorePackageIndex);

  // memcpy root treelet to incoreBuffer
  memcpy( (char*)&_incoreAttributeBuffer->getVector()[incoreAttribPosition],
          (char*)&_attributeBuffers[treeletGid]->getVector().front(),
          _attributeBuffers[treeletGid]->getVector().size()*sizeof(float));



  return true;
}


//////////////////////////////////////////////////////


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


//////////////////////////////////////////////////////


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


//////////////////////////////////////////////////////





} // namespace svo

