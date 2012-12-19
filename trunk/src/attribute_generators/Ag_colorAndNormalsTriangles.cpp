
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



// svo system includes
#include <attribute_generators/Ag_colorAndNormalsTriangles.h>
#include <TreeletBuildManager.h>
#include <Treelet.h>
#include <BuilderTriangleFace.h>

// gloost includes
#include <gloost/Mesh.h>
#include <gloost/ObjMatFile.h>
#include <gloost/InterleavedAttributes.h>
#include <gloost/BinaryBundle.h>


// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   Ag_colorAndNormalsTriangles

  \brief   Retrieves colors and normals from Triangles of a Mesh

  \author  Felix Weiszig
  \date    March 2012
  \remarks ....
*/

///////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

Ag_colorAndNormalsTriangles::Ag_colorAndNormalsTriangles():
    AttributeGenerator()
{
	std::cerr << std::endl << "Attribut Generator: Ag_colorAndNormalsTriangles";
	std::cerr << std::endl << "  Retrieves colors and normals from Triangles of a Mesh. ";
}


///////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

Ag_colorAndNormalsTriangles::~Ag_colorAndNormalsTriangles()
{
}


///////////////////////////////////////////////////////////////////////////////


/**
  \brief   creates attributes for all final leafes of a treelet
  \remarks ...
*/

/*static*/
void
Ag_colorAndNormalsTriangles::createFinalLeafesAttributes( TreeletBuildManager* buildManager,
                                                          unsigned treeletGid,
                                                          gloost::Mesh* mesh)
{

  Treelet* treelet = buildManager->getTreelet(treeletGid);

//  std::cerr << std::endl << "Message from createFinalLeafesAttributes(): ";
//  std::cerr << std::endl << "             creating Attributes for " << treelet->getFinalLeafQueueElements().size() << " final leafes";

  // for all samples
  for (unsigned i=0; i!=treelet->getFinalLeafQueueElements().size(); ++i)
  {

    Treelet::QueueElement& queueElement = treelet->getFinalLeafQueueElements()[i];

    gloost::Vector3 color(0.0,0.0,0.0);
    gloost::Vector3 normal(0.0,0.0,0.0);

    for (unsigned i=0; i!=queueElement._primitiveIds.size(); ++i)
    {
      BuilderTriangleFace triangle(mesh, queueElement._primitiveIds[i]);

      gloost::Point3 voxelCenter = queueElement._aabbTransform * gloost::Point3(0.0,0.0,0.0);

      float u,v;
      triangle.calculateUAndV( voxelCenter, u, v);

      normal += triangle.interpolateNormal(u, v);
      color  += triangle.interpolateColor(u, v);
    }

    normal/= queueElement._primitiveIds.size();
    color/= queueElement._primitiveIds.size();

    // compress attributes
    unsigned compressedNormal = gloost::Vector3::compressAsNormal(normal);
    unsigned compressedColor  = gloost::Vector3::compressAsColor(color);


    // write attributes to attribute buffer
    unsigned nodeAttributePosition = buildManager->getAttributeBuffer(treeletGid)->getPackageIndex(queueElement._localLeafIndex);
    buildManager->getAttributeBuffer(treeletGid)->getVector()[nodeAttributePosition]   = gloost::unsigned_as_float(compressedNormal);
    buildManager->getAttributeBuffer(treeletGid)->getVector()[++nodeAttributePosition] = gloost::unsigned_as_float(compressedColor);
  }

  // clear all final leafes
  std::vector<Treelet::QueueElement>().swap(treelet->getFinalLeafQueueElements());

}


///////////////////////////////////////////////////////////////////////////////


/**
  \brief generates attributes for inner nodes by averaging child attribs
  \param node root node of the (sub)tree
  \remarks ...
*/

/*static*/
void
Ag_colorAndNormalsTriangles::createInnerNodesAttributes( TreeletBuildManager* buildManager,
                                                         gloost::gloostId     treeletGid,
                                                         gloost::gloostId     nodeIndex,
                                                         unsigned             currentDepth)
{
  CpuSvoNode& node = buildManager->getTreelet(treeletGid)->getNodeForIndex(nodeIndex);

  // call generateInnerNodesAttributes recursive
  for (unsigned i=0; i!=8; ++i)
  {
    if (node.getValidMaskFlag(i) && !node.getLeafMaskFlag(i))
    {
      createInnerNodesAttributes(buildManager,
                                 treeletGid,
                                 nodeIndex + node.getNthChildIndex(i),
                                 currentDepth+1);
    }
  }

  averageFromeChildAttributes(buildManager,
                              treeletGid,
                              nodeIndex,
                              currentDepth);
}


///////////////////////////////////////////////////////////////////////////////


/**
  \brief generates attributes for inner nodes by averaging child attribs
  \param node root node of the (sub)tree
  \remarks ...
*/

/*static*/
void
Ag_colorAndNormalsTriangles::recursive_createInnerNodesAttributes( TreeletBuildManager* buildManager,
                                                         gloost::gloostId     treeletGid,
                                                         gloost::gloostId     nodeIndex,
                                                         unsigned             currentDepth)
{
  CpuSvoNode& node = buildManager->getTreelet(treeletGid)->getNodeForIndex(nodeIndex);

  // call generateInnerNodesAttributes recursive
  for (unsigned i=0; i!=8; ++i)
  {
    if (node.getValidMaskFlag(i) && !node.getLeafMaskFlag(i))
    {
      createInnerNodesAttributes(buildManager,
                                 treeletGid,
                                 nodeIndex + node.getNthChildIndex(i),
                                 currentDepth+1);
    }
  }

  averageFromeChildAttributes(buildManager,
                              treeletGid,
                              nodeIndex,
                              currentDepth);
}


///////////////////////////////////////////////////////////////////////////////


/**
  \brief   creates attributes for all final leafes of a treelet
  \remarks ...
*/

/*static*/
void
Ag_colorAndNormalsTriangles::averageFromeChildAttributes( TreeletBuildManager* buildManager,
                                                          gloost::gloostId treeletGid,
                                                          gloost::gloostId nodeIndex,
                                                          unsigned depth)
{

  CpuSvoNode& node = buildManager->getTreelet(treeletGid)->getNodeForIndex(nodeIndex);;

  gloost::Vector3 avarageNormal(0.0,0.0,0.0);
  gloost::Vector3 avarageColor(0.0,0.0,0.0);

  unsigned numChildren = 0;

  for (unsigned i=0; i!=8; ++i)
  {
    if (node.getValidMaskFlag(i))
    {
      ++numChildren;

      unsigned childNodeIndex = nodeIndex + node.getNthChildIndex(i);
      unsigned attribPosition = buildManager->getAttributeBuffer(treeletGid)->getPackageIndex(childNodeIndex);

      float packedNormal = buildManager->getAttributeBuffer(treeletGid)->getVector()[attribPosition];
      float packedColor  = buildManager->getAttributeBuffer(treeletGid)->getVector()[attribPosition+1];

      avarageNormal += gloost::Vector3::uncompressAsNormal(gloost::float_as_unsigned(packedNormal));
    }

  }

  avarageNormal /= (float)numChildren;
  avarageColor  /= (float)numChildren;

  // write attributes to attribute buffer
  unsigned attribPosition = buildManager->getAttributeBuffer(treeletGid)->getPackageIndex(nodeIndex);
  buildManager->getAttributeBuffer(treeletGid)->getVector()[attribPosition++] = gloost::unsigned_as_float(gloost::Vector3::compressAsNormal(avarageNormal));
  buildManager->getAttributeBuffer(treeletGid)->getVector()[attribPosition]   = gloost::unsigned_as_float(gloost::Vector3::compressAsColor(avarageNormal));

}


///////////////////////////////////////////////////////////////////////////////
//
///**
//  \brief   ...
//  \remarks ...
//*/
//
//void
//Ag_colorAndNormalsTriangles::generate(std::vector<Treelet*>& treeletVector,
//                                      gloost::Mesh* mesh,
//                                      gloost::ObjMatFile* materials,
//                                      bool freeDiscreteSamplesAfterwards)
//{
//  if (!treeletVector.size())
//  {
//    std::cerr << std::endl << "ERROR in Ag_colorAndNormalsTriangles::generate: ";
//    std::cerr << std::endl << "         There are NO Treelets!: ";
//    return;
//  }
//
//  // num samples
//  unsigned numSamples = 0;
//  for (unsigned i=0; i!=treeletVector.size(); ++i)
//  {
////    numSamples += treeletVector->getFinalLeafQueueElements().size();
//  }
//
//
//  std::cerr << std::endl;
//  std::cerr << std::endl << "Message from Ag_colorAndNormalsTriangles::generate(): ";
//  std::cerr << std::endl << "  Uncompressed Attribute Buffer Size:  " << treeletVector.size()*treeletVector[0]->getMemSize();
//  std::cerr << std::endl << "  Samples to process for all Treelets: " << numSamples;
//  std::cerr << std::endl << "  Num of leaves:                      " << svo->getNumLeaves();


//  gloost::BinaryBundle attribBundle(svo->getDiscreteSampleLists().size()*6*sizeof(float));
//
//  _attributes[0]->dropReference();
//	_attributes[0] = new gloost::InterleavedAttributes();
//	_attributes[0]->takeReference();
//
//	_attributes[0]->addAttribute(3, 12, "normals");
//	_attributes[0]->addAttribute(3, 12, "colors");
//
//	_attributes[0]->resize(svo->getDiscreteSampleLists().size());
//
//
//  // for all lists with samples
//  for (unsigned i=0; i!=svo->getDiscreteSampleLists().size(); ++i)
//	{
//	  std::vector<DiscreteSample>& samples = svo->getDiscreteSampleList(i);
//
//    gloost::Vector3 color(0.0,0.0,0.0);
//    gloost::Vector3 normal(0.0,0.0,0.0);
//
//    // for all samples of a list
//    for (unsigned i=0; i!=samples.size(); ++i)
//    {
//      svo::BuilderTriangleFace triangle(mesh, samples[i]._primitiveId);
//
//      normal += triangle.interpolateNormal(samples[i]._u, samples[i]._v);
//      color  += triangle.interpolateColor(samples[i]._u, samples[i]._v);
//    }
//
//    // averaging with number of samples
//    normal /= (float) svo->getDiscreteSampleList(i).size();
//    color  /= (float) svo->getDiscreteSampleList(i).size();
//
//    unsigned packageIndex = _attributes[0]->getPackageIndex(i);
//
//    _attributes[0]->getVector()[packageIndex+0] = normal[0];
//    _attributes[0]->getVector()[packageIndex+1] = normal[1];
//    _attributes[0]->getVector()[packageIndex+2] = normal[2];
//
//    _attributes[0]->getVector()[packageIndex+3] = color[0];
//    _attributes[0]->getVector()[packageIndex+4] = color[1];
//    _attributes[0]->getVector()[packageIndex+5] = color[2];
//	}
//
/////	if (freeDiscreteSamplesAfterwards)
/////	{
/////	  svo->clearDiscreteSamples();
/////	}
//
//
//  std::cerr << std::endl;
//  std::cerr << std::endl << "  -> Generating attributes for inner nodes: ";
//
//  // generate non leaf nodes attributes
//	generateInnerNodesAttributesRecursive(svo->getRootNode(), 1);
//	generateCompressAttributes();
//}


///////////////////////////////////////////////////////////////////////////////


/**
  \brief   generates the attributes for one partucular node
  \remarks ...
*/

/*virtual*/
//void
//Ag_colorAndNormalsTriangles::generateCurrentNodesAttribs(SvoNode* node, unsigned depth)
//{
//  gloost::InterleavedAttributes* attribs = _attributes[0];
//
//  // average child attributes for this node
//  gloost::Vector3 averageNormal(0.0,0.0,0.0);
//  gloost::Vector3 averageColor(0.0,0.0,0.0);
//
//  unsigned numChildren = 0;
//
//  for (unsigned int c=0; c!=8; ++c)
//  {
//    // if child exists
//    if (node->getValidMask().getFlag(c))
//    {
//      ++numChildren;
//
//      // reading attribs of children
//
//      unsigned childAttribIndex  = node->getChild(c)->getAttribPosition();
//      unsigned elementIndex      = attribs->getPackageIndex(childAttribIndex);
//
//
//      // reading attribute components and accumulating
//
//      // normals
//      float nx = (*attribs).getVector()[elementIndex];
//      ++elementIndex;
//      float ny = (*attribs).getVector()[elementIndex];
//      ++elementIndex;
//      float nz = (*attribs).getVector()[elementIndex];
//      ++elementIndex;
//
//      averageNormal[0] += nx;
//      averageNormal[1] += ny;
//      averageNormal[2] += nz;
//
//      // colors
//      float r = (*attribs).getVector()[elementIndex];
//      ++elementIndex;
//      float g = (*attribs).getVector()[elementIndex];
//      ++elementIndex;
//      float b = (*attribs).getVector()[elementIndex];
//
//      averageColor[0] += r;
//      averageColor[1] += g;
//      averageColor[2] += b;
//    }
//  }
//
//  // averaging
//  if (numChildren)
//  {
//    averageNormal /= numChildren;
//    averageColor  /= numChildren;
//  }
//  averageNormal.normalize();
//
//  // set attribute position and push attributes
//  static unsigned numAttribPackages = attribs->getNumPackages();
//  node->setAttribPosition(numAttribPackages);
//  ++numAttribPackages;
//
//  attribs->getVector().push_back(averageNormal[0]);
//  attribs->getVector().push_back(averageNormal[1]);
//  attribs->getVector().push_back(averageNormal[2]);
//
//  attribs->getVector().push_back(averageColor[0]);
//  attribs->getVector().push_back(averageColor[1]);
//  attribs->getVector().push_back(averageColor[2]);
//}


///////////////////////////////////////////////////////////////////////////////


///**
//  \brief   compresses the attribute buffer and creates a coresponding attribute buffer with the values
//  \remarks ...
//*/
//
//*virtual*/
//void
//Ag_colorAndNormalsTriangles::generateCompressAttributes()
//{
//
//  std::cerr << std::endl;
//  std::cerr << std::endl << "Message from Ag_colorAndNormalsTriangles::compressAttributes(): ";
//  std::cerr << std::endl << "             Compressing: " << _attributes[0]->getNumPackages() << " attributes";
//
//  _compressedAttributes[0]->dropReference();
//	_compressedAttributes[0] = new gloost::InterleavedAttributes();
//	_compressedAttributes[0]->takeReference();
//
//	_compressedAttributes[0]->addAttribute(1, 4, "compressed_normals");
//	_compressedAttributes[0]->addAttribute(1, 4, "compressed_colors");
//
//	_compressedAttributes[0]->resize(_attributes[0]->getNumPackages());
//	_compressedAttributes[0]->fill(0.0f);
//
//
//	for (unsigned i=0; i!=_attributes[0]->getNumPackages(); ++i)
//	{
//	  std::vector<float>& sourceVector = _attributes[0]->getVector();
//	  std::vector<float>& destVector   = _compressedAttributes[0]->getVector();
//
//	  unsigned sourceIndex = _attributes[0]->getPackageIndex(i);
//	  unsigned destIndex   = _compressedAttributes[0]->getPackageIndex(i);
//
//
//    // compress normals
//    {
//      float nx = sourceVector[sourceIndex];
//      float ny = sourceVector[sourceIndex+1];
//      float nz = sourceVector[sourceIndex+2];
//
//      unsigned char nx8 = (unsigned char)((nx*0.5 + 0.5)*255);
//      unsigned char ny8 = (unsigned char)((ny*0.5 + 0.5)*255);
//      unsigned char nz8 = (unsigned char)((nz*0.5 + 0.5)*255);
//
//      unsigned packedNormal;
//      gloost::packRgbaToUnsigned(packedNormal, nx8, ny8, nz8, (unsigned char)0);
//      destVector[destIndex] = gloost::unsigned_as_float(packedNormal);
//    }
//
//
//    // compress colors
//    {
//      float red32   = sourceVector[sourceIndex+3];
//      float green32 = sourceVector[sourceIndex+4];
//      float blue32  = sourceVector[sourceIndex+5];
//
//      unsigned char red8   = (unsigned char)(gloost::clamp(red32, 0.0f, 1.0f)*255);
//      unsigned char green8 = (unsigned char)(gloost::clamp(green32, 0.0f, 1.0f)*255);
//      unsigned char blue8  = (unsigned char)(gloost::clamp(blue32, 0.0f, 1.0f)*255);
//
//      unsigned packedColor;
//      gloost::packRgbaToUnsigned(packedColor, red8, green8, blue8, (unsigned char)0);
//
//      destVector[destIndex+1] = gloost::unsigned_as_float(packedColor);
//    }
//	}
//
//
//}
//
//
///////////////////////////////////////////////////////
//
//
///**
//  \brief   writes one attribute buffer to a *.ia file
//  \remarks ...
//*/
//
//*virtual*/
//bool
//Ag_colorAndNormalsTriangles::writeAttributeBufferToFile(const std::string& filePath,
//                                                        unsigned id)
//{
//  // if buffer id is not valid
//  if ( !(id < _attributes.size()) )
//  {
//    return false;
//  }
//
//  std::cerr << std::endl;
//  std::cerr << std::endl << "Message from Ag_colorAndNormalsTriangles::writeAttributeBufferToFile():";
//  std::cerr << std::endl << "  Writing attribute buffer " << id << " to:";
//  std::cerr << std::endl << "    " << filePath;
//
//  return _attributes[id]->writeToFile(filePath);
//}
//
//
///////////////////////////////////////////////////////
//
//
///**
//  \brief   writes one compressed attribute buffer to a *.ia file
//  \remarks ...
//*/
//
//*virtual*/
//bool
//Ag_colorAndNormalsTriangles::writeCompressedAttributeBufferToFile(const std::string& filePath,
//                                                        unsigned id)
//{
//  // if buffer id is not valid
//  if ( !(id < _compressedAttributes.size()) )
//  {
//    return false;
//  }
//
//  std::cerr << std::endl;
//  std::cerr << std::endl << "Message from Ag_colorAndNormalsTriangles::writeCompressedAttributeBufferToFile():";
//  std::cerr << std::endl << "  Writing compressed attribute buffer " << id << " to:";
//  std::cerr << std::endl << "    " << filePath;
//
//  return _compressedAttributes[id]->writeToFile(filePath);
//}


/////////////////////////////////////////////////////




} // namespace svo



