
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



#ifndef H_SVO_SVOVISUALIZER
#define H_SVO_SVOVISUALIZER



/// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>
#include <gloost/MatrixStack.h>
#include <gloost/BoundingBox.h>


/// cpp includes
#include <string>


#define SVO_VISUALIZER_MODE_WIRED_NODES   0
#define SVO_VISUALIZER_MODE_BOXED_LEAFES  1



namespace gloost
{
  class Mesh;
  class Vbo;
  class InterleavedAttributes;
}



namespace svo
{

  class SvoBranch;
  class SvoNode;



  //  SvoVisualizer

class SvoVisualizer
{
	public:

    // class constructor
    SvoVisualizer(unsigned maxDepth, unsigned mode = SVO_VISUALIZER_MODE_WIRED_NODES);

    // class destructor
	  virtual ~SvoVisualizer();

	  // builds the visualization from a svo::Svo
	  void build(SvoBranch* svo, gloost::InterleavedAttributes* attributes);


	  // draws the svo visualization
	  void draw();

	  // returns the mesh created by the visualizer
	  gloost::Mesh* getMesh();


	  // writes the visualization as gbm file
	  bool saveAsGbm(const std::string& filePath);

    // adds a wireframe cube to the mesh
    void pushWireCubeToMesh(const gloost::BoundingBox& boundingBox,
                            unsigned currentDepth);

    // adds a wireframe cube to the mesh
    void pushWireCubeToMesh(const gloost::BoundingBox& boundingBox,
                            unsigned currentDepth,
                            const gloost::vec4& color);

    // adds a solid cube to the mesh
    void pushSolidCubeToMesh(const gloost::BoundingBox& bbox,
                             unsigned currentDepth,
                             unsigned attributePosition);


	protected:




	  // builds the visualization from a svo::Svo
	  void buildRecursive( SvoNode* node,
                         const gloost::Vector3& offsetToParent,
                         unsigned currentDepth);



    SvoBranch*                _svo;

    gloost::InterleavedAttributes* _attributes;

    unsigned            _maxDepth;
    gloost::MatrixStack _matrixStack;

    gloost::Mesh*       _mesh;
    gloost::Vbo*        _vbo;

    unsigned            _mode;

    unsigned            _numWireBoxes;
    unsigned            _numSolidBoxes;

    unsigned            _vertexIndex;

};


} // namespace svo


#endif // H_SVO_SVOVISUALIZER


