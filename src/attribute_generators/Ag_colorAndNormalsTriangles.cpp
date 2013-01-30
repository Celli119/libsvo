
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
#include <gloost/Vector3.h>


// cpp includes
#include <string>
#include <iostream>
#include <omp.h>



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
  omp_set_num_threads(4);
  #pragma omp parallel for
  for (unsigned i=0; i<treelet->getFinalLeafQueueElements().size(); ++i)
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

      u = gloost::clamp(u, 0.0f, 1.0f);
      v = gloost::clamp(v, 0.0f, 1.0f);

      normal += triangle.interpolateNormal(u, v);
      color  += triangle.interpolateColor(u, v);
    }

    normal /= queueElement._primitiveIds.size();
    color  /= queueElement._primitiveIds.size();

    normal.normalize();

    color[0] = gloost::clamp(color[0], (gloost::mathType)0.0, (gloost::mathType)1.0);
    color[1] = gloost::clamp(color[1], (gloost::mathType)0.0, (gloost::mathType)1.0);
    color[2] = gloost::clamp(color[2], (gloost::mathType)0.0, (gloost::mathType)1.0);

    // compress attributes
    unsigned compressedNormal = gloost::Vector3::compressAsNormal(normal);
    unsigned compressedColor  = gloost::Vector3::compressAsColor(color);


    // write attributes to attribute buffer
    unsigned nodeAttributePosition = buildManager->getAttributeBuffer(treeletGid)->getPackageIndex(queueElement._localLeafIndex);
    buildManager->getAttributeBuffer(treeletGid)->getVector()[nodeAttributePosition++] = gloost::unsigned_as_float(compressedNormal);
    buildManager->getAttributeBuffer(treeletGid)->getVector()[nodeAttributePosition]   = gloost::unsigned_as_float(compressedColor);
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
                                                         gloost::gloostId     treeletGid)
{
  // start bottom up attribute averaging
  recursive_createInnerNodesAttributes( buildManager,
                                        treeletGid,
                                        0,
                                        0);

  // propergate root attributes to parent treelets corresponding leaf
  if (treeletGid)
  {
    Treelet* treelet = buildManager->getTreelet(treeletGid);

    gloost::InterleavedAttributes* treeletAttributebuffer =  buildManager->getAttributeBuffer(treeletGid);
    unsigned sourceIndex = treeletAttributebuffer->getPackageIndex(0);

    gloost::InterleavedAttributes* parentTreeletAttributebuffer =  buildManager->getAttributeBuffer(treelet->getParentTreeletGid());
    unsigned destIndex = parentTreeletAttributebuffer->getPackageIndex(treelet->getParentTreeletLeafPosition());

    parentTreeletAttributebuffer->getVector()[destIndex++] = treeletAttributebuffer->getVector()[sourceIndex++];
    parentTreeletAttributebuffer->getVector()[destIndex]   = treeletAttributebuffer->getVector()[sourceIndex];

//    std::cerr << std::endl << "propergate from: " << treeletGid << " to " << treelet->getParentTreeletGid() << " at " << treelet->getParentTreeletLeafPosition();
  }
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
      recursive_createInnerNodesAttributes(buildManager,
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
  gloost::Vector3 averageColor(0.0,0.0,0.0);

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
      averageColor  += gloost::Vector3::uncompressAsColor(gloost::float_as_unsigned(packedColor));
    }

  }

  avarageNormal /= (float)numChildren;
  averageColor  /= (float)numChildren;

  avarageNormal.normalize();

  unsigned compressedNormal = gloost::Vector3::compressAsNormal(avarageNormal);

  averageColor[0] = gloost::clamp(averageColor[0], (gloost::mathType)0.0, (gloost::mathType)1.0);
  averageColor[1] = gloost::clamp(averageColor[1], (gloost::mathType)0.0, (gloost::mathType)1.0);
  averageColor[2] = gloost::clamp(averageColor[2], (gloost::mathType)0.0, (gloost::mathType)1.0);

  unsigned compressedColor = gloost::Vector3::compressAsColor(averageColor);

  // write attributes to attribute buffer
  unsigned attribPosition = buildManager->getAttributeBuffer(treeletGid)->getPackageIndex(nodeIndex);
  buildManager->getAttributeBuffer(treeletGid)->getVector()[attribPosition++] = gloost::unsigned_as_float(compressedNormal);
  buildManager->getAttributeBuffer(treeletGid)->getVector()[attribPosition]   = gloost::unsigned_as_float(compressedColor);


}

/////////////////////////////////////////////////////




} // namespace svo



