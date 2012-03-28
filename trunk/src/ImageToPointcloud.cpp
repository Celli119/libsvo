
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



/// gloost system includes
#include <ImageToPointcloud.h>



/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   ImageToPointcloud

  \brief   converts an depth image to a point cloud

  \author  Felix Weiszig
  \date    March 2011
  \remarks ...
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

ImageToPointcloud::ImageToPointcloud()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

ImageToPointcloud::~ImageToPointcloud()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   generates a mesh from a depth image and a color image
  \param   ...
  \remarks ...
*/

/* static*/
gloost::Mesh*
generate( gloost::mathType fieldOfView,
          const gloost::Matrix& viewMatrix,
          const std::string& depthImagePath,
          const std::string& colorImagePath)
{



}


////////////////////////////////////////////////////////////////////////////////





} // namespace svo


