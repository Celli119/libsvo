
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

namespace
{
  class BinaryBundle;
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
        _nodeIndex(0u),
        _idx(0u),
        _parentNodeIndex(0u),
        _depth(0u),
        _aabbTransform(),
        _primitiveIds()
      {
      }

      unsigned              _nodeIndex;         /// index of the node within the serial structure
      char                  _idx;               /// index 0...7 of the node within its parent
      unsigned              _parentNodeIndex;   /// index of the nodes parent within the serial structure
      unsigned              _depth;             /// depth of the node within this treelet
      gloost::Matrix        _aabbTransform;     /// transformation of the AABB/voxel
      std::vector<unsigned> _primitiveIds;      /// primitive ids of all primitives contributing to this node
    };

    typedef std::vector< std::vector<DiscreteSample> > SampleListVector;
    typedef std::vector<DiscreteSample>                SampleList;


    // class constructor
    Treelet(unsigned maxSizeInByte);

    // class constructor
    Treelet(const std::string treeletFilePath);

    // class destructor
	  virtual ~Treelet();



    // normalizes attribs of voxels storing contributions of more than one primitive during creation
    void normalizeLeafAttribs();

    // generates attributes for inner nodes by averaging child attribs
//    void generateInnerNodesAttributes(SvoNode* node, int currentDepth = 0);



    // writes the serialized svo and attributes to a file
    bool writeTreeletToFile(const std::string& filePath);

    // reads a serialized svo and attribute files
    bool loadTreeletFromFile(const std::string& filePath);



    // returns a vector with serialized nodes
    std::vector<CpuSvoNode>& getSerializedNodes();

    // returns a serialized SvoNode for an index
    CpuSvoNode getSerializedNodeForIndex(unsigned index);

    // returns a vector with serialized attribute indices
    std::vector<unsigned>& getSerializedAttributeIndices();



    // returns a map of QueueElement for each leafe assoziated with the leafes index within this Treelet
    std::map<unsigned, QueueElement>& getLeafQueueElements();



    // returns the max size in Bytes
    unsigned getMaxSize() const;

    // returns number of nodes
    int getNumNodes() const;

    // returns number of leafes
    int getNumLeaves() const;


	protected:
    int                 _maxSize;
    int                 _numNodes;
    int                 _numLeaves;


    // serialized svo nodes
    std::vector<CpuSvoNode> _serializedNodes;

    // map of all leaf QueueElements created while building the Treelet
    std::map<unsigned, QueueElement> _leafQueueElements;


};


} // namespace svo


#endif // H_SVO_TREELET


