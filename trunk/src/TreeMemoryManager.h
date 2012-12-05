
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



#ifndef H_GLOOST_TREEMEMORYMANAGER
#define H_GLOOST_TREEMEMORYMANAGER



// libsvo includes
#include <CpuSvoNode.h>


/// gloost system includes
#include <gloost/gloostConfig.h>
//#include <gloost/gloostMath.h>


/// cpp includes
#include <string>
#include <vector>


namespace gloost
{
  class Mesh;
}



namespace svo
{

  class Treelet;


  //  Page, a subtree of the whole svo

class TreeMemoryManager
{
	public:

    // class constructor
    TreeMemoryManager();

    // class destructor
	  virtual ~TreeMemoryManager();

    // builds trunk and all branches from triangle Mesh
	  void buildFromFaces(unsigned treeletSizeInByte,
                        unsigned maxSvoDepth,
                        unsigned numBuildingThreads,
                        gloost::Mesh* mesh);


    // writes svo/Treelet structure to file
    bool writeToFile(const std::string& filePath) const;

    // loads svo/Treelet structure from file
    bool loadFromFile(const std::string& filePath);




    // returns a Treelet for a given Gid
    Treelet* getTreelet(gloost::gloostId id);





	protected:

   bool insertIntoGpuBuffer(gloost::gloostId treeletGid);


	private:

	  unsigned _treeletSizeInByte;

    std::vector<Treelet*>   _treelets;
    std::vector<CpuSvoNode> _treeletGpuBuffer;

};


} // namespace svo


#endif // H_GLOOST_TREEMEMORYMANAGER

