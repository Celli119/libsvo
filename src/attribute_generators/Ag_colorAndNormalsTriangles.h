
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



// svo system includes
#include <attribute_generators/AttributeGenerator.h>


// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>


// cpp includes
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

  class TreeletBuildManager;
  class Treelet;


  //  Retrieves colors and normals from Triangles of a Mesh

class Ag_colorAndNormalsTriangles: public AttributeGenerator
{
	public:

    // class constructor
    Ag_colorAndNormalsTriangles();

    // class destructor
	  virtual ~Ag_colorAndNormalsTriangles();


    // creates attributes for all final leafes of a treelet
    static void createFinalLeafesAttributes(TreeletBuildManager* buildManager,
                                            unsigned treeletGid,
                                            gloost::Mesh* mesh);

    static void createInnerNodesAttributes( TreeletBuildManager* buildManager,
                                            gloost::gloostId     treeletGid);

//
//
//    //
//	  void generate(std::vector<Treelet*>& treeletVector,
//                  gloost::Mesh* mesh,
//                  gloost::ObjMatFile* materials,
//                  bool freeDiscreteSamplesAfterwards = true);
//
//
//    // compresses the attribute buffer and creates a coresponding attribute buffer with the values
//    virtual void generateCompressAttributes();
//
//
//	  // writes one attribute buffer to a *.ia file
//	  virtual bool writeAttributeBufferToFile(const std::string& filePath,
//                                            unsigned id = 0);
//
//	  // writes one compressed attribute buffer to a *.ia file
//	  virtual bool writeCompressedAttributeBufferToFile(const std::string& filePath,
//                                                      unsigned id = 0);


	private:


      static void recursive_createInnerNodesAttributes(TreeletBuildManager* buildManager,
                                                       gloost::gloostId     treeletGid,
                                                       gloost::gloostId     nodeIndex,
                                                       unsigned             currentDepth);

      // generates the attributes for one partucular node
      static void averageFromeChildAttributes(TreeletBuildManager* buildManager,
                                              gloost::gloostId treeletGid,
                                              gloost::gloostId nodeIndex,
                                              unsigned depth);

    // generates the attributes for one partucular node
//	  virtual void generateCurrentNodesAttribs(SvoNode* node, unsigned depth);



};


} // namespace svo


#endif // H_SVO_ATTRIBUTEGENERATOR


