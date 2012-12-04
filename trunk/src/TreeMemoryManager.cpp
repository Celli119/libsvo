
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
  _treeletSizeInBytes(0),
  _treelets(),
  _treeletGpuBuffer()
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
                                  unsigned maxSvoDepth,
                                  unsigned numBuildingThreads,
                                  gloost::Mesh* mesh)
{

  _treeletSizeInBytes = treeletSizeInBytes;

  std::cerr << std::endl;
  std::cerr << std::endl << "################################################";
  std::cerr << std::endl;
  std::cerr << std::endl << "Message from TreeMemoryManager::buildFromFaces():";
  std::cerr << std::endl << "             Building ROOT Treelet from triangles:";
  std::cerr << std::endl << "             Max SVO depth: " << maxSvoDepth;
  std::cerr << std::endl;


  // Build root Treelet
  _treelets.resize(1);
  {
    _treelets[0] = new Treelet(0u,
                               0u,
                               _treeletSizeInBytes);

    svo::TreeletBuilderFromFaces fromTrianglesBuilder(maxSvoDepth);
    fromTrianglesBuilder.build(_treelets[0],
                               mesh);

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


#if 1
  /**
    Build Treelets as long as there are leafes that have not maxDepth within the svo
  */
  std::queue<Treelet*> _treeletsWithSubTreelets;

  if (_treelets[0]->getLeafQueueElements().size())
  {
    _treeletsWithSubTreelets.push(_treelets[0]);
  }

  unsigned treeletId      = 1u;
  unsigned queueItemCount = 1u;


  while(_treeletsWithSubTreelets.size())
  {
    Treelet* parentTreelet = _treeletsWithSubTreelets.front();
    _treeletsWithSubTreelets.pop();

    std::cerr << std::endl;
    std::cerr << std::endl << "Treelet global queue size: " << _treeletsWithSubTreelets.size();
    std::cerr << std::endl;

    // Build Sub-Treelets
    unsigned numSubTreelets = _treelets[parentTreelet->getTreeletGid()]->getLeafQueueElements().size();
    _treelets.resize(numSubTreelets+_treelets.size(), 0);

    std::map<unsigned, Treelet::QueueElement>::iterator queueElementsIt    = parentTreelet->getLeafQueueElements().begin();
    std::map<unsigned, Treelet::QueueElement>::iterator queueElementsEndIt = parentTreelet->getLeafQueueElements().end();


    for (; queueElementsIt!=queueElementsEndIt; ++queueElementsIt)
    {
      std::cerr << std::endl;
      std::cerr << std::endl << "################################################";
      std::cerr << std::endl;
      std::cerr << std::endl << "Message from TreeMemoryManager::buildFromFaces():";
      std::cerr << std::endl << "             Building Treelet " << treeletId << " of " << queueItemCount+numSubTreelets <<  " from triangle samples:";
      std::cerr << std::endl << "             current depth: " << queueElementsIt->second._depth;

      _treelets[treeletId] = new Treelet( treeletId,
                                          parentTreelet->getTreeletGid(),
                                          _treeletSizeInBytes);

      svo::TreeletBuilderFromFaces fromTrianglesBuilder(maxSvoDepth);
      fromTrianglesBuilder.build(_treelets[treeletId],
                                 mesh,
                                 queueElementsIt->second);

     // write Treelet Gid to the coresponding leaf element of the parent Treelet
     if (treeletId)
     {
       parentTreelet->getNodes()[queueElementsIt->first].setFirstChildIndex(treeletId);
     }





      // push sub treelet to global queue if it has leafes with depth < maxDepth
      if (_treelets[treeletId]->getLeafQueueElements().size())
      {
        _treeletsWithSubTreelets.push(_treelets[treeletId]);
        ++queueItemCount;
      }

      // clear primitive ids since we need them anymore
      queueElementsIt->second.clearPrimitiveIds();

  //    svo::Ag_colorAndNormalsTriangles generator2;
  //    generator2.generate(_branches[i], mesh, new gloost::ObjMatFile());


      _treelets[treeletId]->writeTreeletToFile("/home/otaco/Desktop/SVO_DATA/"
                                               + std::string("TreeMemoryManager_out")
  //                                           + "_" + gloost::toString(treeletSizeInBytes/1024)
                                               + std::string("_") + treeletId
                                               + ".svo" );

  //    generator2.writeCompressedAttributeBufferToFile("/home/otaco/Desktop/SVO_DATA/"
  //                                                   + std::string("TreeMemoryManager_out_")
  //                                                   + i
  //                                                   + "_" + gloost::toString(trunkDepth) + "_" + gloost::toString(branchDepth) + "c"
  //                                                   + ".ia" );

      ++treeletId;
    }
  }
#endif

  _treelets.clear();
}


////////////////////////////////////////////////////////////////////////////////





} // namespace svo

