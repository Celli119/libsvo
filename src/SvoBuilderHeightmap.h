
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



#ifndef H_SVO_SVO_BUILDER_HEIGHTMAP
#define H_SVO_SVO_BUILDER_HEIGHTMAP



/// svo system includes


/// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>


/// cpp includes
#include <string>



namespace svo
{

  class Svo;


  //  EmptyClass template

class SvoBuilderHeightmap
{
	public:

    // class constructor
    SvoBuilderHeightmap();

    // class destructor
	  ~SvoBuilderHeightmap();


    // generates the svo from heightmap and a colormap
	  Svo* build( unsigned int heightMapId,
                unsigned int colorMapId,
                unsigned int maxDepth);


	private:

   // ...

};


} // namespace svo


#endif // H_SVO_SVO_BUILDER_HEIGHTMAP


