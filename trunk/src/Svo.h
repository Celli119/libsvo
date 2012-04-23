
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



/// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/BoundingBox.h>
#include <gloost/Mesh.h>


/// svo includes
#include <SvoNode.h>


/// cpp includes
#include <string>
#include <list>
#include <vector>



namespace svo
{

  /// struct needed to serialize the svo nodes
  struct QueuedNode
  {
    /// a pointer to a node
    SvoNode* node;

    /// position within the serialized buffer
    unsigned int position;
  };


  /// struct contains the triangle id and u,v coordinates for an leaf voxel
  struct DiscreteSample
  {
    DiscreteSample(unsigned triangleId,
                   gloost::mathType u,
                   gloost::mathType v)
    {
      _triangleId = triangleId;
      _u          = u;
      _v          = v;
    }

    unsigned _triangleId;

    gloost::mathType _u;
    gloost::mathType _v;
  };



  //  Sparse Voxel Octree with utility functions

class Svo
{
	public:

    // class constructor
    Svo(int maxDepth);

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
    void writeSerialBuffersToFile(const std::string& directory, const std::string& basename);


    typedef std::vector< std::list<DiscreteSample> > SampleListVector;
    typedef std::list<DiscreteSample>                SampleList;

    // creates new set of samples for a leaf node and returns an id
    unsigned createDiscreteSampleList();

    // returns the vector of lists with DiscreteSamples
    std::vector< std::list<DiscreteSample> >& getDiscreteSampleLists();

    // returns a lists of DiscreteSamples
    std::list<DiscreteSample>& getDiscreteSampleList(unsigned id);



//    // pushs an attribute component
//    void pushAttributeComponent(float component);
//
//    // returns current attribute index;
//    unsigned int getCurrentAttribPosition() const;

    // returns the attribute buffer
//    std::vector<float>&  getAttributeBuffer();

//    // adds a element to the nomralizer vector
//    void pushNormalizer();
//
//    // increases the node count within the normalizer to normalize attribs for double voxels
//    void addDoubleNodeToNormalizer(unsigned attribPos);


    // returns the root node
    SvoNode* getRootNode();

    // returns the id of the serializedSvoBufferTexture within the gloost:TextureManager
    unsigned int getSvoBufferTextureId() const;
    // returns the id of the attributeBufferTexture within the gloost:TextureManager
//    unsigned int getAttributeBufferTextureId() const;


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
    float               _minVoxelSize;

    gloost::BoundingBox _boundingBox;

    int                 _numNodes;
    int                 _numLeaves;
    int                 _numOutOfBoundPoints;
    int                 _numDoublePoints;
    int                 _numOneChildNodes;

    std::vector< std::list<DiscreteSample> > _discreteSamples;
    unsigned                                 _discreteSampleIndex;


    // serialized svo nodes
    std::vector<float> _serializedSvoBuffer;
    unsigned int       _serializedSvoBufferTextureId;



    // inserts a point in the svo, builds the tree
    SvoNode* insertAndBuild(SvoNode*             currentParent,
                           int                   currentChildIndex,
                           SvoNode*              currentNode,
                           const gloost::Point3& localPos,
                           int                   currentDepth);


    // returns child index (0...7) for a coordinate in the curent voxel space
    int       getChildIndex(const gloost::Point3& point);



};


} // namespace svo


#endif // H_SVO_SVO


