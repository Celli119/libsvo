
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
#include <attribute_generators/Ag_colorAndNormalsTriangles.h>
#include <Svo.h>
#include <BuilderTriangleFace.h>

// gloost includes
#include <gloost/Mesh.h>
#include <gloost/ObjMatFile.h>
#include <gloost/InterleavedAttributes.h>
#include <gloost/BinaryBundle.h>


/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   Ag_colorAndNormalsTriangles

  \brief   Retrieves colors and normals from Triangles of a Mesh

  \author  Felix Weiszig
  \date    March 2012
  \remarks ....
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

Ag_colorAndNormalsTriangles::Ag_colorAndNormalsTriangles():
    AttributeGenerator()
{
	std::cerr << std::endl << "Attribut Generator: Ag_colorAndNormalsTriangles";
	std::cerr << std::endl << "  Retrieves colors and normals from Triangles of a Mesh. ";

  _attributes.push_back(new gloost::InterleavedAttributes());
  _attributes[0]->takeReference();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

Ag_colorAndNormalsTriangles::~Ag_colorAndNormalsTriangles()
{
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   ...
  \remarks ...
*/

void
Ag_colorAndNormalsTriangles::generate(Svo* svo,
                                      gloost::Mesh* mesh,
                                      gloost::ObjMatFile* materials,
                                      bool freeDiscreteSamplesAfterwards)
{

  std::cerr << std::endl << "  Num Lists of samples: " << svo->getDiscreteSampleLists().size();
  std::cerr << std::endl << "  Num of leaves:        " << svo->getNumLeaves();


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
      svo::BuilderTriangleFace triangle(mesh, (*sampleIt)._primitiveId);

      normal += triangle.interpolateNormal((*sampleIt)._u, (*sampleIt)._v);
      color  += triangle.interpolateColor((*sampleIt)._u, (*sampleIt)._v);
    }

    normal /= (float) svo->getDiscreteSampleList(i).size();
    color  /= (float) svo->getDiscreteSampleList(i).size();

    attribBundle.putFloat(normal[0]);
    attribBundle.putFloat(normal[1]);
    attribBundle.putFloat(normal[2]);
    attribBundle.putFloat(color[0]);
    attribBundle.putFloat(color[1]);
    attribBundle.putFloat(color[2]);
	}

	if (freeDiscreteSamplesAfterwards)
	{
	  svo->clearDiscreteSamples();
	}


  _attributes[0]->dropReference();
	_attributes[0] = new gloost::InterleavedAttributes(attribBundle);
	_attributes[0]->takeReference();

	_attributes[0]->addInterleavedAttrib(3, 12, "normals");
	_attributes[0]->addInterleavedAttrib(3, 12, "colors");

  std::cerr << std::endl;
  std::cerr << std::endl << "  -> Generating attributes for inner nodes: ";
	generateInnerNodesAttributesRecursive(svo->getRootNode(), 0);

}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   generates the attributes for one partucular node
  \remarks ...
*/

/*virtual*/
void
Ag_colorAndNormalsTriangles::generateCurrentNodesAttribs(SvoNode* node, unsigned depth)
{
  gloost::InterleavedAttributes* attribs = _attributes[0];

  // average child attributes for this node
  gloost::Vector3 averageNormal(0.0,0.0,0.0);
  gloost::Vector3 averageColor(0.0,0.0,0.0);
  unsigned int numChildren = 0;

  for (unsigned int c=0; c!=8; ++c)
  {
    // if child exists
    if (node->getValidMask().getFlag(c))
    {
      ++numChildren;

      // reading attribs of children
      unsigned childAttribIndex  = node->getChild(c)->getAttribPosition();
      unsigned elementIndex = attribs->getPackageIndex(childAttribIndex);

      // reading and accumulating
      float nx = (*attribs).getVector()[elementIndex];
      ++elementIndex;
      float ny = (*attribs).getVector()[elementIndex];
      ++elementIndex;
      float nz = (*attribs).getVector()[elementIndex];
      ++elementIndex;

      averageNormal[0] += nx;
      averageNormal[1] += ny;
      averageNormal[2] += nz;

      float r = (*attribs).getVector()[elementIndex];
      ++elementIndex;
      float g = (*attribs).getVector()[elementIndex];
      ++elementIndex;
      float b = (*attribs).getVector()[elementIndex];

      averageColor[0] += r;
      averageColor[1] += g;
      averageColor[2] += b;
    }
  }

  // averaging
  averageNormal /= numChildren;
  averageNormal.normalize();
  averageColor  /= numChildren;

  // set attribute position and push attributes
  node->setAttribPosition(attribs->getNumPackages());

  attribs->getVector().push_back(averageNormal[0]);
  attribs->getVector().push_back(averageNormal[1]);
  attribs->getVector().push_back(averageNormal[2]);

  attribs->getVector().push_back(averageColor[0]);
  attribs->getVector().push_back(averageColor[1]);
  attribs->getVector().push_back(averageColor[2]);
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   writes one attribute buffer to a *.ia file
  \remarks ...
*/

/*virtual*/
bool
Ag_colorAndNormalsTriangles::writeAttributeBufferToFile(const std::string& filePath,
                                                        unsigned id)
{
  // if buffer id is not valid
  if ( !(id < _attributes.size()) )
  {
    return false;
  }

  std::cerr << std::endl;
  std::cerr << std::endl << "Message from Ag_colorAndNormalsTriangles::writeAttributeBufferToFile():";
  std::cerr << std::endl << "  Writing attribute buffer " << id << " to:";
  std::cerr << std::endl << "    " << filePath;

  return _attributes[id]->writeToFile(filePath);
}


////////////////////////////////////////////////////////////////////////////////




} // namespace svo



