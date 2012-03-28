
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



#ifndef H_SVO_IMAGETOPOINTCLOUD
#define H_SVO_IMAGETOPOINTCLOUD



/// gloost system includes
#include <gloostConfig.h>
#include <gloostMath.h>


/// cpp includes
#include <string>



namespace gloost
{
  class Mesh;
}



namespace svo
{


  //  converts an depth image to a point cloud

class ImageToPointcloud
{
	public:

    // generates a mesh from a depth image and a color image
	  static gloost::Mesh* generate(gloost::mathType fieldOfView,
                                  const gloost::Matrix& viewMatrix,
                                  const std::string& depthImagePath,
                                  const std::string& colorImagePath);


	private:

    // class constructor
    ImageToPointcloud();

    // class destructor
	  ~ImageToPointcloud();
   // ...

};


} // namespace svo


#endif // H_SVO_IMAGETOPOINTCLOUD


