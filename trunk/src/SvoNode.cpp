
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
#include <SvoNode.h>
#include <Svo.h>



/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class SvoNode

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

SvoNode::SvoNode():
 _children(),
 _validMask(),
 _leafMask(),
 _attribPosition(SVO_EMPTY_ATTRIB_POS)
{
	_children[0] = 0;
	_children[1] = 0;
	_children[2] = 0;
	_children[3] = 0;
	_children[4] = 0;
	_children[5] = 0;
	_children[6] = 0;
	_children[7] = 0;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

SvoNode::~SvoNode()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns array of children
  \param ...
  \remarks ...
*/

SvoNode*
SvoNode::getChildren()
{
  return (*_children);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns child
  \param ...
  \remarks ...
*/

SvoNode*
SvoNode::getChild(unsigned i)
{
  return _children[i];
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief sets the child on index i
  \param ...
  \remarks ...
*/

void
SvoNode::setChild(unsigned i, SvoNode* node)
{
  _children[i] = node;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns valid mask
  \param ...
  \remarks ...
*/

BitMask8&
SvoNode::getValidMask()
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
SvoNode::getLeafMask()
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
SvoNode::isLeaf() const
{
  return !(bool) _validMask.getValue();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns attribute position in attrib buffer
  \param ...
  \remarks ...
*/

unsigned
SvoNode::getAttribPosition()
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
SvoNode::setAttribPosition(unsigned i)
{
  _attribPosition = i;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns serialized node size
  \param ...
  \remarks ...
*/

/*static*/
int
SvoNode::getSerializedNodeSize()
{
  // ... in Bit
  int memForChildPointer  = 32;
  int memForAttribPointer = 32;
  int memForFarBit        =  0;
  int memForValidMask     = 16;
  int memForLeafMask      = 16;

  return (  memForChildPointer
           +memForAttribPointer
           +memForFarBit
           +memForValidMask
           +memForLeafMask)/8;
}


////////////////////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////////////////////





} // namespace svo


