
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



#ifndef H_GLOOST_TREELETMEMORYMANAGERCL
#define H_GLOOST_TREELETMEMORYMANAGERCL



// libsvo includes
#include <CpuSvoNode.h>
#include <TreeletMemoryManager.h>


/// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/bencl/ClContext.h>


/// cpp includes
#include <string>
#include <vector>
#include <stack>
#include <map>


namespace gloost
{
  class Mesh;
}



namespace svo
{

  class Treelet;


  //  Page, a subtree of the whole svo

class TreeletMemoryManagerCl: public TreeletMemoryManager
{
	public:

    // class constructor
    TreeletMemoryManagerCl(const std::string& svoFilePath,
                           unsigned incoreBufferSizeInByte,
                           gloost::bencl::ClContext* clContext);

    // class destructor
	  virtual ~TreeletMemoryManagerCl();



	  // inits the cl buffer
	  void initClBuffer();

	  // returns the Gid of the ClBuffer containing the SVO
	  gloost::gloostId getClIncoreBufferGid() const;


    // updates device memory by uploading incore buffer slots
    virtual void updateDeviceMemory();



	protected:



	private:

    gloost::bencl::ClContext* _clContext;

    gloost::gloostId          _svoClBufferGid;


};


} // namespace svo


#endif // H_GLOOST_TREELETMEMORYMANAGERCL

