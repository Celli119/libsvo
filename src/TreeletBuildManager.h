
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



#ifndef H_GLOOST_TREELETBUILDMANAGER
#define H_GLOOST_TREELETBUILDMANAGER



// libsvo includes
#include <CpuSvoNode.h>


// gloost system includes
#include <gloost/gloostConfig.h>
//#include <gloost/gloostMath.h>


// cpp includes
#include <string>
#include <vector>
#include <memory>


namespace gloost
{
  class Mesh;
  class InterleavedAttributes;
}



namespace svo
{

  class Treelet;


  //  Builds Treelet-SVO structure from different source types

class TreeletBuildManager
{
	public:

    // class constructor
    TreeletBuildManager();

    // class destructor
	  virtual ~TreeletBuildManager();

    // builds trunk and all branches from triangle Mesh
	  void buildFromFaces(unsigned treeletSizeInByte,
                        unsigned maxSvoDepth,
                        unsigned numBuildingThreads,
                        const std::shared_ptr<gloost::Mesh>& mesh,
                        const std::string& outFilePath);


    // writes svo/Treelet structure to file
    bool writeToFile(const std::string& filePath) const;


    // returns a Treelet for a given Gid
    Treelet* getTreelet(gloost::gloostId Gid);

    // returns a attribute buffer for a given Gid
    std::shared_ptr<gloost::InterleavedAttributes> getAttributeBuffer(gloost::gloostId Gid);


	protected:


	private:

	  unsigned _treeletSizeInByte;

    std::vector<Treelet*>                                         _treelets;
    std::vector< std::shared_ptr<gloost::InterleavedAttributes> > _attributeBuffers;

};


} // namespace svo


#endif // H_GLOOST_TREELETBUILDMANAGER

