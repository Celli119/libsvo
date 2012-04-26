
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



/// gloost system includes
#include <gloost/TextureManager.h>
#include <gloost/BitMask.h>
#include <gloost/MatrixStack.h>
#include <gloost/BinaryFile.h>
#include <gloost/BinaryBundle.h>


#include <Svo.h>

/// cpp includes
#include <string>
#include <iostream>
#include <cmath>
#include <queue>



namespace svo
{

/**
  \class Svo

  \brief Sparse Voxel Octree with utility functions

  \author Felix Weiszig
  \date   Feb 2011
  \remarks ...
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

Svo::Svo(int maxDepth):
  _root(new SvoNode()),
  _maxDepth(maxDepth),
  _minVoxelSize(pow(2, -maxDepth)),
  _boundingBox(gloost::Point3(-0.5, -0.5, -0.5), gloost::Point3(0.5, 0.5, 0.5)),
  _numNodes(1),
  _numLeaves(0),
  _numOutOfBoundPoints(0),
  _numDoublePoints(0),
  _numOneChildNodes(0),
  _discreteSamples(),
  _discreteSampleIndex(0),
  _serializedSvoBuffer(),
  _serializedSvoBufferTextureId(0),
  _serializedSvoBundle(0)
{
//	createDiscreteSampleList();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

Svo::~Svo()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief inserts a position in the octree and returns its leaf node
  \param ...
  \remarks point should be inside the SVOs bounding box
*/

SvoNode*
Svo::insert(const gloost::Point3& point)
{

  // check point with bounding box of svo
//  for (unsigned int i=0; i!=3; ++i)
//  {
//    if (point[i] < _boundingBox.getPMin()[i] || point[i] > _boundingBox.getPMax()[i])
//    {
//      ++_numOutOfBoundPoints;
//      return 0;
//    }
//  }


  return insertAndBuild(0,     // parent
                        0,     // current child index
                        _root, // current node
                        point, // point in current voxel space
                        0);    // depth within the svo
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief inserts a point in the svo, builds the tree
  \param ...
  \remarks ...
*/

SvoNode*
Svo::insertAndBuild(SvoNode*              currentParent,
                    int                   currentChildIndex,
                    SvoNode*              currentNode,
                    const gloost::Point3& localPos,
                    int                   currentDepth)
{

  // max depth reached
  if ( currentDepth  == _maxDepth)
  {
    if (currentParent->getLeafMask().getFlag(currentChildIndex) == 0)
    {
      ++_numLeaves;
      currentParent->getLeafMask().setFlag(currentChildIndex, true);
      return currentNode;
    }
    else
    {
      ++_numDoublePoints;
      return currentParent->getChild(currentChildIndex);
    }
  }

  bool x = (localPos[0] >= 0);
  bool y = (localPos[1] >= 0);
  bool z = (localPos[2] >= 0);

  unsigned int choosenChildIndex = 4*x+2*y+z;

  // add a child node on this index if there wasn't one
  if (!currentNode->getValidMask().getFlag(choosenChildIndex))
  {
    currentNode->getValidMask().setFlag(choosenChildIndex, true);
    currentNode->setChild(choosenChildIndex, new SvoNode());
    ++_numNodes;
  }

  float currentScale = pow(2, -(int)currentDepth);
  float offset       = 0.25*currentScale;


  /// calculate local position
  gloost::Point3 chosenChildLocalPos(localPos[0] - (x*(offset)) + (!x*(offset)),
                                     localPos[1] - (y*(offset)) + (!y*(offset)),
                                     localPos[2] - (z*(offset)) + (!z*(offset)));

  return insertAndBuild( currentNode,
                         choosenChildIndex,
                         currentNode->getChild(choosenChildIndex),
                         chosenChildLocalPos,
                         currentDepth+1);

}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief serializes the svo and returns TextureManager id of resulting texture
  \param ...
  \remarks ...
*/

unsigned int
Svo::serializeSvo()
{
  // assuming RGB32 Texture
  // node = 3 x 32 Bit
  float numComponentsPerTexel = 3.0f; // RGB
  float bitsPerCoponent       = 32.0f;

  // assuming node size of 3 x 32 bit
  int   numTexels             = _numNodes;

  int textureSize = floor(sqrt((float)numTexels)+1);

  std::cerr << std::endl;
  std::cerr << std::endl << "Message from Svo::serializeSvo():";
  std::cerr << std::endl << "             Serialize SVO:";
  std::cerr << std::endl << "               Components per Texel:      " << numComponentsPerTexel;
  std::cerr << std::endl << "               Bits per component:        " << bitsPerCoponent << " Bit";
  std::cerr << std::endl << "               Number of texels:          " << numTexels;
  std::cerr << std::endl << "               Texture size:              " << textureSize << " x " << textureSize << " pix";
  std::cerr << std::endl << "               Texture memory size        " << (textureSize*textureSize * numComponentsPerTexel * bitsPerCoponent) / 8.0f / 1024.0f/ 1024.0f << " MB";


  if (textureSize > 8192)
  {
    std::cerr << std::endl << "SERIALIZED TEXTURE TO BIG!: " << textureSize;
    return 0;
  }

  // Add the root to the buffer
  gloost::BitMask packedMasks;

  for (unsigned int i=0; i!=8; ++i)
  {
    packedMasks.setFlag(i,   _root->getValidMask().getFlag(i));
    packedMasks.setFlag(i+8, _root->getLeafMask().getFlag(i));
  }

  QueuedNode queuedNode;
  queuedNode.node     = _root;
  queuedNode.position = _serializedSvoBuffer.size();

  _serializedSvoBuffer.push_back(0);                           // child pointer (unknown for now)
  _serializedSvoBuffer.push_back(packedMasks.getValue());      // packed masks
  _serializedSvoBuffer.push_back(_root->getAttribPosition());  // attribute position


  std::queue<QueuedNode> nodeQueue;
  nodeQueue.push(queuedNode);

  // add all nodes to the queue in a width search style
  while (nodeQueue.size())
  {
    QueuedNode currentParent = nodeQueue.front();
    nodeQueue.pop();

    for (unsigned int i=0; i!=8; ++i)
    {
      int childCount = 0;

      if (currentParent.node->getValidMask().getFlag(i))
      {
        ++childCount;

        // if this is the first child, put its position into parents childPointer field in the buffer
        if (childCount == 1)
        {
          _serializedSvoBuffer[currentParent.position] = _serializedSvoBuffer.size();
        }

        SvoNode* child = currentParent.node->getChild(i);

        // serialize child and add it to the queue and the serialized buffer
        gloost::BitMask packedMasks;

        for (unsigned int i=0; i!=8; ++i)
        {
          packedMasks.setFlag(i,   child->getValidMask().getFlag(i));
          packedMasks.setFlag(i+8, child->getLeafMask().getFlag(i));
        }

        QueuedNode queuedChild;
        queuedChild.node     = child;
        queuedChild.position = _serializedSvoBuffer.size();

        _serializedSvoBuffer.push_back(0);                           // child pointer (unknown for now)
        _serializedSvoBuffer.push_back(packedMasks.getValue());      // packed masks
        _serializedSvoBuffer.push_back(child->getAttribPosition());  // attribute position

        nodeQueue.push(queuedChild);
      }
    }
  }

  _serializedSvoBundle = new gloost::BinaryBundle((unsigned char*)&_serializedSvoBuffer.front(), _serializedSvoBuffer.size()*sizeof(unsigned));


  // fill up buffer to fit square texture size
  int fillTexels = 0;
  while (_serializedSvoBuffer.size()/(float)numComponentsPerTexel < textureSize*textureSize)
  {
    _serializedSvoBuffer.push_back(0);
    ++fillTexels;
  }


  std::cerr << std::endl << "               Empty texel in texture     " << fillTexels/3.0;


  // create a texture from the buffer
  gloost::Texture* buffer = new gloost::Texture(textureSize,
                                                textureSize,
                                                1,
                                                (unsigned char*)&_serializedSvoBuffer.front(),
                                                12,
                                                GL_TEXTURE_2D,
                                                GL_RGB32UI,
                                                GL_RGB,
                                                GL_FLOAT);

  _serializedSvoBufferTextureId = gloost::TextureManager::getInstance()->addTexture(buffer);



  std::cerr << std::endl << "               Texture id                 " << _serializedSvoBufferTextureId;
  std::cerr << std::endl;

}


////////////////////////////////////////////////////////////////////////////////

//
///**
//  \brief serializes the attribute buffer and returns TextureManager id of resulting texture
//  \param ...
//  \remarks ...
//*/
//
//unsigned int
//Svo::serializeAttributeBuffer()
//{
//  // assuming RG32 Texture
//  // normal = 3 x 32 Bit
//  // color  = 3 x 32 Bit
//
//  float numComponentsPerTexel = 3.0f;  // RGB
//  float bitsPerCoponent       = 32.0f;
//  int   numTexels             = _attributeBuffer.size()/numComponentsPerTexel;  // 6 components for each voxel
//
//  int textureSize = floor(sqrt((float)numTexels)+1);
//
//
//  if (textureSize > 8192)
//  {
//    std::cerr << std::endl << "SERIALIZED TEXTURE TO BIG!: " << textureSize;
//    return 0;
//  }
//
//
//  // fill up buffer to fit square texture size
//  int fillTexels = 0;
//  while (_attributeBuffer.size()/(float)numComponentsPerTexel < textureSize*textureSize)
//  {
//    _attributeBuffer.push_back(0.0);
//    ++fillTexels;
//  }
//
//  std::cerr << std::endl << "Message from Svo::serializeAttributeBuffer():";
//  std::cerr << std::endl << "             Serialize attribute buffer:";
//  std::cerr << std::endl << "               Components per Texel:      " << numComponentsPerTexel;
//  std::cerr << std::endl << "               Bits per component:        " << bitsPerCoponent << " Bit";
//  std::cerr << std::endl << "               Number of texels:          " << numTexels;
//  std::cerr << std::endl << "               Texture size:              " << textureSize << " x " << textureSize << " pix";
//  std::cerr << std::endl << "               Texture memory size        " << textureSize*textureSize * numComponentsPerTexel * bitsPerCoponent / 8.0f / 1024.0f/ 1024.0f << " MB";
//  std::cerr << std::endl << "               Empty texel in texture     " << fillTexels/3.0;
//
//
//  // create a texture from the buffer
//  gloost::Texture* buffer = new gloost::Texture(textureSize,
//                                                textureSize,
//                                                1,
//                                                (unsigned char*)&_attributeBuffer.front(),
//                                                12,
//                                                GL_TEXTURE_2D,
//                                                GL_RGB32F,
//                                                GL_RGB,
//                                                GL_FLOAT);
//
//  _attributeBufferTextureId = gloost::TextureManager::getInstance()->addTexture(buffer);
//
//  std::cerr << std::endl << "               Texture id                 " << _attributeBufferTextureId;
//
//
//  return _attributeBufferTextureId;
//}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief writes the serialized svo and attributes to a file
  \param ...
  \remarks ...
*/

bool
Svo::writeSerializedSvoToFile(const std::string& filePath)
{
//  // svo
//  if (_serializedSvoBufferTextureId)
//  {
//    // getting the texture for width and height
//    gloost::Texture* svoBufferTexture = gloost::TextureManager::getInstance()->getTextureWithoutRefcount(_serializedSvoBufferTextureId);
//
//    gloost::BinaryFile svoFile;
//    if (svoFile.openToWrite(directory + "/" + basename + "_svo.gbi"))
//    {
//      // header
//      svoFile.writeString("GBI ");
//      svoFile.writeString("components 3 ");
//      svoFile.writeString("format float ");
//      svoFile.writeString("sizex " + gloost::toString(svoBufferTexture->getWidth()) + " ");
//      svoFile.writeString("sizey " + gloost::toString(svoBufferTexture->getHeight()) + " ");
////      svoFile.writeString("sizez " + "0");//gloost::toString(svoBufferTexture->get()) + " ");
//      svoFile.writeString("order RGB ");
//
//      // data
//      svoFile.writeBuffer((unsigned char*)&_serializedSvoBuffer.front(), _serializedSvoBuffer.size()*sizeof(float));
//
//      svoFile.close();
//    }
//  }

  if (!_serializedSvoBundle)
  {
    serializeSvo();
  }

  std::cerr << std::endl;
  std::cerr << std::endl << "Message from Svo::writeSerializedSvoToFile():";
  std::cerr << std::endl << "  Writing SVO to:";
  std::cerr << std::endl << "    " << filePath;

  return gloost::BinaryFile::write(filePath, *_serializedSvoBundle);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief creates new set of samples for a leaf node and returns an id
  \param ...
  \remarks ...
*/

unsigned
Svo::createDiscreteSampleList()
{
  _discreteSamples.push_back(std::list<DiscreteSample>() );
  return _discreteSamples.size()-1;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the vector of lists of DiscreteSamples
  \param ...
  \remarks ...
*/

std::vector< std::list<DiscreteSample> >&
Svo::getDiscreteSampleLists()
{
  return _discreteSamples;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns a lists of DiscreteSamples
  \param ...
  \remarks ...
*/

std::list<DiscreteSample>&
Svo::getDiscreteSampleList(unsigned id)
{
  return _discreteSamples[id];
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief clears all discrete samples to save some mem
  \param ...
  \remarks ...
*/

void
Svo::clearDiscreteSamples()
{
  unsigned freedMemory = 0;

  for (unsigned i=0; i!=_discreteSamples.size(); ++i)
  {
    freedMemory += _discreteSamples[i].size()*sizeof(DiscreteSample);
    _discreteSamples[i].clear();
  }
  _discreteSamples.clear();

  std::cerr << std::endl;
  std::cerr << std::endl << "Message from Svo::clearDiscreteSamples(): ";
  std::cerr << std::endl << "  " << freedMemory/1024.0/1024.0 << " MB freed.";
}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns current attribute index;
  \param ...
  \remarks ...
*/

//unsigned int
//Svo::getCurrentAttribPosition() const
//{
//  return _attributeBuffer.size();
//}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the attribute buffer
  \param ...
  \remarks ...
*/

//std::vector<float>&
//Svo::getAttributeBuffer()
//{
//  return _attributeBuffer;
//}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief adds a element to the nomralizer vector
  \param ...
  \remarks ...
*/

//void
//Svo::pushNormalizer()
//{
//  _attribNormalizers.push_back(1);
//}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief increases the node count within the normalizer to normalize attribs for double voxels
  \param ...
  \remarks ...
*/

//void
//Svo::addDoubleNodeToNormalizer(unsigned attribPos)
//{
//  ++_attribNormalizers[attribPos];
//}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the root node
  \param ...
  \remarks ...
*/

SvoNode*
Svo::getRootNode()
{
  return _root;
}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the id of the attributeBufferTexture within the gloost:TextureManager
  \param ...
  \remarks You have to call Svo::serializeSvo() to create the texture first
*/

unsigned int
Svo::getSvoBufferTextureId() const
{
  return _serializedSvoBufferTextureId;
}
//
//////////////////////////////////////////////////////////////////////////////////
//
//
///**
//  \brief returns the id of the attributeBufferTexture within the gloost:TextureManager
//  \param ...
//  \remarks You have to call Svo::serializeAttributeBuffer() to create the texture first
//*/
//
//unsigned int
//Svo::getAttributeBufferTextureId() const
//{
//  return _attributeBufferTextureId;
//}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the BoundingBox of the Svo
  \param ...
  \remarks
*/

const gloost::BoundingBox&
Svo::getBoundingBox() const
{
  return _boundingBox;
}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the depth of the svo
  \param ...
  \remarks ...
*/

unsigned int
Svo::getMaxDepth() const
{
  return _maxDepth;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns number of nodes
  \param ...
  \remarks ...
*/

int
Svo::getNumNodes() const
{
  return _numNodes;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns number of leafes
  \param ...
  \remarks ...
*/

int
Svo::getNumLeaves() const
{
  return _numLeaves;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns number of points that didn't make it in the svo because they where out of bound
  \param ...
  \remarks ...
*/

int
Svo::getNumOutOfBoundPoints() const
{
  return _numOutOfBoundPoints;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns number of points that didn't make it in the svo because of there was allready one in the tree
  \param ...
  \remarks ...
*/

int
Svo::getNumDoublePoints() const
{
  return _numDoublePoints;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns number nodes with only one child
  \param ...
  \remarks ...
*/

int
Svo::getNumOneChildNodes() const
{
  return _numOneChildNodes;
}


////////////////////////////////////////////////////////////////////////////////

} // namespace svo


