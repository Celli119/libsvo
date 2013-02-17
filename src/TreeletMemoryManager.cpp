
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
#include <contrib/Timer.h>
#include <contrib/TimerLog.h>


// cpp includes
#include <string>
#include <iostream>
#include <omp.h>

#define MAX_VISIBILITY 8



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
  _slots(),
//  _firstDynamicSlotIndex(0),
  _childTreeletsInIncoreBuffer(),
  _freeIncoreSlots(),
  _incoreSlotsToUpload()
{

  // load svo file
  if (!loadFromFile(svoFilePath))
  {
    exit(0);
    return;
  }

  if (_treelets[0])
  {
    _treeletSizeInByte = _treelets[0]->getMemSize();
  }

  resetIncoreBuffer();


  gloostTest::TimerLog::get()->addTimer("memory_manager.update_incore_buffer_host");

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
  if (gid > -1 && gid < _treelets.size())
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
    return false;
  }

  unsigned numAttributeBuffers = inFileAttributes.readUInt32();
  _attributeBuffers.resize(numAttributeBuffers);

  for (unsigned i=0; i!=numTreelets; ++i)
  {
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
  _incoreSlotsToUpload.clear();

  std::vector< std::set<gloost::gloostId > >().swap(_childTreeletsInIncoreBuffer);
  _childTreeletsInIncoreBuffer.resize(_treelets.size(), std::set<gloost::gloostId>());



  // allocate incoreBuffer
  unsigned numNodesInIncoreBuffer = (_incoreBufferSizeInByte/SVO_CPUSVONODE_NODE_SIZE);
  _incoreBuffer.resize(numNodesInIncoreBuffer);

  unsigned maxNumTreeletsInIncoreBuffer = std::floor(_incoreBufferSizeInByte/(float)_treeletSizeInByte);


  std::cerr << std::endl;
  std::cerr << std::endl << "Message from TreeletMemoryManager::initIncoreBuffer(): ";
  std::cerr << std::endl << "             incore buffer size:      " << _incoreBufferSizeInByte/1024/1024 << " MB";
  std::cerr << std::endl << "             incore num nodes:        " << _incoreBuffer.size();
  std::cerr << std::endl << "             incore num Treelets:     " << maxNumTreeletsInIncoreBuffer;


  _slots.resize(maxNumTreeletsInIncoreBuffer, SlotInfo());

  for (unsigned i=0; i!=_slots.size(); ++i)
  {
    _slots[i] = SlotInfo();
  }

  for (unsigned i=1; i!=maxNumTreeletsInIncoreBuffer+1; ++i)
  {
    int slotId = (int)maxNumTreeletsInIncoreBuffer - (int)i;
    _freeIncoreSlots.push(slotId);
  }


  // create incore buffer for attributes
  std::cerr << std::endl << "             Attribute incore buffer: " << numNodesInIncoreBuffer * _attributeBuffers[0]->getPackageStride() / 1024.0 /1024.0 << " MB";
  if (_attributeBuffers.size())
  {
    _incoreAttributeBuffer = new gloost::InterleavedAttributes(_attributeBuffers[0]->getLayout(),
                                                               numNodesInIncoreBuffer);
  }
  std::cerr << std::endl << "             Attribute incore buffer size: " << _incoreAttributeBuffer->getVector().size();

  insertTreeletIntoIncoreBuffer(SlotInfo(0, MAX_VISIBILITY, 999));


  // fill up incoreBuffer
#if 1
//  _firstDynamicSlotIndex = 0;
//  std::set<gloost::gloostId> secondTreeletStageGids;
//  for (unsigned i=1; i!=_treelets.size(); ++i)
//  {
//    if (_treelets[i]->getParentTreeletGid() == 0)
//    {
//      insertTreeletIntoIncoreBuffer(SlotInfo(_treelets[i]->getTreeletGid(), MAX_VISIBILITY, 100));
//     ++_firstDynamicSlotIndex;
//     secondTreeletStageGids.insert(_treelets[i]->getTreeletGid());
//    }
//  }

//  for (unsigned i=_firstDynamicSlotIndex; i!=_treelets.size(); ++i)
//  {
//    if (secondTreeletStageGids.find(_treelets[i]->getParentTreeletGid()) != secondTreeletStageGids.end())
//    {
//      insertTreeletIntoIncoreBuffer(SlotInfo(_treelets[i]->getTreeletGid(), MAX_VISIBILITY, 100));
//     _firstDynamicSlotIndex = _freeIncoreSlots.top();
//    }
//  }

#endif


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
  // timer
  gloostTest::Timer timerupdateIncoreBufferHost;
  timerupdateIncoreBufferHost.start();


  // update visibility of treelets allready in the incore buffer
  {
    // decrement visibiliti of all slots
    omp_set_num_threads(4);
    #pragma omp parallel for
    for (unsigned i=1; i<_slots.size(); ++i)
    {
      --_slots[i]._visibility;
    }

    // set visibility of visible treelets
    std::multiset<RenderPassAnalyse::TreeletGidAndError>& visibleOldTreelets = renderPassAnalyse->getVisibleOldTreeletsGids();

    std::multiset<RenderPassAnalyse::TreeletGidAndError>::iterator treeletGidAndErrorIt    = visibleOldTreelets.begin();
    std::multiset<RenderPassAnalyse::TreeletGidAndError>::iterator treeletGidAndErrorEndIt = visibleOldTreelets.end();

    for (; treeletGidAndErrorIt!=treeletGidAndErrorEndIt; ++treeletGidAndErrorIt)
    {
      unsigned slotPos = _treelets[(*treeletGidAndErrorIt)._treeletGid]->getSlotGid();
      _slots[slotPos]._visibility = MAX_VISIBILITY;
   }
  }

  std::multiset<RenderPassAnalyse::TreeletGidAndError>& visibleTreelets = renderPassAnalyse->getVisibleNewTreeletsGids();

  std::multiset<RenderPassAnalyse::TreeletGidAndError>::iterator treeletGidIt    = visibleTreelets.begin();
  std::multiset<RenderPassAnalyse::TreeletGidAndError>::iterator treeletGidEndIt = visibleTreelets.end();

  for (; treeletGidIt!=treeletGidEndIt; ++treeletGidIt)
  {
    insertTreeletIntoIncoreBuffer(SlotInfo((*treeletGidIt)._treeletGid, MAX_VISIBILITY, (*treeletGidIt)._error));
  }

//  for (unsigned i=0; i!=100; ++i)
//  {
//    freeIncorePosition(SlotInfo());
//  }


  // /timer
  timerupdateIncoreBufferHost.stop();
  gloostTest::TimerLog::get()->putSample("memory_manager.update_incore_buffer_host", timerupdateIncoreBufferHost.getDurationInMicroseconds()/1000.0);

}


//////////////////////////////////////////////////////


/**
  \brief   inserts a Treelet into the gpu buffer
  \param   ...
  \remarks ...
*/

bool
TreeletMemoryManager::insertTreeletIntoIncoreBuffer(const SlotInfo& tve)
{
  if (tve._treeletGid >= _treelets.size())
  {
    std::cerr << std::endl << "TreeletMemoryManager::insertTreeletIntoIncoreBuffer: " << tve._treeletGid;
    return false;
  }


  if (!_freeIncoreSlots.size())
  {
    if (!freeIncorePosition(tve))
    {
      return false;
    }
  }

  gloost::gloostId freeSlotGid = _freeIncoreSlots.top();
  _freeIncoreSlots.pop();

  if (freeSlotGid == 0)
  {
    std::cerr << std::endl << "!!! freeSlotGid == 0: ";
  }


  _slots[freeSlotGid] = tve;

  // set slot Gid to Treelet
  _treelets[tve._treeletGid]->setSlotGid(freeSlotGid);

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
  markIncoreSlotForUpload(freeSlotGid);


  // updating corresponding parent treelets leaf
  if (tve._treeletGid == 0)
  {
    return true;
  }

  gloost::gloostId parentTreeletGid                = _treelets[tve._treeletGid]->getParentTreeletGid();
  gloost::gloostId parentTreeletLeafPosition       = _treelets[tve._treeletGid]->getParentTreeletLeafPosition();
  gloost::gloostId parentTreeletLeafParentPosition = _treelets[tve._treeletGid]->getParentTreeletLeafsParentPosition();
  gloost::gloostId parentTreeletLeafIdx            = _treelets[tve._treeletGid]->getParentTreeletLeafIdx();

  unsigned leafPositionInIncoreBuffer       = _treelets[parentTreeletGid]->getSlotGid()
                                            * _numNodesPerTreelet+parentTreeletLeafPosition;
  unsigned leafParentPositionInIncoreBuffer = _treelets[parentTreeletGid]->getSlotGid()
                                            * _numNodesPerTreelet+parentTreeletLeafParentPosition;

  // copy root node of new Treelet to the leaf of parents Treelet
  _incoreBuffer[leafPositionInIncoreBuffer] = _incoreBuffer[incoreNodePosition];

  // update leaf mask of leafs parent so that the leaf is no leaf anymore
  _incoreBuffer[leafParentPositionInIncoreBuffer].setLeafMaskFlag(parentTreeletLeafIdx, false);

  // update first child position within leaf/root (this is a relative value within the incore buffer)
  _incoreBuffer[leafPositionInIncoreBuffer].setFirstChildIndex( (int)(incoreNodePosition+1)
                                                               -(int)leafPositionInIncoreBuffer );

  // mark incore slot of parent to be uploaded to device memory
  markIncoreSlotForUpload(_treelets[parentTreeletGid]->getSlotGid());

  // note treeletGid to parent position within the _childTreeletsInIncoreBuffer
  _childTreeletsInIncoreBuffer[parentTreeletGid].insert(tve._treeletGid);

  return true;
}


//////////////////////////////////////////////////////


/**
  \brief   frees one or more slots
  \param   ...
  \remarks ...
*/

bool
TreeletMemoryManager::freeIncorePosition(const SlotInfo& tve)
{
  RenderPassAnalyse::TreeletGidAndError freeableTreeletGid(0,0);

  unsigned              testCounter = 0;
  static const unsigned numProbes   = 100;
  static unsigned       searchPos   = 1;

  for (unsigned i=0; i!=_slots.size(); ++i)
  {
    if (_slots[searchPos]._visibility < 0 && _slots[searchPos]._treeletGid != -1)
    {
      gloost::gloostId parentTreeletGid = _treelets[_slots[searchPos]._treeletGid]->getParentTreeletGid();

      if (_slots[_treelets[parentTreeletGid]->getSlotGid()]._visibility < 0)
      {
        freeableTreeletGid = RenderPassAnalyse::TreeletGidAndError(_slots[searchPos]._treeletGid, _slots[searchPos]._error);
        break;
      }
    }

    ++searchPos;
    ++testCounter;
    if (searchPos ==_slots.size())
    {
      searchPos = 1;
    }

    if (testCounter == numProbes)
    {
      break;
    }
  }


  // no slot to free found
  if (freeableTreeletGid._treeletGid == 0)
  {
    return false;
  }


  // check all treeletGids attached below this treelet in the incore buffer
  std::queue<gloost::gloostId> childTreeletsQueue;
  childTreeletsQueue.push(freeableTreeletGid._treeletGid);

  unsigned freeCounter = 0;
  std::vector<unsigned> childTreeletsTreeletsToClear;
  while (childTreeletsQueue.size())
  {
    gloost::gloostId parentTreeletGid = childTreeletsQueue.front();
    childTreeletsQueue.pop();

    std::set<gloost::gloostId >::iterator childrenInIncoreBufferSetIt    = _childTreeletsInIncoreBuffer[parentTreeletGid].begin();
    std::set<gloost::gloostId >::iterator childrenInIncoreBufferSetEndIt = _childTreeletsInIncoreBuffer[parentTreeletGid].end();

    for (; childrenInIncoreBufferSetIt!=childrenInIncoreBufferSetEndIt; ++childrenInIncoreBufferSetIt)
    {
      gloost::gloostId childTreeletGid = (*childrenInIncoreBufferSetIt);
      childTreeletsQueue.push(childTreeletGid);
      childTreeletsTreeletsToClear.push_back(childTreeletGid);
    }

    _childTreeletsInIncoreBuffer[parentTreeletGid] = std::set<gloost::gloostId >();
  }

  _childTreeletsInIncoreBuffer[freeableTreeletGid._treeletGid] = std::set<gloost::gloostId >();

  // free slots of child treelets
  for (unsigned i=0; i!=childTreeletsTreeletsToClear.size(); ++i)
  {
    unsigned childTreeletGid = childTreeletsTreeletsToClear[i];
    unsigned slotId          = _treelets[childTreeletGid]->getSlotGid();

    if (slotId == 0)
    {
      std::cerr << std::endl;
      std::cerr << std::endl << "!!! TRYING TO FREE SLOT 0: ";
      std::cerr << std::endl << "               Treelet id = " << childTreeletGid;
      std::cerr << std::endl;
    }
    else
    {
      _freeIncoreSlots.push(slotId);
      _treelets[childTreeletGid]->setSlotGid(-1);
      _slots[slotId] = SlotInfo();
    }
  }

  removeTreeletFromIncoreBuffer(freeableTreeletGid._treeletGid);

  return true;
}


//////////////////////////////////////////////////////


/**
  \brief   removes a Treelet from the gpu buffer
  \param   ...
  \remarks ...
*/

bool
TreeletMemoryManager::removeTreeletFromIncoreBuffer(gloost::gloostId treeletGid)
{
  gloost::gloostId parentTreeletGid                = _treelets[treeletGid]->getParentTreeletGid();
  gloost::gloostId parentTreeletLeafPosition       = _treelets[treeletGid]->getParentTreeletLeafPosition();
  gloost::gloostId parentTreeletLeafParentPosition = _treelets[treeletGid]->getParentTreeletLeafsParentPosition();
  gloost::gloostId parentTreeletLeafIdx            = _treelets[treeletGid]->getParentTreeletLeafIdx();

  gloost::gloostId slotGid       = _treelets[treeletGid]->getSlotGid();
  gloost::gloostId parentSlotGid = _treelets[parentTreeletGid]->getSlotGid();

  unsigned leafPositionInIncoreBuffer       = parentSlotGid*_numNodesPerTreelet + parentTreeletLeafPosition;
  unsigned leafParentPositionInIncoreBuffer = parentSlotGid*_numNodesPerTreelet + parentTreeletLeafParentPosition;

  // copy original node/leaf to incore leaf position
  _incoreBuffer[leafPositionInIncoreBuffer] = getTreelet(parentTreeletGid)->getNodeForIndex(parentTreeletLeafPosition);

  // update leaf mask of leafs parent so that the leaf is a leaf again
  _incoreBuffer[leafParentPositionInIncoreBuffer].setLeafMaskFlag(parentTreeletLeafIdx, true);

  // mark incore slot of parent to be uploaded to device memory
  markIncoreSlotForUpload(parentSlotGid);

  // clear slot info
  _slots[slotGid] = SlotInfo();

  // add slots to available/free slots
  _freeIncoreSlots.push(slotGid);

  // remove assoziation from treelet gid to slot
  _treelets[treeletGid]->setSlotGid(-1);

  _childTreeletsInIncoreBuffer[parentTreeletGid].erase(treeletGid);

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
std::vector<TreeletMemoryManager::SlotInfo>&
TreeletMemoryManager::getSlots()
{
  return _slots;
}


//////////////////////////////////////////////////////


bool
sortSlotsByVisibility (const TreeletMemoryManager::SlotInfo a,
                       const TreeletMemoryManager::SlotInfo b)
{
  return (a._visibility<b._visibility);
}


} // namespace svo

