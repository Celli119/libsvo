
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



#ifndef H_SVO_SVO_NODE
#define H_SVO_SVO_NODE



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

class SvoNode
{
	public:

    // class constructor
    SvoNode();

    // class destructor
    ~SvoNode();


    // returns array of children
    SvoNode* getChildren();

    // returns array of children
    SvoNode* getChild(unsigned i);

    // sets the child on index i
    void setChild(unsigned i, SvoNode* node);


    // returns valid mask
    BitMask8& getValidMask();

    // returns leaf mask
    BitMask8& getLeafMask();


    // returns true if node is a leaf
    bool isLeaf() const;


    // returns attribute position in attrib buffer
    unsigned getAttribPosition();

    // sets the attribute position within the attribute buffer
    void setAttribPosition(unsigned i);


    // returns serialized node size
    static int getSerializedNodeSize();

	private:

    // ...
    SvoNode* _children[8];

    BitMask8 _validMask;
    BitMask8 _leafMask;

    unsigned _attribPosition;

};


} // namespace svo


#endif // H_SVO_SVO_NODE


