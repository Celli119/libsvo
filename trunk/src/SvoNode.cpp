
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
 _attribPosition(0)
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
SvoNode::getChild(unsigned int i)
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
SvoNode::setChild(unsigned int i, SvoNode* node)
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

unsigned int
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
SvoNode::setAttribPosition(unsigned int i)
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


/**
  \brief returns a child index for a coordinate between (-0.5,-0.5,-0.5) and (0.5,0.5,0.5)
  \param ...
  \remarks ...
*/

/*static*/
unsigned int
SvoNode::getChildIndexForPosition(const gloost::Point3& pos)
{
  gloost::mathType delta = 0.00001;

  gloost::Point3 posBetter(pos);

  if (gloost::abs(posBetter[0]) < delta)
    posBetter[0] = 0;
  if (gloost::abs(posBetter[1]) < delta)
    posBetter[1] = 0;
  if (gloost::abs(posBetter[2]) < delta)
    posBetter[2] = 0;



  return 4*(pos[0] >= 0.0) + 2*(pos[1] >= 0.0) + (pos[2] >= 0.0);
//  return 4*(abs(pos[0]) >= delta) + 2*(abs(pos[1]) >= delta) + (abs(pos[2]) >= delta);
}


////////////////////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////////////////////





} // namespace svo


