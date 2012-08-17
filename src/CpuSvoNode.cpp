
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
#include <Svo.h>



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
 _firstChildIndex(0),
 _validMask(),
 _leafMask(),
 _attribPosition(SVO_EMPTY_ATTRIB_POS)
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
 _firstChildIndex(firstchildIndex),
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
  return _firstChildIndex;
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
  _firstChildIndex = index;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the nth child node index within the serialized data
  \param ...
  \remarks ...
*/

unsigned
CpuSvoNode::getNthChildIndex(unsigned childIndex)
{
  unsigned index = _firstChildIndex;

  for (unsigned i=0; i!=childIndex; ++i)
  {
    index += _validMask.getFlag(i);
  }

  return index;
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
  \brief returns valid mask
  \param ...
  \remarks ...
*/

const BitMask8&
CpuSvoNode::getValidMask() const
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
  \brief returns leaf mask
  \param ...
  \remarks ...
*/

const BitMask8&
CpuSvoNode::getLeafMask() const
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
CpuSvoNode::getAttribPosition() const
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
CpuSvoNode::setAttribPosition(unsigned i)
{
  _attribPosition = i;
}


////////////////////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////////////////////





} // namespace svo


