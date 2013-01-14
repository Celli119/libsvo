
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
    _visibleOldTreeletsGids()
{



  // setting up feedback buffer
  _hostSideFeedbackBuffer.resize(_bufferWidth*_bufferHeight, FeedBackDataElement());

  std::cerr << std::endl;
  std::cerr << std::endl << "Setting up feedback buffer: ";
  std::cerr << std::endl << "  width:  " << _bufferWidth;
  std::cerr << std::endl << "  height: " << _bufferHeight;
  std::cerr << std::endl;

	_feedbackBufferGid = _memoryManager->getContext()->createClBuffer(CL_MEM_WRITE_ONLY,
                                                                   (unsigned char*)&_hostSideFeedbackBuffer.front(),
                                                                    _hostSideFeedbackBuffer.size()*sizeof(FeedBackDataElement));


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

  // start raycasting for analysis
  const gloost::Frustum& frustum = camera->getFrustum();

  gloost::Vector3 frustumH_vec          = frustum.far_lower_right - frustum.far_lower_left;
  gloost::Vector3 frustumOnePixelWidth  = frustumH_vec/_bufferWidth;
  gloost::Vector3 frustumV_vec          = frustum.far_upper_left - frustum.far_lower_left;
  gloost::Vector3 frustumOnePixelHeight = frustumV_vec/_bufferHeight;


  // adding a random offset to the frustum.far_lower_left
  gloost::Vector3 frustumFarLowerLeftPlusOffset = frustum.far_lower_left
                                                + gloost::frand()*frameBufferToFeedbackBufferRatio*frameBufferFrustumOnePixelWidth
                                                + gloost::frand()*frameBufferToFeedbackBufferRatio*frameBufferFrustumOnePixelHeight;


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


  // read back the filled buffer
  _memoryManager->getContext()->readFromClBufferToCharPointerComplete(deviceGid,
                                                                      _feedbackBufferGid,
                                                                      (unsigned char*)&_hostSideFeedbackBuffer.front(),
                                                                      true);
  clFinish( _memoryManager->getContext()->getDevice(deviceGid)->getClCommandQueue() );


  // analyse buffer
  std::map<gloost::gloostId, float> visibleNewTreeletGidsMap;
  std::map<gloost::gloostId, float> visibleOldTreeletGidsMap;


  unsigned hitCounter = 0;

  for (unsigned i=0; i!=_hostSideFeedbackBuffer.size(); ++i)
  {
    // if there was a hit within the svo
    if (_hostSideFeedbackBuffer[i]._nodePosOrTreeletGid)
    {
      ++hitCounter;

      // if this hit was a leaf with assoziated subTreelet
      if (_hostSideFeedbackBuffer[i]._isLeafeWithSubtreelet)
      {
        std::map<gloost::gloostId, float>::iterator pos = visibleNewTreeletGidsMap.find(_hostSideFeedbackBuffer[i]._nodePosOrTreeletGid);

        if (pos == visibleNewTreeletGidsMap.end())
        {
          visibleNewTreeletGidsMap[_hostSideFeedbackBuffer[i]._nodePosOrTreeletGid] = _hostSideFeedbackBuffer[i]._errorIfLeafe;
        }
        else
        {
          if (pos->second < _hostSideFeedbackBuffer[i]._errorIfLeafe)
          {
            pos->second = _hostSideFeedbackBuffer[i]._errorIfLeafe;
          }
        }

        addParentTreeletsToVisibleTreelets( i,
                                            _hostSideFeedbackBuffer[i]._nodePosOrTreeletGid,
                                            _hostSideFeedbackBuffer[i]._errorIfLeafe,
                                            visibleOldTreeletGidsMap);
      }
      // if this hit was an inner node or a final leaf
      else
      {
        unsigned slotId     = _hostSideFeedbackBuffer[i]._nodePosOrTreeletGid / _memoryManager->getNumNodesPerTreelet();
        unsigned treeletGid = _memoryManager->getSlots()[slotId];

        addParentTreeletsToVisibleTreelets( i,
                                            treeletGid,
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
    _visibleOldTreeletsGids.insert((*visibleOldIt).first);
    ++visibleOldIt;
  }


//  std::cerr << std::endl << "-----------------------------:";
//  std::cerr << std::endl << "hitCounter:                " << hitCounter;
//  std::cerr << std::endl << "uniqueTreeletCounter:      " << uniqueTreeletCounter.size();
//  std::cerr << std::endl << "visibleTreeletGids:        " << visibleTreeletGids.size();
//  std::cerr << std::endl << "_visibleOldTreeletsGids:   " << _visibleOldTreeletsGids.size();


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
}


//////////////////////////////////////////////////////


/**
  \brief   adds all parent Treelet Gids to the container, replaces error values with bigger ones
  \param   ...
  \remarks ...
*/

void
RenderPassAnalyse::addParentTreeletsToVisibleTreelets( unsigned feedbackBufferIndex,
                                                       gloost::gloostId treeletId,
                                                       float error,
                                                       std::map<gloost::gloostId, float>& treeletGidContainer)
{
  while (treeletId != 0)
  {
    std::map<gloost::gloostId, float>::iterator pos = treeletGidContainer.find(treeletId);

    // if this gid was not found
    if (pos == treeletGidContainer.end())
    {
      // add to countainer
      treeletGidContainer[treeletId] = _hostSideFeedbackBuffer[feedbackBufferIndex]._errorIfLeafe;
    }
    else
    {
//      // if the error is bigger
//      if ((*pos).second < _hostSideFeedbackBuffer[feedbackBufferIndex]._errorIfLeafe)
//      {
//        // replace error value to the bigger one
//        (*pos).second = _hostSideFeedbackBuffer[feedbackBufferIndex]._errorIfLeafe;
//      }
    }
    // repeat for the parent
    treeletId = _memoryManager->getTreelet(treeletId)->getParentTreeletGid();
  }
}


//////////////////////////////////////////////////////


/**
  \brief   returns a std::set of TreeletIds belonging to visible leaves
  \param   ...
  \remarks ...
*/

std::set<RenderPassAnalyse::TreeletGidAndError>&
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

std::set<gloost::gloostId>&
RenderPassAnalyse::getVisibleOldTreeletsGids()
{
  return _visibleOldTreeletsGids;
}


//////////////////////////////////////////////////////





} // namespace svo


