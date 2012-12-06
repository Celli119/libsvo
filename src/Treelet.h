
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
#define SVO_TREELET_FILE_HEADER_SIZE 5u*sizeof(unsigned)

/// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/Matrix.h>
#include <gloost/Mesh.h>


/// svo includes
#include <CpuSvoNode.h>


/// cpp includes
#include <string>
#include <map>
#include <vector>

namespace gloost
{
//  class BinaryBundle;
  class BinaryFile;
}


namespace svo
{

  /// struct contains the triangle id and u,v coordinates for an leaf voxel
  struct DiscreteSample
  {
    DiscreteSample(unsigned primitiveId,
                   gloost::mathType u,
                   gloost::mathType v,
                   const gloost::Matrix& AABBTransform)
    {
      _primitiveId    = primitiveId;
      _u              = u;
      _v              = v;
      _AABBTransform = AABBTransform;
    }

    unsigned _primitiveId;

    gloost::mathType _u;
    gloost::mathType _v;
    gloost::Matrix   _AABBTransform;
  };

  //  Sparse Voxel Octree with utility functions

class Treelet
{
	public:

    ///
    struct QueueElement
    {
      QueueElement():
        _localNodeIndex(0u),
        _idx(0u),
        _parentLocalNodeIndex(0u),
        _depth(0u),
        _aabbTransform(),
        _primitiveIds()
      {
      }

      void clearPrimitiveIds()
      {
        _primitiveIds.clear();
      }

      unsigned              _localNodeIndex;         /// absolute local index of the node within the serial structure
      char                  _idx;                    /// index 0...7 of the node within its parent
      unsigned              _parentLocalNodeIndex;   /// index of the nodes parent within the serial structure
      unsigned              _depth;                  /// depth of the node within this treelet
      gloost::Matrix        _aabbTransform;          /// transformation of the AABB/voxel
      std::vector<unsigned> _primitiveIds;           /// primitive ids of all primitives contributing to this node
    };

    typedef std::vector< std::vector<DiscreteSample> > SampleListVector;
    typedef std::vector<DiscreteSample>                SampleList;


    // class constructor
    Treelet();


    // class constructor
    Treelet(gloost::gloostId  treeletGid,
            gloost::gloostId  parentTreeletGid,
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
    CpuSvoNode getNodeForIndex(unsigned index);


    // returns a map of QueueElement for each leafe assoziated with the leafes index within this Treelet
    std::vector<QueueElement>& getLeafQueueElements();


    // returs treeletId
    gloost::gloostId getTreeletGid() const;

    // returs parents treeletId
    gloost::gloostId getParentTreeletGid() const;



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


    // returns the index of the first leaf node
    unsigned getFirstLeafIndex() const;


    // returns the absolute position of the root node of this treelet within the gpu buffer
    unsigned getIncorePosition() const;
    // sets the absolute position of the Treelet within the gpu buffer
    void setIncorePosition(unsigned incorePos);


	protected:

    gloost::gloostId    _treeletGid;
    gloost::gloostId    _parentTreeletGid;

    unsigned            _memSize;
    unsigned            _numNodes;
    unsigned            _numLeaves;
    unsigned            _incorePosition;  // absolute index within the gpu buffer
//    int                 _firstLeafIndex;


    // serialized svo nodes
    std::vector<CpuSvoNode> _serializedNodes;

    // map of all leaf QueueElements created while building the Treelet
    std::vector<QueueElement> _leafQueueElements;


};


} // namespace svo


#endif // H_SVO_TREELET


