
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
#include <TreeletBuildManager.h>
#include <Treelet.h>
#include <TreeletBuilderFromFaces.h>
#include <attribute_generators/Ag_colorAndNormalsTriangles.h>


// gloost system includes
#include <gloost/Mesh.h>
#include <gloost/BinaryFile.h>


// cpp includes
#include <string>
#include <iostream>
#include <queue>



namespace svo
{

/**
  \class   TreeletBuildManager

  \brief   TreeletBuildManager, a subtree of the whole svo

  \author  Felix Weiszig
  \date    october 2012
  \remarks ...
*/

//////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

TreeletBuildManager::TreeletBuildManager():
  _treeletSizeInByte(0),
  _treelets()
{

}


//////////////////////////////////////////////////////


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


//////////////////////////////////////////////////////


/**
  \brief   builds trunk and all branches from triangle Mesh
  \param   ...
  \remarks ...
*/

void
TreeletBuildManager::buildFromFaces(unsigned treeletSizeInByte,
                                    unsigned maxSvoDepth,
                                    unsigned numBuildingThreads,
                                    gloost::Mesh* mesh,
                                    const std::string& outFilePath)
{

  _treeletSizeInByte = treeletSizeInByte;

  std::cerr << std::endl;
  std::cerr << std::endl << "################################################";
  std::cerr << std::endl;
  std::cerr << std::endl << "Message from TreeletBuildManager::buildFromFaces():";
  std::cerr << std::endl << "             Building ROOT Treelet from triangles:";
  std::cerr << std::endl << "             Min SVO depth: " << maxSvoDepth;
  std::cerr << std::endl;


  // creating the Interleaved Attribute Layout
  gloost::InterleavedAttributes attributeProto;
  attributeProto.addAttribute(1, 4, "compressedNormal");
  attributeProto.addAttribute(1, 4, "compressedColor");


  // Build root Treelet
  _treelets.resize(1);
  _attributeBuffers.resize(1, new gloost::InterleavedAttributes());
  {
    _treelets[0] = new Treelet(0u,  // this treelet Gid
                               0u,  // root node depth
                               0u,  // parents treelet Gid
                               0u,  // leaf position within parent treelet
                               0u,  // idx (0...7) of the leaf in its parent
                               0u,  // position of leaves parent within the parent treelet
                               _treeletSizeInByte);


    _attributeBuffers[0] = new gloost::InterleavedAttributes(attributeProto.getLayout(),
                                                             _treelets[0]->getNodes().size());

    _attributeBuffers[0]->fill(0.0);

    svo::TreeletBuilderFromFaces fromTrianglesBuilder(maxSvoDepth);
    fromTrianglesBuilder.build(_treelets[0],
                               mesh);
  }


  /*
    Build Treelets as long as there are leafes that have not maxDepth within the svo
  */
  std::queue<Treelet*> treeletsWithSubTreeletsQueue;

  if (_treelets[0]->getIncompleteLeafQueueElements().size())
  {
    treeletsWithSubTreeletsQueue.push(_treelets[0]);
  }
  else
  {
    Ag_colorAndNormalsTriangles::createFinalLeafesAttributes(this,
                                                             0,
                                                             mesh);

    Ag_colorAndNormalsTriangles::createInnerNodesAttributes( this,
                                                             0);
  }

#if 1
  unsigned treeletId      = 1u;
  unsigned queueItemCount = _treelets[0]->getIncompleteLeafQueueElements().size();


  while(treeletsWithSubTreeletsQueue.size())
  {
    Treelet* parentTreelet = treeletsWithSubTreeletsQueue.front();
    treeletsWithSubTreeletsQueue.pop();


    // Build Sub-Treelets
    std::vector<Treelet::QueueElement>& parentIncompleteQueueElements = parentTreelet->getIncompleteLeafQueueElements();
    unsigned numSubTreelets = parentIncompleteQueueElements.size();
    _treelets.resize(numSubTreelets+_treelets.size(), 0);
    _attributeBuffers.resize(numSubTreelets+_treelets.size(), 0);


    for (unsigned i=0; i!=numSubTreelets; ++i)
    {

#ifdef SVO_BUILDING_VERBOSE
      std::cerr << std::endl;
      std::cerr << std::endl << "################################################";
      std::cerr << std::endl;
      std::cerr << std::endl << "Message from TreeletBuildManager::buildFromFaces():";
      std::cerr << std::endl << "             Building Treelet " << treeletId << " of " << queueItemCount <<  " from triangle samples (" << (queueItemCount*(float)treeletSizeInByte)*0.000976562*0.000976562*0.000976562 << " GB)";
      std::cerr << std::endl << "             Current depth:   " << parentIncompleteQueueElements[i]._depth;
#else
      if (treeletId % 1000 == 0)
      {
        float completeRatio = (float)treeletId/queueItemCount * 100.0;

        std::cerr << std::endl;
        std::cerr << std::endl << "Message from TreeletBuildManager::buildFromFaces():";
        std::cerr << std::endl << "            Building Treelet:   " << treeletId << " of " << queueItemCount;
        std::cerr << std::endl << "            progress:           " << completeRatio << " %";
        std::cerr << std::endl << "            estimated svo size: " << (queueItemCount*(float)treeletSizeInByte)*0.000976562*0.000976562*0.000976562 << " (x2) GB";
        std::cerr << std::endl << "            Current depth:      " << parentIncompleteQueueElements[i]._depth;
      }
#endif

      _treelets[treeletId] = new Treelet( treeletId,
                                          parentIncompleteQueueElements[i]._depth,
                                          parentTreelet->getTreeletGid(),
                                          parentIncompleteQueueElements[i]._localLeafIndex,
                                          parentIncompleteQueueElements[i]._idx,
                                          parentIncompleteQueueElements[i]._parentLocalNodeIndex,
                                          _treeletSizeInByte);

      _attributeBuffers[treeletId] = new gloost::InterleavedAttributes(attributeProto.getLayout(),
                                                                       _treelets[treeletId]->getNodes().size());

      _attributeBuffers[treeletId]->fill(0.0);

      svo::TreeletBuilderFromFaces fromTrianglesBuilder(maxSvoDepth);
      fromTrianglesBuilder.build(_treelets[treeletId],
                                 mesh,
                                 parentIncompleteQueueElements[i]);

     // write Treelet Gid of the coresponding Treelet Gid to the parent Treelets leaf
     if (treeletId)
     {
       parentTreelet->getNodes()[parentIncompleteQueueElements[i]._localLeafIndex].setFirstChildIndex(treeletId);
     }

      // push sub treelet to global queue if it has leafes with depth < maxDepth
      if (_treelets[treeletId]->getIncompleteLeafQueueElements().size())
      {
        treeletsWithSubTreeletsQueue.push(_treelets[treeletId]);
        queueItemCount += _treelets[treeletId]->getIncompleteLeafQueueElements().size();
      }

      if (_treelets[treeletId]->getFinalLeafQueueElements().size())
      {
        // create attributes for all final leaf samples and delete the final leafes samples
        Ag_colorAndNormalsTriangles::createFinalLeafesAttributes(this,
                                                                 treeletId,
                                                                 mesh);
      }

      ++treeletId;

    } // while(treeletsWithSubTreeletsQueue.size())


    // clear incomplete QueueElements since they are queued in the treeletsWithSubTreeletsQueue now
    std::vector<Treelet::QueueElement>().swap(parentTreelet->getIncompleteLeafQueueElements());

  }
#endif
  std::cerr << std::endl;
  std::cerr << std::endl << "Message from TreeletBuildManager::buildFromFaces():";
  std::cerr << std::endl << "             Creating inner nodes attributes";
  std::cerr << std::endl << "             for " << _treelets.size() << " treelets";

  for (int i=_treelets.size()-1; i!=-1; --i)
  {
//    std::cerr << std::endl << "## Creating attributes for Treelet: " << i;
    Ag_colorAndNormalsTriangles::createInnerNodesAttributes( this,
                                                             i);
  }


  writeToFile( outFilePath + std::string("TreeletBuildManager_out"));
}


///////////////////////////////////////////////////////////////////////////////


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
  std::cerr << std::endl << "             Writing SVO to:";
  std::cerr << std::endl << "             " << filePath<< ".svo";

  if (!_treelets.size())
  {
    return false;
  }

  gloost::BinaryFile outFile;
  if (!outFile.openToWrite(filePath + ".svo"))
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


  // write attributes
  if (!_attributeBuffers.size())
  {
    return false;
  }

  std::cerr << std::endl << "  Writing ATTRIBUTES to:";
  std::cerr << std::endl << "    " << filePath + ".ia";

  if (!outFile.openToWrite(filePath + ".ia"))
  {
    return false;
  }

  // write svo header
  outFile.writeUInt32(_attributeBuffers.size());

  for (unsigned i=0; i!=_treelets.size(); ++i)
  {
    _attributeBuffers[i]->writeToFile(outFile);
  }

  outFile.close();


  return true;
}


///////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns a Treelet for a given Gid
  \param   ...
  \remarks ...
*/

Treelet*
TreeletBuildManager::getTreelet(gloost::gloostId Gid)
{
  return _treelets[Gid];
}


///////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns a Treelet for a given Gid
  \param   ...
  \remarks ...
*/

gloost::InterleavedAttributes*
TreeletBuildManager::getAttributeBuffer(gloost::gloostId Gid)
{
  return _attributeBuffers[Gid];
}


//////////////////////////////////////////////////////





} // namespace svo

