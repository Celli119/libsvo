
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



#ifndef H_SVO_ATTRIBUTEGENERATOR
#define H_SVO_ATTRIBUTEGENERATOR



/// gloost system includes
#include <gloostConfig.h>
#include <gloostMath.h>


/// cpp includes
#include <string>


namespace gloost
{
  class Mesh;
  class ObjMatFile;
}



namespace svo
{

  class Svo;


  //  generates attributes per voxel

class AttributeGenerator
{
	public:

    // class constructor
    AttributeGenerator();

    // class destructor
	  ~AttributeGenerator();


    // init the AttributeGenerator
	  void generate(Svo* svo, gloost::Mesh* mesh, gloost::ObjMatFile* materials);


	private:

   // ...

};


} // namespace svo


#endif // H_SVO_ATTRIBUTEGENERATOR


