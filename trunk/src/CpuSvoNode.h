
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



#ifndef H_SVO_CPUSVONODE
#define H_SVO_CPUSVONODE



// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>
#include <gloost/UniformTypes.h>
#include <gloost/BitMask.h>


// svo system includes
#include <BitMask8.h>


// cpp includes
#include <string>

#define SVO_CPUSVONODE_OFFSET_LEAFMASK  0
#define SVO_CPUSVONODE_OFFSET_VALIDMASK 8
#define SVO_CPUSVONODE_NODE_SIZE        8  // size of a node in byte


namespace svo
{


  //  OctreeNode

class CpuSvoNode
{
	public:

    // class constructor
    CpuSvoNode();

    // class constructor
    CpuSvoNode( int             firstchildIndex,
                const BitMask8& validMask,
                const BitMask8& leafMask,
                unsigned        attribPosition);

    // class destructor
    ~CpuSvoNode();


    // returns array of children
    int getFirstChildIndex() const;

    // sets the child on index i
    void setFirstChildIndex(int index);

    // returns the nth child node index within the serialized data
    int getNthChildIndex(int childIndex);


    // sets the slot index of the corresponding treelet for this leaf
    void setLeafTreeletSlotGid(gloost::gloostId slotGid);



    // returns valid mask
    bool getValidMaskFlag(unsigned i);
    void setValidMaskFlag(unsigned i, bool flag);

    // returns leaf mask
    bool getLeafMaskFlag(unsigned i);
    void setLeafMaskFlag(unsigned i, bool flag);

	private:

    // ...
    int _firstChildIndex;

    gloost::BitMask _masks; // 8 bit leaf mask (0..7) + 8 bit valid mask (8..15)


};


} // namespace svo


#endif // H_SVO_CPUSVONODE


