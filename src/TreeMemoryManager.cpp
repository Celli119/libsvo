
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



/// libsvo system includes
#include <TreeMemoryManager.h>
#include <Treelet.h>
#include <TreeletBuilderFromFaces.h>
#include <attribute_generators/Ag_colorAndNormalsTriangles.h>


// gloost system includes
#include <gloost/Mesh.h>


/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   TreeMemoryManager

  \brief   TreeMemoryManager, a subtree of the whole svo

  \author  Felix Weiszig
  \date    october 2012
  \remarks ...
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

TreeMemoryManager::TreeMemoryManager():
  _treelets()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

TreeMemoryManager::~TreeMemoryManager()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   builds trunk and all branches from triangle Mesh
  \param   ...
  \remarks ...
*/

void
TreeMemoryManager::buildFromFaces(unsigned treeletSizeInBytes,
                                  unsigned numBuildingThreads,
                                  gloost::Mesh* mesh)
{

//#ifndef GLOOST_SYSTEM_DISABLE_OUTPUT_MESSAGES
    std::cerr << std::endl;
    std::cerr << std::endl << "################################################";
    std::cerr << std::endl << "Message from TreeMemoryManager::buildFromFaces():";
    std::cerr << std::endl << "             Building ROOT treelet from triangles:";
//#endif


  // Build root Treelet
  _treelets.resize(1);
  {
    _treelets[0] = new Treelet(treeletSizeInBytes);
//    _treelets[0] = new Treelet(1*1024);

    svo::TreeletBuilderFromFaces fromTrianglesBuilder(numBuildingThreads);
    fromTrianglesBuilder.build(_treelets[0], mesh);

    // attribute generator
    //  svo::Ag_colorAndNormalsTriangles generator;
    //  generator.generate(_trunk, mesh, new gloost::ObjMatFile());

    _treelets[0]->writeTreeletToFile("/home/otaco/Desktop/SVO_DATA/"
                                     + std::string("TreeMemoryManager_out")
//                                     + "_" + gloost::toString(treeletSizeInBytes/1024)
                                                + std::string("_") + "0"
                                     + ".svo" );

    //  generator.writeCompressedAttributeBufferToFile("/home/otaco/Desktop/SVO_DATA/"
    //                                                 + std::string("TreeMemoryManager_out_trunk")
    //                                                 + "_" + gloost::toString(trunkDepth) + "_" + gloost::toString(branchDepth) + "c"
    //                                                 + ".ia" );
  }



  // Build Sub-Treelets
  unsigned numSubTreelets = _treelets[0]->getLeafQueueElements().size();
  _treelets.resize(numSubTreelets, 0);

  std::map<unsigned, Treelet::QueueElement>::iterator queueElementsIt    = _treelets[0]->getLeafQueueElements().begin();
  std::map<unsigned, Treelet::QueueElement>::iterator queueElementsEndIt = _treelets[0]->getLeafQueueElements().end();

  unsigned subTreeletId = 1u;

  for (; queueElementsIt!=queueElementsEndIt; ++queueElementsIt)
  {
    std::cerr << std::endl;
    std::cerr << std::endl << "################################################";
    std::cerr << std::endl;
    std::cerr << std::endl << "Message from SvoBuilderFaces::buildFromFaces():";
    std::cerr << std::endl << "             Building BRANCHE " << subTreeletId << " of " << numSubTreelets <<  " from triangle samples:";

    _treelets[subTreeletId] = new Treelet(treeletSizeInBytes);

    svo::TreeletBuilderFromFaces fromTrianglesBuilder(numBuildingThreads);
    fromTrianglesBuilder.build(_treelets[subTreeletId], mesh, queueElementsIt->second);

//    svo::Ag_colorAndNormalsTriangles generator2;
//    generator2.generate(_branches[i], mesh, new gloost::ObjMatFile());


    _treelets[subTreeletId]->writeTreeletToFile("/home/otaco/Desktop/SVO_DATA/"
                                                + std::string("TreeMemoryManager_out")
//                                              + "_" + gloost::toString(treeletSizeInBytes/1024)
                                                + std::string("_") + subTreeletId
                                                + ".svo" );

//    generator2.writeCompressedAttributeBufferToFile("/home/otaco/Desktop/SVO_DATA/"
//                                                   + std::string("TreeMemoryManager_out_")
//                                                   + i
//                                                   + "_" + gloost::toString(trunkDepth) + "_" + gloost::toString(branchDepth) + "c"
//                                                   + ".ia" );

    ++subTreeletId;
  }



}


////////////////////////////////////////////////////////////////////////////////





} // namespace svo

