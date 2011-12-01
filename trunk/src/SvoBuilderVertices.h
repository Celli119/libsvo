
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



#ifndef H_SVO_SVO_BUILDER_VERTICES
#define H_SVO_SVO_BUILDER_VERTICES



/// svo system includes


/// gloost system includes
#include <gloostConfig.h>
#include <gloostMath.h>
#include <Mesh.h>


/// cpp includes
#include <string>



namespace svo
{

  class Svo;


  //  generates the svo structure from a mesh using the vertices

class SvoBuilderVertices
{
	public:

    // class constructor
    SvoBuilderVertices();

    // class destructor
	  ~SvoBuilderVertices();


    // generates the svo from vertices
	  Svo* build(Svo* svo, gloost::Mesh* mesh);


	private:

   // ...

};


} // namespace svo


#endif // H_SVO_SVO_BUILDER_VERTICES


