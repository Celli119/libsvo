
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



/// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>
#include <gloost/UniformTypes.h>


/// svo system includes
#include <BitMask8.h>


/// cpp includes
#include <string>


namespace svo
{


  //  OctreeNode

class CpuSvoNode
{
	public:

    // class constructor
    CpuSvoNode( );

    // class constructor
    CpuSvoNode( unsigned        firstchildIndex,
                const BitMask8& validMask,
                const BitMask8& leafMask,
                unsigned        attribPosition);

    // class destructor
    ~CpuSvoNode();


    // returns array of children
    unsigned getFirstChildIndex() const;

    // sets the child on index i
    void setFirstChildIndex(unsigned index);


    // returns the nth child node index within the serialized data
    unsigned getNthChildIndex(unsigned childIndex);



    // returns valid mask
    BitMask8& getValidMask();
    const BitMask8& getValidMask() const;

    // returns leaf mask
    BitMask8& getLeafMask();
    const BitMask8& getLeafMask() const;


    // returns true if node is a leaf
    bool isLeaf() const;


    // returns attribute position in attrib buffer
    unsigned int getAttribPosition() const;

    // sets the attribute position within the attribute buffer
    void setAttribPosition(unsigned i);


	private:

    // ...
    unsigned _firstChildIndex;

    BitMask8 _validMask;
    BitMask8 _leafMask;

    unsigned _attribPosition;

};


} // namespace svo


#endif // H_SVO_CPUSVONODE


