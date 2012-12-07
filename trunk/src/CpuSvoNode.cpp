
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
 _firstChildIndex(0),
 _masks(0)
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
 _masks(0)
{
  _masks = validMask.getValue();
  _masks.getValue() <<= 8;
  _masks.getValue() |= leafMask.getValue();
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
    index += _masks.getFlag(i+SVO_CPUSVONODE_OFFSET_VALIDMASK);
  }

  return index;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   sets the slot index of the corresponding treelet for this leaf
  \param   ...
  \remarks ...
*/

void
CpuSvoNode::setLeafTreeletSlotGid(gloost::gloostId slotGid)
{
  _masks = slotGid;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns valid mask flag at position i
  \param ...
  \remarks ...
*/

bool
CpuSvoNode::getValidMaskFlag(unsigned i)
{
  return _masks.getFlag(i+SVO_CPUSVONODE_OFFSET_VALIDMASK);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief sets valid mask flag at position i
  \param ...
  \remarks ...
*/

void
CpuSvoNode::setValidMaskFlag(unsigned i, bool flag)
{
  return _masks.setFlag(i+SVO_CPUSVONODE_OFFSET_VALIDMASK, flag);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns leaf mask bit at position i
  \param ...
  \remarks ...
*/

bool
CpuSvoNode::getLeafMaskFlag(unsigned i)
{
  return _masks.getFlag(i+SVO_CPUSVONODE_OFFSET_LEAFMASK);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief sets leaf mask bit at position i
  \param ...
  \remarks ...
*/

void
CpuSvoNode::setLeafMaskFlag(unsigned i, bool flag)
{
  return _masks.setFlag(i+SVO_CPUSVONODE_OFFSET_LEAFMASK, flag);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns attribute position in attrib buffer
  \param ...
  \remarks ...
*/

//unsigned int
//CpuSvoNode::getAttribPosition() const
//{
//  return _attribPosition;
//}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief sets the attribute position within the attribute buffer
  \param ...
  \remarks ...
*/

//void
//CpuSvoNode::setAttribPosition(unsigned i)
//{
//  _attribPosition = i;
//}


////////////////////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////////////////////





} // namespace svo


