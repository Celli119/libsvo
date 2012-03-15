
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
#include <attribute_generators/Ag_colorAndNormals.h>
#include <Svo.h>
#include <BuilderTriangleFace.h>

// gloost includes
#include <Mesh.h>
#include <ObjMatFile.h>
#include <InterleavedAttributes.h>
#include <BinaryBundle.h>


/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   Ag_colorAndNormals

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

Ag_colorAndNormals::Ag_colorAndNormals():
    AttributeGenerator(),
    _interleavedAttributBuffer(new gloost::InterleavedAttributes())
{
	std::cerr << std::endl << "Attribut Generator: ";
	std::cerr << std::endl << "  Retrieves colors and normals from Vertices of a Mesh.";

	_interleavedAttributBuffer->takeReference();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

Ag_colorAndNormals::~Ag_colorAndNormals()
{
	_interleavedAttributBuffer->dropReference();
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   ...
  \remarks ...
*/

void
Ag_colorAndNormals::generate(Svo* svo,
                             gloost::Mesh* mesh,
                             gloost::ObjMatFile* materials)
{

  std::cerr << std::endl << "lists:  " << svo->getDiscreteSampleLists().size();
  std::cerr << std::endl << "leaves: " << svo->getNumLeaves();


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

      normal += triangle.interpolateNormal((*sampleIt)._u, (*sampleIt)._v);
      color  += triangle.interpolateColor((*sampleIt)._u, (*sampleIt)._v);
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
Ag_colorAndNormals::getAttributeBuffer()
{
  return _interleavedAttributBuffer;
}



} // namespace svo



