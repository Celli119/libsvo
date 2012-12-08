
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
#include <TreeletBuildManager.h>
#include <Treelet.h>
#include <TreeletBuilderFromFaces.h>
#include <attribute_generators/Ag_colorAndNormalsTriangles.h>


// gloost system includes
#include <gloost/Mesh.h>
#include <gloost/BinaryFile.h>


/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   TreeletBuildManager

  \brief   TreeletBuildManager, a subtree of the whole svo

  \author  Felix Weiszig
  \date    october 2012
  \remarks ...
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

TreeletBuildManager::TreeletBuildManager():
  _treeletSizeInByte(0),
  _treelets()
{

}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

TreeletBuildManager::~TreeletBuildManager()
{
	for (unsigned i=0; i!=_treelets.size(); ++i)
	{
	  delete _treelets[i];
	}
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   builds trunk and all branches from triangle Mesh
  \param   ...
  \remarks ...
*/

void
TreeletBuildManager::buildFromFaces(unsigned treeletSizeInByte,
                                    unsigned maxSvoDepth,
                                    unsigned numBuildingThreads,
                                    gloost::Mesh* mesh)
{

  _treeletSizeInByte = treeletSizeInByte;

  std::cerr << std::endl;
  std::cerr << std::endl << "################################################";
  std::cerr << std::endl;
  std::cerr << std::endl << "Message from TreeletBuildManager::buildFromFaces():";
  std::cerr << std::endl << "             Building ROOT Treelet from triangles:";
  std::cerr << std::endl << "             Max SVO depth: " << maxSvoDepth;
  std::cerr << std::endl;


  // Build root Treelet
  _treelets.resize(1);
  {
    _treelets[0] = new Treelet(0u,
                               0u,
                               0u,
                               _treeletSizeInByte);

    svo::TreeletBuilderFromFaces fromTrianglesBuilder(maxSvoDepth);
    fromTrianglesBuilder.build(_treelets[0],
                               mesh);

    // attribute generator
    //  svo::Ag_colorAndNormalsTriangles generator;
    //  generator.generate(_trunk, mesh, new gloost::ObjMatFile());

//    _treelets[0]->writeToFile("/home/otaco/Desktop/SVO_DATA/"
//                               + std::string("TreeletBuildManager_out")
//                               + std::string("_") + "0"
//                               + ".treelet" );

    //  generator.writeCompressedAttributeBufferToFile("/home/otaco/Desktop/SVO_DATA/"
    //                                                 + std::string("TreeletBuildManager_out_trunk")
    //                                                 + "_" + gloost::toString(trunkDepth) + "_" + gloost::toString(branchDepth) + "c"
    //                                                 + ".ia" );
  }


#if 1
  /**
    Build Treelets as long as there are leafes that have not maxDepth within the svo
  */
  std::queue<Treelet*> treeletsWithSubTreeletsQueue;

  if (_treelets[0]->getLeafQueueElements().size())
  {
    treeletsWithSubTreeletsQueue.push(_treelets[0]);
  }

  unsigned treeletId      = 1u;
  unsigned queueItemCount = _treelets[0]->getLeafQueueElements().size();


  while(treeletsWithSubTreeletsQueue.size())
  {

    Treelet* parentTreelet = treeletsWithSubTreeletsQueue.front();
    treeletsWithSubTreeletsQueue.pop();

    std::cerr << std::endl;
    std::cerr << std::endl << "Treelet global queue size: " << treeletsWithSubTreeletsQueue.size();
    std::cerr << std::endl;

    // Build Sub-Treelets
    std::vector<Treelet::QueueElement>& parentQueueElements = parentTreelet->getLeafQueueElements();
    unsigned numSubTreelets = parentQueueElements.size();
    _treelets.resize(numSubTreelets+_treelets.size(), 0);


    for (unsigned i=0; i!=numSubTreelets; ++i)
    {
      std::cerr << std::endl;
      std::cerr << std::endl << "################################################";
      std::cerr << std::endl;
      std::cerr << std::endl << "Message from TreeletBuildManager::buildFromFaces():";
      std::cerr << std::endl << "             Building Treelet " << treeletId << " of " << queueItemCount <<  " from triangle samples (" << (queueItemCount*(float)treeletSizeInByte)*0.000976562*0.000976562*0.000976562 << " GB)";
      std::cerr << std::endl << "             Current depth:   " << parentQueueElements[i]._depth;

      _treelets[treeletId] = new Treelet( treeletId,
                                          parentTreelet->getTreeletGid(),
                                          parentQueueElements[i]._localLeafIndex,
                                          _treeletSizeInByte);

      svo::TreeletBuilderFromFaces fromTrianglesBuilder(maxSvoDepth);
      fromTrianglesBuilder.build(_treelets[treeletId],
                                 mesh,
                                 parentQueueElements[i]);

     // write Treelet Gid of the coresponding Treelet Gid to the parent Treelets leaf
     if (treeletId)
     {
       parentTreelet->getNodes()[parentQueueElements[i]._localLeafIndex].setFirstChildIndex(treeletId);
     }

      // push sub treelet to global queue if it has leafes with depth < maxDepth
      if (_treelets[treeletId]->getLeafQueueElements().size())
      {
        treeletsWithSubTreeletsQueue.push(_treelets[treeletId]);
        queueItemCount += _treelets[treeletId]->getLeafQueueElements().size();
      }

      // clear primitive ids since we need them anymore
      parentQueueElements[i].clearPrimitiveIds();

  //    svo::Ag_colorAndNormalsTriangles generator2;
  //    generator2.generate(_branches[i], mesh, new gloost::ObjMatFile());


//      _treelets[treeletId]->writeToFile("/home/otaco/Desktop/SVO_DATA/"
//                                         + std::string("TreeletBuildManager_out")
//                                         + std::string("_") + treeletId
//                                         + ".treelet" );

  //    generator2.writeCompressedAttributeBufferToFile("/home/otaco/Desktop/SVO_DATA/"
  //                                                   + std::string("TreeletBuildManager_out_")
  //                                                   + i
  //                                                   + "_" + gloost::toString(trunkDepth) + "_" + gloost::toString(branchDepth) + "c"
  //                                                   + ".ia" );

      ++treeletId;
    }
  }
#endif

  writeToFile( "/home/otaco/Desktop/SVO_DATA/"
               + std::string("TreeletBuildManager_out")
               + ".svo" );


}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   writes svo/Treelet structure to file
  \param   ...
  \remarks ...
*/

bool
TreeletBuildManager::writeToFile(const std::string& filePath) const
{
  std::cerr << std::endl;
  std::cerr << std::endl << "Message from TreeletBuildManager::writeToFile():";
  std::cerr << std::endl << "  Writing SVO to:";
  std::cerr << std::endl << "    " << filePath;

  if (!_treelets.size())
  {
    return false;
  }

  gloost::BinaryFile outFile;
  if (!outFile.openToWrite(filePath))
  {
    return false;
  }

  // write svo header
  outFile.writeUInt32(_treelets.size());
  outFile.writeUInt32(_treelets[0]->getMemSize());

  for (unsigned i=0; i!=_treelets.size(); ++i)
  {
    _treelets[i]->writeToFile(outFile);
  }


  outFile.close();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns a Treelet for a given Gid
  \param   ...
  \remarks ...
*/

Treelet*
TreeletBuildManager::getTreelet(gloost::gloostId id)
{
  return _treelets[id];
}


////////////////////////////////////////////////////////////////////////////////





} // namespace svo

