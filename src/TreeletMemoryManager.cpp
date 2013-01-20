
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
#include <RenderPassAnalyse.h>
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
  _slots(),
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
#if 1
  unsigned treeletGid = 1;
  while (treeletGid < _treelets.size() && insertTreeletIntoIncoreBuffer(VisibilityAndError(treeletGid, 8, 100)))
  {
    ++treeletGid;
  }
#endif

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
TreeletMemoryManager::getTreelet(gloost::gloostId gid)
{
  if (gid > 0 && gid < _treelets.size())
  {
    return _treelets[gid];
  }
  std::cerr << std::endl;
  std::cerr << std::endl << "ERROR in TreeletMemoryManager::getTreelet(): ";
  std::cerr << std::endl << "         Tried to access Treelet with Gid: " << gid;
  return 0;
}


//////////////////////////////////////////////////////


/**
  \brief   returns a reference of a std::vector of Treelet*
  \param   ...
  \remarks ...
*/

std::vector<Treelet*>&
TreeletMemoryManager::getTreelets()
{
  return _treelets;
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
  \brief   returns the number of nodes per treelet
  \param   ...
  \remarks ...
*/

unsigned
TreeletMemoryManager::getNumNodesPerTreelet() const
{
  return _numNodesPerTreelet;
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
  unsigned numNodesInIncoreBuffer = (_incoreBufferSizeInByte/SVO_CPUSVONODE_NODE_SIZE);
  _incoreBuffer.resize(numNodesInIncoreBuffer);

  std::cerr << std::endl << "             incore buffer size:      " << _incoreBufferSizeInByte/1024/1024 << " MB";
  std::cerr << std::endl << "             incore num nodes:        " << _incoreBuffer.size();

  unsigned maxNumTreeletsInIncoreBuffer = std::floor(_incoreBufferSizeInByte/(float)_treeletSizeInByte);

  _slots.resize(maxNumTreeletsInIncoreBuffer, VisibilityAndError());

  for (unsigned i=1; i!=maxNumTreeletsInIncoreBuffer+1; ++i)
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

  insertTreeletIntoIncoreBuffer(VisibilityAndError(0, 8, 999));
}


//////////////////////////////////////////////////////


/**
  \brief   updates the client side incore buffer
  \param   ...
  \remarks ...
*/

void
TreeletMemoryManager::updateClientSideIncoreBuffer(RenderPassAnalyse* renderPassAnalyse)
{
  std::multiset<RenderPassAnalyse::TreeletGidAndError>& visibleTreelets = renderPassAnalyse->getVisibleNewTreeletsGids();

  std::multiset<RenderPassAnalyse::TreeletGidAndError>::iterator treeletGidIt    = visibleTreelets.begin();
  std::multiset<RenderPassAnalyse::TreeletGidAndError>::iterator treeletGidEndIt = visibleTreelets.end();

  for (; treeletGidIt!=treeletGidEndIt; ++treeletGidIt)
  {
    insertTreeletIntoIncoreBuffer(VisibilityAndError((*treeletGidIt)._treeletGid, 8, (*treeletGidIt)._error));
  }
}


//////////////////////////////////////////////////////


/**
  \brief   inserts a Treelet into the gpu buffer
  \param   ...
  \remarks ...
*/

bool
TreeletMemoryManager::insertTreeletIntoIncoreBuffer(const VisibilityAndError& tve)
{

  // test if Treelet is allready in incore buffer
  std::map<gloost::gloostId, unsigned>::iterator pos = _treeletGidToSlotGidMap.find(tve._treeletGid);
  if (pos != _treeletGidToSlotGidMap.end() &&  (*pos).second != 0)
  {
    std::cerr << std::endl << "insertTreeletIntoIncoreBuffer: " << tve._treeletGid << " is allready in incoreBuffer !!!";
    return false;
  }

//  bool messageOut = false;

  if (!_freeIncoreSlots.size() || tve._treeletGid >= _treelets.size())
  {
    return false;
    // make space available ...
  }

  unsigned freeSlotGid = _freeIncoreSlots.top();
  _freeIncoreSlots.pop();

  _slots[freeSlotGid] = tve;

  // update lookup the treeletGid -> incoreBufferSlot
  _treeletGidToSlotGidMap[tve._treeletGid] = freeSlotGid;

  // set slot Gid to Treelet
  _treelets[tve._treeletGid]->setIncoreSlotPosition(freeSlotGid);

  // calculate where to put the Treelet
  unsigned incoreNodePosition = freeSlotGid*_numNodesPerTreelet;


  // memcpy root treelet to incoreBuffer
  memcpy( (char*)&_incoreBuffer[incoreNodePosition],
          (char*)&_treelets[tve._treeletGid]->getNodes().front(),
          _treeletSizeInByte);



  // ### copy treelet attributes to incoreBuffer
  unsigned incorePackageIndex   = freeSlotGid*_numNodesPerTreelet;
  unsigned incoreAttribPosition = _attributeBuffers[tve._treeletGid]->getPackageIndex(incorePackageIndex);

  // memcpy root treelet to incoreBuffer
  memcpy( (char*)&_incoreAttributeBuffer->getVector()[incoreAttribPosition],
          (char*)&_attributeBuffers[tve._treeletGid]->getVector().front(),
          _attributeBuffers[tve._treeletGid]->getVector().size()*sizeof(float));


  // mark incore slot of new Treelet to be uploaded to device memory
  markIncoreSlotForUpload(_treeletGidToSlotGidMap[tve._treeletGid]);


  // updating corresponding parent treelets leaf
  if (tve._treeletGid == 0)
  {
    return true;
  }

  gloost::gloostId parentTreeletGid                = _treelets[tve._treeletGid]->getParentTreeletGid();
  gloost::gloostId parentTreeletLeafPosition       = _treelets[tve._treeletGid]->getParentTreeletLeafPosition();
  gloost::gloostId parentTreeletLeafParentPosition = _treelets[tve._treeletGid]->getParentTreeletLeafsParentPosition();
  gloost::gloostId parentTreeletLeafIdx            = _treelets[tve._treeletGid]->getParentTreeletLeafIdx();

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


/**
  \brief   returns a const reference to the stack of free incore slots
  \param   ...
  \remarks ...
*/

/*virtual */
const std::stack<unsigned>&
TreeletMemoryManager::getFreeIncoreSlotStack() const
{
  return _freeIncoreSlots;
}


//////////////////////////////////////////////////////


/**
  \brief   returns a set of incore slot Gids that should be transfered to the server
  \param   ...
  \remarks ...
*/

/*virtual */
const std::set<gloost::gloostId>&
TreeletMemoryManager::getIncoreSlotsToUpload() const
{
  return _incoreSlotsToUpload;
}


//////////////////////////////////////////////////////


/**
  \brief   returns a vector containing a Treelet Gid for each slot
  \param   ...
  \remarks ...
*/

/*virtual */
std::vector<TreeletMemoryManager::VisibilityAndError>& TreeletMemoryManager::getSlots()
{
  return _slots;
}


//////////////////////////////////////////////////////





} // namespace svo

