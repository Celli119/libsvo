
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
#include <RenderPassAnalyse.h>
#include <TreeletMemoryManagerCl.h>

/// gloost system includes
#include <gloost/TextureManager.h>
#include <gloost/Texture.h>

/// cpp includes
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

////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

RenderPassAnalyse::RenderPassAnalyse( TreeletMemoryManagerCl* memoryManager,
                                      unsigned bufferWidth,
                                      unsigned bufferHeight,
                                      float    tScaleRatio):
    _memoryManager(memoryManager),
    _bufferWidth(bufferWidth),
    _bufferHeight(bufferHeight),
    _tScaleRatio(tScaleRatio),
    _hostSideFeedbackBuffer(),
    _feedbackBufferTextureGid(0)
{



  // setting up feedback buffer
  _hostSideFeedbackBuffer.resize(_bufferWidth*_bufferHeight, FeedBackDataElement());

  std::cerr << std::endl;
  std::cerr << std::endl << "Setting up feedback buffer: ";
  std::cerr << std::endl << "  width:  " << _bufferWidth;
  std::cerr << std::endl << "  height: " << _bufferHeight;
  std::cerr << std::endl;

  gloost::Texture* texture = new gloost::Texture( _bufferWidth,
                                                  _bufferHeight,
                                                  1,
                                                  (unsigned char*)&_hostSideFeedbackBuffer.front(),
                                                  8,
                                                  GL_TEXTURE_2D,
                                                  GL_LUMINANCE_ALPHA32F_ARB,
                                                  GL_LUMINANCE_ALPHA,
                                                  GL_FLOAT);

  _feedbackBufferTextureGid = gloost::TextureManager::get()->addTexture(texture);

  texture->setTexParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  texture->setTexParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  texture->setTexParameter(GL_TEXTURE_BASE_LEVEL, 0);
  texture->setTexParameter(GL_TEXTURE_MAX_LEVEL, 0);
  texture->initInContext();

  gloost::gloostId renderBufferGid = memoryManager->getContext()->createClImage(CL_MEM_WRITE_ONLY,
                                                                                texture->getTarget(),
                                                                                texture->getTextureHandle());


}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

RenderPassAnalyse::~RenderPassAnalyse()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


///**
//  \brief   Inits the RenderPassAnalyse
//  \param   ...
//  \remarks ...
//*/
//
//void
//RenderPassAnalyse::init()
//{
//	// insert your code here
//}




////////////////////////////////////////////////////////////////////////////////





} // namespace svo


