
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
#include <CpuSvoNode.h>



/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class CpuSvoNode

  \brief Node of a Sparse Voxel Octree

  \author Felix Weiszig
  \date   December 2009
  \remarks ...
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

CpuSvoNode::CpuSvoNode():
 _firstchildIndex(0),
 _validMask(),
 _leafMask(),
 _attribPosition(0)
{

}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

CpuSvoNode::CpuSvoNode( unsigned        firstchildIndex,
                        const BitMask8& validMask,
                        const BitMask8& leafMask,
                        unsigned        attribPosition):
 _firstchildIndex(firstchildIndex),
 _validMask(validMask),
 _leafMask(leafMask),
 _attribPosition(attribPosition)
{

}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

CpuSvoNode::~CpuSvoNode()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns array of children
  \param ...
  \remarks ...
*/

unsigned
CpuSvoNode::getFirstChildIndex() const
{
  return _firstchildIndex;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief sets the child on index i
  \param ...
  \remarks ...
*/

void
CpuSvoNode::setFirstChildIndex(unsigned index)
{
  _firstchildIndex = index;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns child
  \param ...
  \remarks ...
*/

//CpuSvoNode*
//CpuSvoNode::getChild(unsigned i)
//{
//  return _children[i];
//}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns valid mask
  \param ...
  \remarks ...
*/

BitMask8&
CpuSvoNode::getValidMask()
{
  return _validMask;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns leaf mask
  \param ...
  \remarks ...
*/

BitMask8&
CpuSvoNode::getLeafMask()
{
  return _leafMask;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns true if node is a leaf
  \param ...
  \remarks ...
*/

bool
CpuSvoNode::isLeaf() const
{
  return !(bool) _validMask.getValue();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns attribute position in attrib buffer
  \param ...
  \remarks ...
*/

unsigned int
CpuSvoNode::getAttribPosition()
{
  return _attribPosition;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief sets the attribute position within the attribute buffer
  \param ...
  \remarks ...
*/

void
CpuSvoNode::setAttribPosition(unsigned int i)
{
  _attribPosition = i;
}


////////////////////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////////////////////





} // namespace svo


