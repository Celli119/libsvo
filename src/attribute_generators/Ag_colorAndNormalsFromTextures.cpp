
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
#include <attribute_generators/Ag_colorAndNormalsFromTextures.h>
#include <Svo.h>
#include <BuilderTriangleFace.h>

// gloost includes
#include <gloost/Mesh.h>
#include <gloost/ObjMatFile.h>
#include <gloost/InterleavedAttributes.h>
#include <gloost/BinaryBundle.h>
#include <gloost/Image.h>


/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   Ag_colorAndNormalsFromTextures

  \brief   ...

  \author  Felix Weiszig
  \date    March 2012
  \remarks Lazy mans way to create a new class ;-)
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

Ag_colorAndNormalsFromTextures::Ag_colorAndNormalsFromTextures():
    AttributeGenerator(),
    _interleavedAttributBuffer(new gloost::InterleavedAttributes())
{
	std::cerr << std::endl << "Attribut Generator: ";
	std::cerr << std::endl << "  Retrieves colors and normals from Vertices,";
	std::cerr << std::endl << "  color map and normal map";

	_interleavedAttributBuffer->takeReference();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

Ag_colorAndNormalsFromTextures::~Ag_colorAndNormalsFromTextures()
{
	_interleavedAttributBuffer->dropReference();
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   ...
  \remarks ...
*/

void
Ag_colorAndNormalsFromTextures::generate(Svo* svo,
                             gloost::Mesh* mesh,
                             gloost::ObjMatFile* materials)
{
  /// load images
//  gloost::Image map_color("../data/textures/240_color.jpg");
  gloost::Image map_color("../data/textures/testtex_512.png");
//  gloost::Image map_normal("../data/textures/240_normal.jpg");



  gloost::BinaryBundle attribBundle(svo->getDiscreteSampleLists().size()*6*sizeof(float));

  // for all lists with samples
  for (unsigned i=0; i!=svo->getDiscreteSampleLists().size(); ++i)
	{
    Svo::SampleList::iterator sampleIt    = svo->getDiscreteSampleList(i).begin();
    Svo::SampleList::iterator sampleEndIt = svo->getDiscreteSampleList(i).end();

    gloost::Vector3 color(0.0,0.0,0.0);
    gloost::Vector3 normal(0.0,0.0,0.0);

    // for all samples of a list
    for (; sampleIt!=sampleEndIt; ++sampleIt)
    {
      svo::BuilderTriangleFace triangle(mesh, (*sampleIt)._triangleId);

      gloost::Point3 texCoord = triangle.interpolateTexCoord((*sampleIt)._u, (*sampleIt)._v);
      texCoord[0] *= map_color.getWidth();
      texCoord[1] *= map_color.getHeight();

//      std::cerr << std::endl << "word: " << texCoord;

      normal += triangle.interpolateNormal((*sampleIt)._u, (*sampleIt)._v);
//      std::cerr << std::endl << "word: " << map_normal.getPixelColor(texCoord[0], texCoord[1]).rgb()*0.003921569 * 2.0 - gloost::Vector3(0.5, 0.5, 0.5);
//      color  += triangle.interpolateColor((*sampleIt)._u, (*sampleIt)._v);
      color  += map_color.getPixelColor(texCoord[0], texCoord[1]).rgb()*0.003921569;
    }

    normal /= svo->getDiscreteSampleList(i).size();
    color  /= svo->getDiscreteSampleList(i).size();

//    color = mult(color,color);

    attribBundle.putFloat(normal[0]);
    attribBundle.putFloat(normal[1]);
    attribBundle.putFloat(normal[2]);
    attribBundle.putFloat(color[0]);
    attribBundle.putFloat(color[1]);
    attribBundle.putFloat(color[2]);
	}

	_interleavedAttributBuffer->dropReference();
	_interleavedAttributBuffer = new gloost::InterleavedAttributes(attribBundle);
	_interleavedAttributBuffer->takeReference();

	_interleavedAttributBuffer->addInterleavedAttrib(3, 12, "normals");
	_interleavedAttributBuffer->addInterleavedAttrib(3, 12, "colors");
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   ...
  \remarks ...
*/

gloost::InterleavedAttributes*
Ag_colorAndNormalsFromTextures::getAttributeBuffer()
{
  return _interleavedAttributBuffer;
}



} // namespace svo



