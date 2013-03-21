
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

#include <contrib/TimerLog.h>

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
  if (!_incoreSlotsToUpload.size())
  {
    return;
  }

  // collect incore buffer ranges to upload
  gloost::bencl::ClBuffer* incoreClBuffer = _clContext->getClBuffer(_svoClBufferGid);
  gloost::bencl::ClDevice* device         = _clContext->getDevice(0);

  gloostTest::TimerLog::get()->putSample("zz_memory_manager-cl_slots_to_upload", _incoreSlotsToUpload.size());


  std::set<gloost::gloostId>::iterator slotGidIt    = _incoreSlotsToUpload.begin();
  std::set<gloost::gloostId>::iterator slotGidEndIt = _incoreSlotsToUpload.end();

//  std::cerr << std::endl;
//  std::cerr << std::endl << "_incoreSlotsToUpload.size(): " << _incoreSlotsToUpload.size();
  unsigned numCopyCalls = 0;

  while (slotGidIt!=slotGidEndIt)
  {

    gloost::gloostId startSlot = (*slotGidIt);
//    ++slotGidIt;

    unsigned numSlots    = 1;
    unsigned numTreelets = 1;



    float treeletToSlotRatio = 0.6f * 2.0f;


    if (slotGidIt!=slotGidEndIt)
    {

      while(slotGidIt!=slotGidEndIt)
      {
        int   slotDist = (*slotGidIt) - (int)startSlot;
        float ratio    = (numTreelets+1) / (float)(slotDist+1);

        if (ratio >= treeletToSlotRatio)
        {
          numSlots = slotDist+1;
          ++numTreelets;
          ++slotGidIt;
        }
        else
        {
          break;
        }

      }
    }

    ++numCopyCalls;

    // svo data
    unsigned srcIndex         = (startSlot)*_numNodesPerTreelet;
    unsigned destOffsetInByte = (startSlot)*getTreeletSizeInByte();

    int status = incoreClBuffer->enqueueWrite( device->getClCommandQueue(),
                                                false,
                                                destOffsetInByte,
                                                numSlots*getTreeletSizeInByte(),
                                                (const char*)&(_incoreBuffer[srcIndex]));

    // attrib data
    unsigned attribSrcIndex         = (startSlot)*_numNodesPerTreelet*_incoreAttributeBuffer->getNumElementsPerPackage();
    unsigned attribDestOffsetInByte = (startSlot)*_attributeBuffers[0]->getVector().size()*sizeof(float);

    gloost::bencl::ClBuffer* incoreAttributeClBuffer = _clContext->getClBuffer(_attributeClBufferGid);

    status = incoreAttributeClBuffer->enqueueWrite(device->getClCommandQueue(),
                                                   false,
                                                   attribDestOffsetInByte,
                                                   numSlots*_attributeBuffers[0]->getVector().size()*sizeof(float),
                                                   (const char*)&(_incoreAttributeBuffer->getVector()[attribSrcIndex]));
  }
  clFinish( device->getClCommandQueue() );

//  std::cerr << std::endl << "numCopyCalls: " << numCopyCalls;
//  std::cerr << std::endl << "            : " << (float)numCopyCalls/_incoreSlotsToUpload.size() * 100.0 << " %";

  _incoreSlotsToUpload.erase(_incoreSlotsToUpload.begin(), _incoreSlotsToUpload.end());



  gloostTest::TimerLog::get()->putSample("zz_memory_manager-num_copy_calls", numCopyCalls);




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

