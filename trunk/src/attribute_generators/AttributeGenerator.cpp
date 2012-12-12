//
//*
//                       ___                            __
//                      /\_ \                          /\ \__
//                   __ \//\ \     ___     ___     ____\ \  _\
//                 /'_ `\ \ \ \   / __`\  / __`\  /  __\\ \ \/
//                /\ \ \ \ \_\ \_/\ \ \ \/\ \ \ \/\__   \\ \ \_
//                \ \____ \/\____\ \____/\ \____/\/\____/ \ \__\
//                 \/___/\ \/____/\/___/  \/___/  \/___/   \/__/
//                   /\____/
//                   \_/__/
//
//                   OpenGL framework for fast demo programming
//
//                             http://www.gloost.org
//
//    This file is part of the gloost framework. You can use it in parts or as
//       whole under the terms of the GPL (http://www.gnu.org/licenses/#GPL).
//
//            gloost is being created by Felix Weißig and Stephan Beck
//
//     Felix Weißig (thesleeper@gmx.net), Stephan Beck (stephan@pixelstars.de)
//*/
//
//
//
//// svo system includes
//#include <attribute_generators/AttributeGenerator.h>
//
/// gloost includes
//#include <gloost/Mesh.h>
//#include <gloost/ObjMatFile.h>
//#include <gloost/TextureManager.h>
//#include <gloost/InterleavedAttributes.h>
//
//
//// cpp includes
//#include <string>
//#include <iostream>
//
//
//
//namespace svo
//{
//
//**
//  \class   AttributeGenerator
//
//  \brief   A class template because I'm to lazy to write a class from scretch
//
//  \author  Felix Weiszig
//  \date    May 2011
//  \remarks Lazy mans way to create a new class ;-)
//*/
//
///////////////////////////////////////////////////////
//
//
//**
//  \brief   Class constructor
//  \remarks ...
//*/
//
//AttributeGenerator::AttributeGenerator():
//    _attributes(),
//    _compressedAttributes()
//{
//	// insert your code here
//}
//
//
///////////////////////////////////////////////////////
//
//
//**
//  \brief   Class destructor
//  \remarks ...
//*/
//
//AttributeGenerator::~AttributeGenerator()
//{
//	for (unsigned i=0; i!=_attributes.size(); ++i)
//	{
//    _attributes[i]->dropReference();
//	}
//	for (unsigned i=0; i!=_compressedAttributes.size(); ++i)
//	{
//    _compressedAttributes[i]->dropReference();
//	}
//}
//
//
///////////////////////////////////////////////////////
//
//
//**
//  \brief   returns a vector of texture ids, textures containing serialized attribute data
//  \remarks ...
//*/
//
//gloost::InterleavedAttributes*
//AttributeGenerator::getAttributeBuffer(unsigned id)
//{
//	return _attributes[id];
//}
//
//
///////////////////////////////////////////////////////
//
//
//**
//  \brief generates attributes for inner nodes by averaging child attribs
//  \param node root node of the (sub)tree
//  \remarks ...
//*/
//
//void
//AttributeGenerator::generateInnerNodesAttributesRecursive(SvoNode* node, unsigned currentDepth)
//{
//  if (node->isLeaf())
//  {
//    return;
//  }
//
//  // call generateInnerNodesAttributes recursive
//  for (unsigned i=0; i!=8; ++i)
//  {
//    if (node->getValidMask().getFlag(i))
//    {
//      generateInnerNodesAttributesRecursive(node->getChild(i), currentDepth+1);
//    }
//  }
//
//  generateCurrentNodesAttribs(node, currentDepth);
//}
//
//
//
//
///////////////////////////////////////////////////////
//
//
//
//
//
//} // namespace svo
//
//
//
