
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



// libsvo system includes
#include <TreeletMemoryManagerCl.h>
#include <Treelet.h>
#include <TreeletBuilderFromFaces.h>
#include <attribute_generators/Ag_colorAndNormalsTriangles.h>


// gloost system includes
#include <gloost/Mesh.h>
#include <gloost/BinaryFile.h>


// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   TreeletMemoryManagerCl

  \brief   TreeletMemoryManagerCl, a subtree of the whole svo

  \author  Felix Weiszig
  \date    december 2012
  \remarks ...
*/

//////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

TreeletMemoryManagerCl::TreeletMemoryManagerCl(const std::string& svoFilePath,
                                               unsigned incoreBufferSizeInByte,
                                               gloost::bencl::ClContext* clContext):
  TreeletMemoryManager(svoFilePath,incoreBufferSizeInByte),
  _clContext(clContext),
  _svoClBufferGid(0),
  _attributeClBufferGid(0)
{


}


//////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

TreeletMemoryManagerCl::~TreeletMemoryManagerCl()
{
	// insert your code here
}


//////////////////////////////////////////////////////


/**
  \brief   inits the cl buffer
  \remarks ...
*/

void
TreeletMemoryManagerCl::initClBuffer()
{
//  CL_MEM_COPY_HOST_PTR | CL_MEM_ALLOC_HOST_PTR

	_svoClBufferGid = _clContext->createClBuffer(CL_MEM_COPY_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,
                                              (unsigned char*)&getIncoreBuffer().front(),
                                              getIncoreBuffer().size()*sizeof(CpuSvoNode));

	_attributeClBufferGid = _clContext->createClBuffer(CL_MEM_COPY_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,
                                                    (unsigned char*)&getAttributeIncoreBuffer()->getVector().front(),
                                                    getAttributeIncoreBuffer()->getVector().size()*sizeof(float));
}


//////////////////////////////////////////////////////


/**
  \brief   returns the Gid of the ClBuffer containing the SVO
  \remarks ...
*/

gloost::gloostId
TreeletMemoryManagerCl::getClIncoreBufferGid() const
{
	return _svoClBufferGid;
}


//////////////////////////////////////////////////////


/**
  \brief   returns the Gid of the ClBuffer containing the ATTRIBUTES
  \remarks ...
*/

gloost::gloostId
TreeletMemoryManagerCl::getClAttributeIncoreBufferGid() const
{
	return _attributeClBufferGid;
}


//////////////////////////////////////////////////////


/**
  \brief   updates device memory by uploading incore buffer slots
  \remarks ...
*/

/*virtual*/
void
TreeletMemoryManagerCl::updateDeviceMemory()
{

  // collect incore buffer ranges to upload
  gloost::bencl::ClBuffer* incoreClBuffer = _clContext->getClBuffer(_svoClBufferGid);
  gloost::bencl::ClDevice* device         = _clContext->getDevice(0);

  std::set<gloost::gloostId>::iterator slotGidIt    = _incoreSlotsToUpload.begin();
  std::set<gloost::gloostId>::iterator slotGidEndIt = _incoreSlotsToUpload.end();

  if (!_incoreSlotsToUpload.size())
  {
    return;
  }

//  static const unsigned maxUploadAmount = 1024u*1204u;
//  unsigned numBytesUploaded             = 0u;


  while (/*numBytesUploaded < maxUploadAmount &&*/ slotGidIt!=slotGidEndIt)
  {

    // svo data
    unsigned srcIndex         = (*slotGidIt)*_numNodesPerTreelet;
    unsigned destOffsetInByte = (*slotGidIt)*getTreeletSizeInByte();

    int status = incoreClBuffer->enqueueWrite( device->getClCommandQueue(),
                                                false,
                                                destOffsetInByte,
                                                getTreeletSizeInByte(),
                                                (const char*)&(_incoreBuffer[srcIndex]));


    // attrib data
    unsigned attribSrcIndex         = (*slotGidIt)*_numNodesPerTreelet*_incoreAttributeBuffer->getNumElementsPerPackage();
    unsigned attribDestOffsetInByte = (*slotGidIt)*_attributeBuffers[0]->getVector().size()*sizeof(float);

    gloost::bencl::ClBuffer* incoreAttributeClBuffer = _clContext->getClBuffer(_attributeClBufferGid);

    status = incoreAttributeClBuffer->enqueueWrite(device->getClCommandQueue(),
                                                   false,
                                                   attribDestOffsetInByte,
                                                   _attributeBuffers[0]->getVector().size()*sizeof(float),
                                                   (const char*)&(_incoreAttributeBuffer->getVector()[attribSrcIndex]));

    ++slotGidIt;
  }
  clFinish( device->getClCommandQueue() );

  _incoreSlotsToUpload.erase(_incoreSlotsToUpload.begin(), slotGidIt);
}


//////////////////////////////////////////////////////


/**
  \brief   returns the ClContext
  \remarks ...
*/

/*virtual*/
gloost::bencl::ClContext*
TreeletMemoryManagerCl::getContext()
{
  return _clContext;
}


//////////////////////////////////////////////////////





} // namespace svo

