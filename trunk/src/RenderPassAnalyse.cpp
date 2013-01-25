
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
#include <RenderPassAnalyse.h>
#include <TreeletMemoryManagerCl.h>
#include <Treelet.h>
#include <contrib/Timer.h>
#include <contrib/TimerLog.h>

// gloost system includes
#include <gloost/TextureManager.h>
#include <gloost/Texture.h>
#include <gloost/PerspectiveCamera.h>
#include <gloost/Frustum.h>

// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   RenderPassAnalyse

  \brief   Renders the SVO and creates the feedback buffer in the process

  \author  Felix Weiszig
  \date    December 2012
  \remarks
*/

//////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

RenderPassAnalyse::RenderPassAnalyse( TreeletMemoryManagerCl* memoryManager,
                                      unsigned bufferWidth,
                                      unsigned bufferHeight):
    _memoryManager(memoryManager),
    _bufferWidth(bufferWidth),
    _bufferHeight(bufferHeight),
    _hostSideFeedbackBuffer(),
    _feedbackBufferGid(0),
    _visibleNewTreeletsGids(),
    _visibleOldTreeletsGids(),
    _rumble(true)
{

  // setting up feedback buffer
  _hostSideFeedbackBuffer.resize(_bufferWidth*_bufferHeight, FeedBackDataElement());

  std::cerr << std::endl;
  std::cerr << std::endl << "Setting up feedback buffer: ";
  std::cerr << std::endl << "  width:  " << _bufferWidth;
  std::cerr << std::endl << "  height: " << _bufferHeight;
  std::cerr << std::endl;

	_feedbackBufferGid = _memoryManager->getContext()->createClBuffer(CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                                                   (unsigned char*)&_hostSideFeedbackBuffer.front(),
                                                                    _hostSideFeedbackBuffer.size()*sizeof(FeedBackDataElement));

  gloostTest::TimerLog::get()->addTimer("analyse.1_enqueueKernel");
  gloostTest::TimerLog::get()->addTimer("analyse.2_readbackBuffer");
  gloostTest::TimerLog::get()->addTimer("analyse.3_processBuffer");
}


//////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

RenderPassAnalyse::~RenderPassAnalyse()
{
	_memoryManager->getContext()->removeBuffer(_feedbackBufferGid);
}


//////////////////////////////////////////////////////


/**
  \brief   performs the analyse pass
  \param   ...
  \remarks ...
*/

void
RenderPassAnalyse::performAnalysePass(gloost::gloostId           deviceGid,
                                      gloost::PerspectiveCamera* camera,
                                      const gloost::Matrix&      modelMatrix,
                                      float                      tScaleRatio,
                                      const gloost::Vector3&     frameBufferFrustumOnePixelWidth,
                                      const gloost::Vector3&     frameBufferFrustumOnePixelHeight,
                                      unsigned                   frameBufferToFeedbackBufferRatio)
{
  _visibleNewTreeletsGids.clear();
  _visibleOldTreeletsGids.clear();


  // timer for kernel
  gloostTest::Timer timerAnalysePass;
  timerAnalysePass.start();


  // start raycasting for analysis
  const gloost::Frustum& frustum = camera->getFrustum();

  gloost::Vector3 frustumH_vec          = frustum.far_lower_right - frustum.far_lower_left;
  gloost::Vector3 frustumOnePixelWidth  = frustumH_vec/_bufferWidth;
  gloost::Vector3 frustumV_vec          = frustum.far_upper_left - frustum.far_lower_left;
  gloost::Vector3 frustumOnePixelHeight = frustumV_vec/_bufferHeight;


  // adding a random offset to the frustum.far_lower_left
  gloost::Vector3 frustumFarLowerLeftPlusOffset = frustum.far_lower_left;

  if (_rumble)
  {
    frustumFarLowerLeftPlusOffset = frustum.far_lower_left
                                    + gloost::frand()*frameBufferToFeedbackBufferRatio*frameBufferFrustumOnePixelWidth
                                    + gloost::frand()*frameBufferToFeedbackBufferRatio*frameBufferFrustumOnePixelHeight;
  }

  gloost::bencl::ClContext* clContext = _memoryManager->getContext();
  clContext->setKernelArgBuffer("renderToFeedbackBuffer", 0, _feedbackBufferGid);
  clContext->setKernelArgBuffer("renderToFeedbackBuffer", 1, _memoryManager->getClIncoreBufferGid());
  clContext->setKernelArgFloat4("renderToFeedbackBuffer", 2, gloost::Vector3(_bufferWidth, _bufferHeight, tScaleRatio));
  clContext->setKernelArgFloat4("renderToFeedbackBuffer", 3, frustumOnePixelWidth);
  clContext->setKernelArgFloat4("renderToFeedbackBuffer", 4, frustumOnePixelHeight);
  clContext->setKernelArgFloat4("renderToFeedbackBuffer", 5, frustumFarLowerLeftPlusOffset /*frustum.far_lower_left*/);
  clContext->setKernelArgFloat4("renderToFeedbackBuffer", 6, modelMatrix * camera->getPosition());

  clContext->enqueueKernel(deviceGid,
                           "renderToFeedbackBuffer",
                           2u,
                           gloost::Vector3(_bufferWidth, _bufferHeight, 1),
                           true,
                           gloost::Vector3(8, 8, 0));


  timerAnalysePass.stop();
  gloostTest::TimerLog::get()->putSample("analyse.1_enqueueKernel", timerAnalysePass.getDurationInMicroseconds()/1000.0);




  // timer
  gloostTest::Timer timerReadback;
  timerReadback.start();

  // read back the filled buffer
  _memoryManager->getContext()->readFromClBufferToCharPointerComplete(deviceGid,
                                                                      _feedbackBufferGid,
                                                                      (unsigned char*)&_hostSideFeedbackBuffer.front(),
                                                                      true);

  timerReadback.stop();
  gloostTest::TimerLog::get()->putSample("analyse.2_readbackBuffer", timerReadback.getDurationInMicroseconds()/1000.0);
  // analyse buffer
  // use mapsd here to ensure unique treelt ids


  // timer
  gloostTest::Timer timerProcessBuffer;
  timerProcessBuffer.start();

  std::map<gloost::gloostId, float> visibleNewTreeletGidsMap;
  std::map<gloost::gloostId, float> visibleOldTreeletGidsMap;


  for (unsigned i=0; i!=_hostSideFeedbackBuffer.size(); ++i)
  {
    // if there was a hit within the svo
    if (_hostSideFeedbackBuffer[i]._nodePosOrTreeletGid)
    {
      // if this hit was a leaf with assoziated subTreelet
      if (_hostSideFeedbackBuffer[i]._isLeafeWithSubtreelet)
      {

        if (_hostSideFeedbackBuffer[i]._nodePosOrTreeletGid < 0 || _hostSideFeedbackBuffer[i]._nodePosOrTreeletGid > _memoryManager->getTreelets().size())
        {
          std::cerr << std::endl;
          std::cerr << std::endl << "ERROR in RenderPassAnalyse::performAnalysePass(): ";
          std::cerr << std::endl << "         Tried to access Treelet with Gid: " << _hostSideFeedbackBuffer[i]._nodePosOrTreeletGid;
          std::cerr << std::endl << "         (For an leafe with sub treelet): ";
          return;
        }

        // check if treeletGid was already noted and update error if necessary
        std::map<gloost::gloostId, float>::iterator pos = visibleNewTreeletGidsMap.find(_hostSideFeedbackBuffer[i]._nodePosOrTreeletGid);

        if (pos == visibleNewTreeletGidsMap.end())
        {
          visibleNewTreeletGidsMap[_hostSideFeedbackBuffer[i]._nodePosOrTreeletGid] = _hostSideFeedbackBuffer[i]._errorIfLeafe;
        }
        else
        {
          if (_hostSideFeedbackBuffer[i]._errorIfLeafe > pos->second)
          {
            pos->second = _hostSideFeedbackBuffer[i]._errorIfLeafe;
          }
        }

        addNodeAndItsParentTreeletsToVisibleTreelets( _memoryManager->getTreelet(_hostSideFeedbackBuffer[i]._nodePosOrTreeletGid)->getParentTreeletGid(),
                                                      _hostSideFeedbackBuffer[i]._errorIfLeafe,
                                                      visibleOldTreeletGidsMap);
      }
      // if this hit was an inner node or a final leaf
      else
      {
        unsigned slotId = _hostSideFeedbackBuffer[i]._nodePosOrTreeletGid / _memoryManager->getNumNodesPerTreelet();

        if (slotId < 0 || slotId > _memoryManager->getTreelets().size())
        {
          std::cerr << std::endl;
          std::cerr << std::endl << "ERROR in RenderPassAnalyse::performAnalysePass(): ";
          std::cerr << std::endl << "         Tried to access slotId with Gid: " << slotId;
          std::cerr << std::endl << "         (For an inner node): ";
          return;
        }


        unsigned treeletGid = _memoryManager->getSlots()[slotId]._treeletGid;

        if (treeletGid < 0 || treeletGid > _memoryManager->getTreelets().size())
        {
          std::cerr << std::endl;
          std::cerr << std::endl << "ERROR in RenderPassAnalyse::performAnalysePass(): ";
          std::cerr << std::endl << "         Tried to access Treelet with Gid: " << treeletGid;
          std::cerr << std::endl << "         (For an inner node): ";
          return;
        }

        addNodeAndItsParentTreeletsToVisibleTreelets( treeletGid,
                                                      _hostSideFeedbackBuffer[i]._errorIfLeafe,
                                                      visibleOldTreeletGidsMap);
      }
    }
  }


  // copy visible new treelets
  std::map<gloost::gloostId, float>::iterator visibleNewIt    = visibleNewTreeletGidsMap.begin();
  std::map<gloost::gloostId, float>::iterator visibleNewEndIt = visibleNewTreeletGidsMap.end();

  while (visibleNewIt != visibleNewEndIt)
  {
    _visibleNewTreeletsGids.insert(TreeletGidAndError(visibleNewIt->first,visibleNewIt->second));
    ++visibleNewIt;
  }

  // copy visible old treelets
  std::map<gloost::gloostId, float>::iterator visibleOldIt    = visibleOldTreeletGidsMap.begin();
  std::map<gloost::gloostId, float>::iterator visibleOldEndIt = visibleOldTreeletGidsMap.end();

  while (visibleOldIt != visibleOldEndIt)
  {
    _visibleOldTreeletsGids.insert(TreeletGidAndError((*visibleOldIt).first, (*visibleOldIt).second));
    ++visibleOldIt;
  }



  static const unsigned maxTreeletsToPropergate = 1024;
  if (_visibleNewTreeletsGids.size() > maxTreeletsToPropergate)
  {
    std::set<TreeletGidAndError>::iterator vtIt = _visibleNewTreeletsGids.begin();
    unsigned count = 0u;

    while (count < maxTreeletsToPropergate && vtIt != _visibleNewTreeletsGids.end())
    {
      ++count;
      ++vtIt;
    }

    if (vtIt != _visibleNewTreeletsGids.end() )
    {
       _visibleNewTreeletsGids.erase(vtIt, _visibleNewTreeletsGids.end());
    }
  }


  timerProcessBuffer.stop();
  gloostTest::TimerLog::get()->putSample("analyse.3_processBuffer", timerProcessBuffer.getDurationInMicroseconds()/1000.0);



}


//////////////////////////////////////////////////////


/**
  \brief   adds all parent Treelet Gids to the container, replaces error values with bigger ones
  \param   ...
  \remarks ...
*/

void
RenderPassAnalyse::addNodeAndItsParentTreeletsToVisibleTreelets( gloost::gloostId treeletGid,
                                                                 float error,
                                                                 std::map<gloost::gloostId, float>& treeletGidContainer)
{

  while (treeletGid != 0)
  {
    std::map<gloost::gloostId, float>::iterator pos = treeletGidContainer.find(treeletGid);

    // if this gid was not found
    if (pos == treeletGidContainer.end())
    {
      // add to countainer
      treeletGidContainer[treeletGid] = error;
    }
    else
    {
      // if the error is bigger
      if (error > (*pos).second)
      {
        // replace error value to the bigger one
        (*pos).second = error;
      }
    }

    treeletGid = _memoryManager->getTreelet(treeletGid)->getParentTreeletGid();

  }
}


//////////////////////////////////////////////////////


/**
  \brief   returns a std::set of TreeletIds belonging to visible leaves
  \param   ...
  \remarks ...
*/

std::multiset<RenderPassAnalyse::TreeletGidAndError>&
RenderPassAnalyse::getVisibleNewTreeletsGids()
{
  return _visibleNewTreeletsGids;
}


//////////////////////////////////////////////////////


/**
  \brief   returns a std::set of TreeletIds belonging inner nodes or final leaves
  \param   ...
  \remarks ...
*/

std::multiset<RenderPassAnalyse::TreeletGidAndError>&
RenderPassAnalyse::getVisibleOldTreeletsGids()
{
  return _visibleOldTreeletsGids;
}


//////////////////////////////////////////////////////


/**
  \brief   ...
  \param   ...
  \remarks ...
*/

void
RenderPassAnalyse::setEnableRumble(bool onOrOff)
{
  std::cerr << std::endl << "setEnableRumble: " << onOrOff;

  _rumble = onOrOff;
}


//////////////////////////////////////////////////////


/**
  \brief   ...
  \param   ...
  \remarks ...
*/

bool
RenderPassAnalyse::getEnableRumble()
{
  return _rumble;
}


//////////////////////////////////////////////////////





} // namespace svo


