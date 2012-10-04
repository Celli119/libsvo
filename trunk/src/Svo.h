
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



#ifndef H_SVO_SVO
#define H_SVO_SVO

#define SVO_EMPTY_ATTRIB_POS 999999999

/// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/BoundingBox.h>
#include <gloost/Mesh.h>


/// svo includes
#include <SvoNode.h>
#include <CpuSvoNode.h>


/// cpp includes
#include <string>
#include <list>
#include <vector>

namespace
{
  class BinaryBundle;
}


namespace svo
{

  /// struct needed to serialize the svo nodes
  struct QueuedNode
  {
    /// a pointer to a node
    SvoNode* node;

    /// position within the serialized buffer
    unsigned index;
  };


  /// struct contains the triangle id and u,v coordinates for an leaf voxel
  struct DiscreteSample
  {
    DiscreteSample(unsigned primitiveId,
                   gloost::mathType u,
                   gloost::mathType v)
    {
      _primitiveId = primitiveId;
      _u           = u;
      _v           = v;
    }

    unsigned _primitiveId;

    gloost::mathType _u;
    gloost::mathType _v;
  };



  //  Sparse Voxel Octree with utility functions

class Svo
{
	public:

    // class constructor
    Svo(int maxDepth);

    // class constructor
    Svo(const std::string svoFilePath);

    // class destructor
	  virtual ~Svo();


	  // inserts a position in the octree and returns its leaf node
    SvoNode* insert(const gloost::Point3& point);


    // normalizes attribs of voxels storing contributions of more than one primitive during creation
    void normalizeLeafAttribs();

    // generates attributes for inner nodes by averaging child attribs
    void generateInnerNodesAttributes(SvoNode* node, int currentDepth = 0);



    // serializes the svo and returns TextureManager id of resulting texture
    unsigned int serializeSvo();

    // serializes the attribute buffer and returns TextureManager id of resulting texture
    unsigned int serializeAttributeBuffer();

    // writes the serialized svo and attributes to a file
    bool writeSerializedSvoToFile(const std::string& filePath);

    // reads a serialized svo and attribute files
    bool loadSerializedSvoFromFile(const std::string& filePath);


    // returns a vector with serialized nodes
    std::vector<CpuSvoNode>& getSerializedNodes();

    // returns a serialized SvoNode for an index
    CpuSvoNode getSerializedNodeForIndex(unsigned index);



    typedef std::vector< std::vector<DiscreteSample> > SampleListVector;
    typedef std::vector<DiscreteSample>                SampleList;

    // creates new set of samples for a leaf node and returns an id
    unsigned createDiscreteSampleList();

    // returns the vector of lists with DiscreteSamples
    std::vector< SampleList >& getDiscreteSampleLists();

    // returns a lists of DiscreteSamples
    SampleList& getDiscreteSampleList(unsigned id);

    // returns the number of discrete samples currently stored
    unsigned getNumDiscreteSamples() const;

    // clears all discrete samples to save some mem
    void clearDiscreteSamples();



    // returns the root node
    SvoNode* getRootNode();


    // returns the BoundingBox of the Svo
    const gloost::BoundingBox& getBoundingBox() const;



    // returns the depth of the svo
    unsigned int getMaxDepth() const;

    // returns number of nodes
    int getNumNodes() const;

    // returns number of leafes
    int getNumLeaves() const;

    // returns number of points that didn't make it in the svo because they where out of bound
    int getNumOutOfBoundPoints() const;

    // returns number of points that didn't make it in the svo because of there was allready one in the tree
    int getNumDoublePoints() const;

    // returns number nodes with only one child
    int getNumOneChildNodes() const;



	protected:

    SvoNode*            _root;
    int                 _maxDepth;

    gloost::BoundingBox _boundingBox;

    int                 _numNodes;
    int                 _numLeaves;
    int                 _numOutOfBoundPoints;
    int                 _numDoublePoints;
    int                 _numOneChildNodes;

    std::vector< std::vector<DiscreteSample> > _discreteSamples;
    unsigned                                   _discreteSampleIndex;


    // serialized svo nodes
    std::vector<CpuSvoNode> _serializedCpuSvoNodes;
    std::vector<unsigned>   _serializedAttributeIndices;
    gloost::BinaryBundle*   _serializedSvoBundle;



    // inserts a point in the svo, builds the tree
    SvoNode* insertAndBuild(SvoNode*             currentParent,
                           int                   currentChildIndex,
                           SvoNode*              currentNode,
                           const gloost::Point3& localPos,
                           int                   currentDepth);


    // returns child index (0...7) for a coordinate in the curent voxel space
    int getChildIndex(const gloost::Point3& point);



};


} // namespace svo


#endif // H_SVO_SVO


