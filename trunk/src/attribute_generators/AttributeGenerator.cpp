
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



/// svo system includes
#include <attribute_generators/AttributeGenerator.h>
#include <Svo.h>

// gloost includes
#include <gloost/Mesh.h>
#include <gloost/ObjMatFile.h>
#include <gloost/TextureManager.h>


/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   AttributeGenerator

  \brief   A class template because I'm to lazy to write a class from scretch

  \author  Felix Weiszig
  \date    May 2011
  \remarks Lazy mans way to create a new class ;-)
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

AttributeGenerator::AttributeGenerator():
    _attributeTextureIds()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

AttributeGenerator::~AttributeGenerator()
{
	for (unsigned i=0; i!=_attributeTextureIds.size(); ++i)
	{
	  gloost::TextureManager::get()->dropReference(_attributeTextureIds[i]);
	}
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns a vector of texture ids, textures containing serialized attribute data
  \remarks ...
*/

std::vector<unsigned>&
AttributeGenerator::getAttribTextures()
{
	return _attributeTextureIds;
}


////////////////////////////////////////////////////////////////////////////////





} // namespace svo



