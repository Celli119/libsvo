
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
  class InterleavedAttributes;
}



namespace svo
{

  class Treelet;
  class RenderPassAnalyse;


  //  Page, a subtree of the whole svo

class TreeletMemoryManager
{
	public:

    // class constructor
    TreeletMemoryManager(const std::string& svoFilePath, unsigned incoreBufferSizeInByte);

    // class destructor
	  virtual ~TreeletMemoryManager();


    // returns a Treelet for a given Gid
    Treelet* getTreelet(gloost::gloostId gid);

    // returns a reference of a std::vector of Treelet*
    std::vector<Treelet*>& getTreelets();


    // returns the Treelet size in bytes
    unsigned getTreeletSizeInByte() const;

    // returns the number of nodes per treelet
    unsigned getNumNodesPerTreelet() const;


    // returns the incore buffer
    std::vector<CpuSvoNode>& getIncoreBuffer();

    // returns the attribute incore buffer
    gloost::InterleavedAttributes* getAttributeIncoreBuffer();



    // updates the client side incore buffer
    void updateClientSideIncoreBuffer(RenderPassAnalyse* renderPassAnalyse);

    // inserts a Treelet into the gpu buffer
    bool insertTreeletIntoIncoreBuffer(gloost::gloostId treeletGid);

    // updates device memory by uploading incore buffer slots
    virtual void updateDeviceMemory();




    // inits the incoreBuffer
    virtual void resetIncoreBuffer();


    // returns a const reference to the stack of free incore slots
    virtual const std::stack<unsigned>& getFreeIncoreSlotStack() const;

    // returns a set of incore slot Gids that should be transfered to the server
    virtual const std::set<gloost::gloostId>& getIncoreSlotsToUpload() const;


    // rettuns a vector containing a Treelet Gid for each slot
    std::vector<unsigned>& getSlots();


	protected:


	  unsigned                _treeletSizeInByte;
	  unsigned                _numNodesPerTreelet;


    std::vector<Treelet*>                        _treelets;
    std::vector<gloost::InterleavedAttributes*>  _attributeBuffers;

    std::vector<CpuSvoNode>                      _incoreBuffer;
    gloost::InterleavedAttributes*               _incoreAttributeBuffer;
    unsigned                                     _incoreBufferSizeInByte;
    std::map<gloost::gloostId, unsigned>         _treeletGidToSlotGidMap; // << assoziation from Treelet Gid to slot id
    std::vector<unsigned>                        _slots;                  // << assoziation from slot id to Treelet Gid
    std::stack<unsigned>                         _freeIncoreSlots;

    std::set<gloost::gloostId>                   _incoreSlotsToUpload;



    // loads svo/Treelet structure from file
    bool loadFromFile(const std::string& filePath);



    // marks a incore slot to be uploaded to device memory
    void markIncoreSlotForUpload(gloost::gloostId slotGid);


	private:




};


} // namespace svo


#endif // H_GLOOST_TREELETMEMORYMANAGER

