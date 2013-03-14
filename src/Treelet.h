
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



#ifndef H_SVO_TREELET
#define H_SVO_TREELET

#define SVO_EMPTY_ATTRIB_POS 999999999


// svo includes
#include <CpuSvoNode.h>


// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/Matrix.h>
#include <gloost/InterleavedAttributes.h>



// cpp includes
#include <string>
#include <map>
#include <vector>
#include <list>

namespace gloost
{
//  class BinaryBundle;
  class BinaryFile;
}


namespace svo
{

  //  Sparse Voxel Octree with utility functions

class Treelet
{
	public:

    //
    struct QueueElement
    {
      QueueElement():
        _localLeafIndex(0u),
        _idx(0u),
        _parentLocalNodeIndex(0u),
        _depth(0u),
        _aabbTransform(),
        _primitiveIds()
      {}

      unsigned              _localLeafIndex;         // absolute local index of the node within the serial structure
      char                  _idx;                    // index 0...7 of the node within its parent
      unsigned              _parentLocalNodeIndex;   // index of the nodes parent within the serial structure
      unsigned              _depth;                  // depth of the node within this treelet
      gloost::Matrix        _aabbTransform;          // transformation of the AABB/voxel
      std::vector<unsigned> _primitiveIds;           // primitive ids of all primitives contributing to this node
    };


    // class constructor
    Treelet();


    // class constructor
    Treelet(gloost::gloostId  treeletGid,
            unsigned          rootNodeDepth,
            gloost::gloostId  parentTreeletGid,
            gloost::gloostId  parentTreeletLeafPosition,
            gloost::gloostId  parentTreeletLeafIdx,
            gloost::gloostId  parentTreeletLeafsParentPosition,
            unsigned          memSizeInByte);

    // class constructor
    Treelet(const std::string treeletFilePath);

    // class destructor
	  virtual ~Treelet();



    // normalizes attribs of voxels storing contributions of more than one primitive during creation
//    void normalizeLeafAttribs();

    // generates attributes for inner nodes by averaging child attribs
//    void generateInnerNodesAttributes(SvoNode* node, int currentDepth = 0);



    // writes the serialized svo and attributes to a file
    bool writeToFile(const std::string& filePath);
    bool writeToFile(gloost::BinaryFile& outFileToAppend);

    // loads a *.treelet file
    bool loadFromFile(const std::string& filePath);

    // loads a Treelet from a *.svo file
    bool loadFromFile(gloost::BinaryFile& inFile);



    // returns a vector with serialized nodes
    std::vector<CpuSvoNode>& getNodes();

    // returns a serialized SvoNode for an index
    CpuSvoNode& getNodeForIndex(gloost::gloostId  index);






    // returns a container of QueueElement for each leafe with smaller depth as required
    std::vector<QueueElement>& getIncompleteLeafQueueElements();

    // returns a container of QueueElement for each leafe with required or bigger depth
    std::vector<QueueElement>& getFinalLeafQueueElements();


    // returs treeletId
    gloost::gloostId getTreeletGid() const;

    // returs parents treeletId
    gloost::gloostId getParentTreeletGid() const;

    // returs the position of the leaf within the parent Treelet
    gloost::gloostId getParentTreeletLeafPosition() const;

    // returs the idx (0...7) of the parent treelets leaf in its parent node
    gloost::gloostId getParentTreeletLeafIdx() const;

    // returs the position of the parent treelets leaf parent node
    gloost::gloostId getParentTreeletLeafsParentPosition() const;



    // returns the memory size in Bytes
    unsigned getMemSize() const;

    // returns number of nodes
    unsigned getNumNodes() const;
    // sets the number of nodes
    void setNumNodes(unsigned value);


    // returns number of leafes
    unsigned getNumLeaves() const;
    // sets the number of nodes
    void setNumLeaves(unsigned value);

    // returns the depth of the Treelets root node within the svo
    unsigned getRootNodeDepth() const;


    // returns the index of the first leaf node
    unsigned getFirstLeafIndex() const;


    // returns the slot Gid of this treelet within the incore buffer, -1 if not incore
    gloost::gloostId  getSlotGid() const;

    // sets the slot Gid of this treelet within the incore buffer, -1 if not incore
    void setSlotGid(gloost::gloostId incoreSlotGid);


	protected:

    gloost::gloostId    _treeletGid;
    unsigned            _rootNodeDepth;                     // number of treelets above this Treelet in the hierarchy
    gloost::gloostId    _parentTreeletGid;
    gloost::gloostId    _parentTreeletLeafPosition;        // position of the leaf within the parent Treelet
    gloost::gloostId    _parentTreeletLeafIdx;             // index 0...7 of the leaf in its parent
    gloost::gloostId    _parentTreeletLeafsParentPosition; // position of the leafs parent node within the parent Treelet

    unsigned            _memSize;
    unsigned            _numNodes;        // num inner nodes + num leaves (! num all nodes)
    unsigned            _numLeaves;


    // serialized svo nodes
    std::vector<CpuSvoNode> _serializedNodes;

    // container of all leaf QueueElements with smaller depth as required created while building the Treelet
    std::vector<QueueElement> _incompleteLeafQueueElements;

    // container of all leaf QueueElements with required or more depth
    std::vector<QueueElement> _finalLeafQueueElements;

    gloost::gloostId   _incoreSlotGid;  // slot Gid of this treelet within the incore buffer
};


} // namespace svo


#endif // H_SVO_TREELET


