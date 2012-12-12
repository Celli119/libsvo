
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



#ifndef H_GLOOST_TREELETMEMORYMANAGER
#define H_GLOOST_TREELETMEMORYMANAGER



// libsvo includes
#include <CpuSvoNode.h>


// gloost system includes
#include <gloost/gloostConfig.h>
//#include <gloost/gloostMath.h>


// cpp includes
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <set>


namespace gloost
{
  class Mesh;
}



namespace svo
{

  class Treelet;


  //  Page, a subtree of the whole svo

class TreeletMemoryManager
{
	public:

    // class constructor
    TreeletMemoryManager(const std::string& svoFilePath, unsigned incoreBufferSizeInByte);

    // class destructor
	  virtual ~TreeletMemoryManager();


    // returns a Treelet for a given Gid
    Treelet* getTreelet(gloost::gloostId id);


    // returns the Treelet size in bytes
    unsigned getTreeletSizeInByte() const;


    // returns the incore buffer
    std::vector<CpuSvoNode>& getIncoreBuffer();


    // updates device memory by uploading incore buffer slots
    virtual void updateDeviceMemory();

    // inserts a Treelet into the gpu buffer
    bool insertTreeletIntoIncoreBuffer(gloost::gloostId treeletGid);


	protected:


	  unsigned                _treeletSizeInByte;
	  unsigned                _numNodesPerTreelet;

    std::vector<Treelet*>   _treelets;

    std::vector<CpuSvoNode>                      _incoreBuffer;
    unsigned                                     _incoreBufferSizeInByte;
    std::map<gloost::gloostId, gloost::gloostId> _treeletGidToSlotGidMap;
    std::stack<unsigned>                         _freeIncoreSlots;

    std::set<gloost::gloostId>                   _incoreSlotsToUpload;



    // loads svo/Treelet structure from file
    bool loadFromFile(const std::string& filePath);

    // inits the incoreBuffer
    void initIncoreBuffer();


    // marks a incore slot to be uploaded to device memory
    void markIncoreSlotForUpload(gloost::gloostId slotGid);


	private:




};


} // namespace svo


#endif // H_GLOOST_TREELETMEMORYMANAGER

