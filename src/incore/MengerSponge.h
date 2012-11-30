
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



#ifndef H_MENGERSPONGE
#define H_MENGERSPONGE



/// gloost system includes
#include <gloostConfig.h>
#include <gloostMath.h>
#include <MatrixStack.h>
#include <BoundingBox.h>
#include <Point3.h>


/// cpp includes
#include <string>
#include <vector>


namespace gloost
{
  class Mesh;
  class Vbo;
}



namespace svo
{


  //  SvoVisualizer

class MengerSponge
{
	public:

    // class constructor
    MengerSponge(unsigned int maxDepth);

    // class destructor
	  virtual ~MengerSponge();


	  // draws the svo visualization
	  void draw();

	  // returns the mesh created by the visualizer
	  gloost::Mesh* getMesh();


	  // writes the visualization as gbm file
	  bool saveAsGbm(const std::string& filePath);

    // adds a solid cube to the mesh
    void pushSolidCubeToMesh(const gloost::Point3& center,
                             float scale);


	protected:



    unsigned int        _maxDepth;

    gloost::Mesh*       _mesh;
    gloost::Vbo*        _vbo;

    unsigned int        _vertexIndex;

};


} // namespace svo


#endif // H_MENGERSPONGE


