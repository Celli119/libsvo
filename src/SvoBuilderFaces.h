
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



#ifndef H_SVO_SVO_BUILDER_FACES
#define H_SVO_SVO_BUILDER_FACES



/// svo system includes
#include <BuilderTriangleFace.h>


/// gloost system includes
#include <gloostConfig.h>
#include <gloostMath.h>
#include <Mesh.h>
#include <Vector3.h>
#include <Point3.h>
#include <MatrixStack.h>


/// cpp includes
#include <string>

namespace svo
{

  class Svo;
  class SvoNode;




  //  generates the svo structure by discretising faces

class SvoBuilderFaces
{
	public:

    // class constructor
    SvoBuilderFaces();

    // class destructor
	  virtual ~SvoBuilderFaces();


    // generates the svo by discretising faces
	  Svo* build(Svo* svo, gloost::Mesh* mesh);

	protected:

    // builds the svo recursive from triangle faces
    void buildRecursive(unsigned int           currentDepth,
                        const BuilderTriangleFace&  triangle);


    Svo*                _svo;
    gloost::Mesh*       _mesh;
    gloost::MatrixStack _matrixStack;

};


} // namespace svo


#endif // H_SVO_SVO_BUILDER_FACES


