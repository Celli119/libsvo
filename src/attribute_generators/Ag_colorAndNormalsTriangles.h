
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



#ifndef H_SVO_AG_COLORS_AND_NORMALS
#define H_SVO_AG_COLORS_AND_NORMALS



/// svo system includes
#include <attribute_generators/AttributeGenerator.h>


/// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>


/// cpp includes
#include <string>


namespace gloost
{
  class Mesh;
  class ObjMatFile;
  class BinaryBundle;
  class InterleavedAttributes;
}



namespace svo
{

  class Svo;


  //  Retrieves colors and normals from Triangles of a Mesh

class Ag_colorAndNormalsTriangles: public AttributeGenerator
{
	public:

    // class constructor
    Ag_colorAndNormalsTriangles();

    // class destructor
	  virtual ~Ag_colorAndNormalsTriangles();

    //
	  void generate(Svo* svo, gloost::Mesh* mesh, gloost::ObjMatFile* materials = 0);


	  virtual void generateCurrentNodesAttribs(SvoNode* node, unsigned depth);

	protected:



};


} // namespace svo


#endif // H_SVO_ATTRIBUTEGENERATOR

