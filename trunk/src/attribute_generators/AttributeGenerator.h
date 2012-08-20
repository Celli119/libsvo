
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
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>


/// cpp includes
#include <string>


namespace gloost
{
  class Mesh;
  class ObjMatFile;
  class InterleavedAttributes;
}



namespace svo
{

  class Svo;
  class SvoNode;


  //  generates attributes per voxel

class AttributeGenerator
{
	public:

    // class constructor
    AttributeGenerator();

    // class destructor
	  virtual ~AttributeGenerator();


    // init the AttributeGenerator
	  virtual void generate(Svo* svo,
                          gloost::Mesh* mesh,
                          gloost::ObjMatFile* materials,
                          bool freeDiscreteSamplesAfterwards = true) = 0;


    // compresses the attribute buffer and creates a coresponding attribute buffer
    virtual void generateCompressAttributes() = 0;


	  // returns a vector of texture ids, textures containing serialized attribute data
	  gloost::InterleavedAttributes* getAttributeBuffer(unsigned id = 0);


	  // returns a vector of texture ids, textures containing serialized attribute data
	  gloost::InterleavedAttributes* getCompressedAttributeBuffer(unsigned id = 0);


	  // writes one attribute buffer to a *.ia file
	  virtual bool writeAttributeBufferToFile(const std::string& filePath,
                                            unsigned id = 0) = 0;

	  // writes one compressed attribute buffer to a *.ia file
	  virtual bool writeCompressedAttributeBufferToFile(const std::string& filePath,
                                                      unsigned id = 0) = 0;


	protected:

	  virtual void generateInnerNodesAttributesRecursive(SvoNode* node, unsigned currentDepth);

    // generates the attributes for one partucular node
	  virtual void generateCurrentNodesAttribs(SvoNode* node, unsigned depth) = 0;


    // vector of texture ids of serialized attribute data
    std::vector<gloost::InterleavedAttributes*> _attributes;
    std::vector<gloost::InterleavedAttributes*> _compressedAttributes;
};


} // namespace svo


#endif // H_SVO_ATTRIBUTEGENERATOR


