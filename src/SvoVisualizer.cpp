
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
#include <SvoVisualizer.h>
#include <Svo.h>
#include <SvoNode.h>


/// gloost system includes
#include <gloost/Mesh.h>
#include <gloost/Vbo.h>
#include <gloost/MatrixStack.h>
#include <gloost/GbmWriter.h>
#include <gloost/InterleavedAttributes.h>


/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class SvoVisualizer

  \brief Crates different kinds of visualization of a given Svo. Provides a
         gloost::Mesh with lines or faces


  \author Felix Weiszig
  \date   March 2011
  \remarks ...
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

SvoVisualizer::SvoVisualizer(unsigned maxDepth, unsigned mode):
  _svo(0),
  _maxDepth(maxDepth),
  _matrixStack(),
  _mesh(new gloost::Mesh()),
  _vbo(new gloost::Vbo(_mesh)),
  _mode(mode),
  _numWireBoxes(0),
  _numSolidBoxes(0),
  _vertexIndex(0)
{
	_mesh->takeReference();


  if (_mode == SVO_VISUALIZER_MODE_BOXED_LEAFES)
  {
    _mesh->setSupportedInterleavedAttribute(GLOOST_MESH_POSITIONS, true);
    _mesh->setSupportedInterleavedAttribute(GLOOST_MESH_COLORS, true);
  }

	_vbo->takeReference();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

SvoVisualizer::~SvoVisualizer()
{
	_mesh->dropReference();
	_vbo->dropReference();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief builds the visualization from a svo::Svo
  \param ...
  \remarks ...
*/

void
SvoVisualizer::build(Svo* svo, gloost::InterleavedAttributes* attributes)
{

  _svo        = svo;
  _attributes = attributes;

  if (svo->getMaxDepth() < _maxDepth)
  {
    _maxDepth = svo->getMaxDepth();
  }

#ifndef GLOOST_SYSTEM_DISABLE_OUTPUT_MESSAGES
    std::cerr << std::endl;
    std::cerr << std::endl << "Message from SvoVisualizer::build(Svo* svo):";
    std::cerr << std::endl << "             Building SVO visualisation:";
    std::cerr << std::endl << "               max depth                  " << _maxDepth;
    std::cerr << std::endl << "               Number of nodes:           " << svo->getNumNodes();
    std::cerr << std::endl << "               Number of Leaves:          " << svo->getNumLeaves();


#endif

  gloost::BoundingBox bbox(gloost::Point3(-0.5,-0.5,-0.5), gloost::Point3(0.5,0.5,0.5));
  // push a box for each node
  if (_mode == SVO_VISUALIZER_MODE_WIRED_NODES)
  {
    pushWireCubeToMesh(bbox,
                       0);
  }


  buildRecursive( svo->getRootNode(),
                  gloost::Vector3(0.0, 0.0, 0.0),
                  0);

#ifndef GLOOST_SYSTEM_DISABLE_OUTPUT_MESSAGES
    std::cerr << std::endl << "               Number of wireframe boxes: " << _numWireBoxes;
    std::cerr << std::endl << "               Number of solid boxes:     " << _numSolidBoxes;
    std::cerr << std::endl;

#endif

  _mesh->recalcBoundingBox();

  _svo = 0;


//  _mesh->mergeVertices();
  _mesh->interleave(true);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief builds the visualization from a svo::Svo
  \param ...
  \remarks ...
*/

void
SvoVisualizer::buildRecursive( SvoNode*               node,
                               const gloost::Vector3& offsetToParent,
                               unsigned               currentDepth)
{

  // max depth reached
//  if ( node->isLeaf() )
  if ( currentDepth  == _maxDepth || node->isLeaf())
  {
      gloost::BoundingBox bbox(gloost::Point3(-0.5,-0.5,-0.5), gloost::Point3(0.5,0.5,0.5));
      bbox.transform(_matrixStack.top());

      // push a box for each node
      if (_mode == SVO_VISUALIZER_MODE_WIRED_NODES)
      {
        pushWireCubeToMesh(bbox,
                           currentDepth);
      }

      // push a box for leafs only
      if (_mode == SVO_VISUALIZER_MODE_BOXED_LEAFES)
      {
        pushSolidCubeToMesh(bbox,
                            currentDepth,
                            node->getAttribPosition());
      }

      return;
  }


  float offset = 0.25;

  for (int z=0; z!=2; ++z)
  {
    for (int y=0; y!=2; ++y)
    {
      for (int x=0; x!=2; ++x)
      {

        int choosenChildIndex = 4*x+2*y+z;

        if (node->getValidMask().getFlag(choosenChildIndex))
        {
          gloost::Vector3 childOffset(0.0,0.0,0.0);

          if (x)
          {
            childOffset[0] += offset;
          }
          else
          {
            childOffset[0] -= offset;
          }


          if (y)
          {
            childOffset[1] += offset;
          }
          else
          {
            childOffset[1] -= offset;
          }


          if (z)
          {
            childOffset[2] += offset;
          }
          else
          {
            childOffset[2] -= offset;
          }


          _matrixStack.push();
          {
            _matrixStack.translate(childOffset[0], childOffset[1], childOffset[2] );
            _matrixStack.scale(0.5);

            gloost::BoundingBox bbox(gloost::Point3(-0.5,-0.5,-0.5), gloost::Point3(0.5,0.5,0.5));

            bbox.transform(_matrixStack.top());

            // push a box for each node
            if (_mode == SVO_VISUALIZER_MODE_WIRED_NODES)
            {
              pushWireCubeToMesh(bbox,
                                 currentDepth);
            }

            if ( currentDepth  < _maxDepth)
            {
              buildRecursive(node->getChild(choosenChildIndex),
                             childOffset,
                             currentDepth+1);
            }

          }
          _matrixStack.pop();

        }
      }
    }
  }

}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief adds a wireframe cube to the mesh
  \param ...
  \remarks ...
*/

void
SvoVisualizer::pushWireCubeToMesh(const gloost::BoundingBox& bbox,
                                  unsigned currentDepth,
                                  const gloost::vec4& color)
{


  ++_numWireBoxes;

/*
      _pMin
            g-----h
           /     /|
          /     / |0
          c----d  f
    e --> |    | /
          |    |/
          a----b
               _pMax
  */

  gloost::Point3 a(bbox.getPMin()[0], bbox.getPMax()[1], bbox.getPMax()[2]);
  gloost::Point3 b(bbox.getPMax());
  gloost::Point3 c(bbox.getPMin()[0], bbox.getPMin()[1], bbox.getPMax()[2]);
  gloost::Point3 d(bbox.getPMax()[0], bbox.getPMin()[1], bbox.getPMax()[2]);
  gloost::Point3 e(bbox.getPMin()[0], bbox.getPMax()[1], bbox.getPMin()[2]);
  gloost::Point3 f(bbox.getPMax()[0], bbox.getPMax()[1], bbox.getPMin()[2]);
  gloost::Point3 g(bbox.getPMin());
  gloost::Point3 h(bbox.getPMax()[0], bbox.getPMin()[1], bbox.getPMin()[2]);


  unsigned int aIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(a);
  unsigned int bIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(b);
  unsigned int cIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(c);
  unsigned int dIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(d);
  unsigned int eIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(e);
  unsigned int fIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(f);
  unsigned int gIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(g);
  unsigned int hIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(h);


//  float ratio = currentDepth/(float)_maxDepth;

  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);


  _mesh->getLines().push_back(gloost::Line(aIndex,bIndex));
  _mesh->getLines().push_back(gloost::Line(aIndex,cIndex));
  _mesh->getLines().push_back(gloost::Line(aIndex,eIndex));
  _mesh->getLines().push_back(gloost::Line(hIndex,dIndex));
  _mesh->getLines().push_back(gloost::Line(hIndex,gIndex));
  _mesh->getLines().push_back(gloost::Line(hIndex,fIndex));
  _mesh->getLines().push_back(gloost::Line(cIndex,gIndex));
  _mesh->getLines().push_back(gloost::Line(cIndex,dIndex));
  _mesh->getLines().push_back(gloost::Line(fIndex,eIndex));
  _mesh->getLines().push_back(gloost::Line(fIndex,bIndex));
  _mesh->getLines().push_back(gloost::Line(gIndex,eIndex));
  _mesh->getLines().push_back(gloost::Line(bIndex,dIndex));
}




////////////////////////////////////////////////////////////////////////////////


/**
  \brief adds a wireframe cube to the mesh
  \param ...
  \remarks ...
*/

void
SvoVisualizer::pushWireCubeToMesh(const gloost::BoundingBox& bbox,
                                  unsigned currentDepth)
{


  ++_numWireBoxes;

/*
      _pMin
            g-----h
           /     /|
          /     / |0
          c----d  f
    e --> |    | /
          |    |/
          a----b
               _pMax
  */

  gloost::Point3 a(bbox.getPMin()[0], bbox.getPMax()[1], bbox.getPMax()[2]);
  gloost::Point3 b(bbox.getPMax());
  gloost::Point3 c(bbox.getPMin()[0], bbox.getPMin()[1], bbox.getPMax()[2]);
  gloost::Point3 d(bbox.getPMax()[0], bbox.getPMin()[1], bbox.getPMax()[2]);
  gloost::Point3 e(bbox.getPMin()[0], bbox.getPMax()[1], bbox.getPMin()[2]);
  gloost::Point3 f(bbox.getPMax()[0], bbox.getPMax()[1], bbox.getPMin()[2]);
  gloost::Point3 g(bbox.getPMin());
  gloost::Point3 h(bbox.getPMax()[0], bbox.getPMin()[1], bbox.getPMin()[2]);


  unsigned int aIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(a);
  unsigned int bIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(b);
  unsigned int cIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(c);
  unsigned int dIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(d);
  unsigned int eIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(e);
  unsigned int fIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(f);
  unsigned int gIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(g);
  unsigned int hIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(h);


  float ratio = currentDepth/(float)_maxDepth;
  gloost::vec4 color (ratio,
                      ratio,
                      1.0 - ratio,
                      0.5 * (1.0 - ratio*0.25));

  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);
  _mesh->getColors().push_back(color);


  _mesh->getLines().push_back(gloost::Line(aIndex,bIndex));
  _mesh->getLines().push_back(gloost::Line(aIndex,cIndex));
  _mesh->getLines().push_back(gloost::Line(aIndex,eIndex));
  _mesh->getLines().push_back(gloost::Line(hIndex,dIndex));
  _mesh->getLines().push_back(gloost::Line(hIndex,gIndex));
  _mesh->getLines().push_back(gloost::Line(hIndex,fIndex));
  _mesh->getLines().push_back(gloost::Line(cIndex,gIndex));
  _mesh->getLines().push_back(gloost::Line(cIndex,dIndex));
  _mesh->getLines().push_back(gloost::Line(fIndex,eIndex));
  _mesh->getLines().push_back(gloost::Line(fIndex,bIndex));
  _mesh->getLines().push_back(gloost::Line(gIndex,eIndex));
  _mesh->getLines().push_back(gloost::Line(bIndex,dIndex));
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief adds a solid cube to the mesh
  \param ...
  \remarks ...
*/

void
SvoVisualizer::pushSolidCubeToMesh(const gloost::BoundingBox& bbox,
                                   unsigned currentDepth,
                                   unsigned attributePosition)
{


  attributePosition*=_attributes->getNumElementsPerPackage();

  ++_numSolidBoxes;

/*
      _pMinattributePosition
            g-----h
           /     /|
          /     / |0
          c----d  f
    e --> |    | /
          |    |/
          a----b
               _pMax
  */

  gloost::Point3 a(bbox.getPMin()[0], bbox.getPMax()[1], bbox.getPMax()[2]);
  gloost::Point3 b(bbox.getPMax());
  gloost::Point3 c(bbox.getPMin()[0], bbox.getPMin()[1], bbox.getPMax()[2]);
  gloost::Point3 d(bbox.getPMax()[0], bbox.getPMin()[1], bbox.getPMax()[2]);
  gloost::Point3 e(bbox.getPMin()[0], bbox.getPMax()[1], bbox.getPMin()[2]);
  gloost::Point3 f(bbox.getPMax()[0], bbox.getPMax()[1], bbox.getPMin()[2]);
  gloost::Point3 g(bbox.getPMin());
  gloost::Point3 h(bbox.getPMax()[0], bbox.getPMin()[1], bbox.getPMin()[2]);


  unsigned int aIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(a);
  unsigned int bIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(b);
  unsigned int cIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(c);
  unsigned int dIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(d);
  unsigned int eIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(e);
  unsigned int fIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(f);
  unsigned int gIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(g);
  unsigned int hIndex = _mesh->getVertices().size();
  _mesh->getVertices().push_back(h);


  if (_attributes->getVector().size())
  {
    gloost::Vector3 normal;
    gloost::vec4 color;

    normal[0] = _attributes->getVector()[attributePosition++];
    normal[1] = _attributes->getVector()[attributePosition++];
    normal[2] = _attributes->getVector()[attributePosition++];

    color.r = _attributes->getVector()[attributePosition++];
    color.g = _attributes->getVector()[attributePosition++];
    color.b = _attributes->getVector()[attributePosition++];
    color.a = 1.0;


    // push 8 normals and 8 colors ...
    _mesh->getNormals().push_back(normal);
    _mesh->getNormals().push_back(normal);
    _mesh->getNormals().push_back(normal);
    _mesh->getNormals().push_back(normal);
    _mesh->getNormals().push_back(normal);
    _mesh->getNormals().push_back(normal);
    _mesh->getNormals().push_back(normal);
    _mesh->getNormals().push_back(normal);

    _mesh->getColors().push_back(color);
    _mesh->getColors().push_back(color);
    _mesh->getColors().push_back(color);
    _mesh->getColors().push_back(color);
    _mesh->getColors().push_back(color);
    _mesh->getColors().push_back(color);
    _mesh->getColors().push_back(color);
    _mesh->getColors().push_back(color);
  }

#define USE_TRIANGLES_FOR_SOLID_CUBE

#ifdef USE_TRIANGLES_FOR_SOLID_CUBE
  _mesh->getTriangles().push_back(gloost::TriangleFace(gIndex,eIndex,aIndex));
  _mesh->getTriangles().push_back(gloost::TriangleFace(gIndex,aIndex,cIndex));
  _mesh->getTriangles().push_back(gloost::TriangleFace(cIndex,aIndex,bIndex));
  _mesh->getTriangles().push_back(gloost::TriangleFace(cIndex,bIndex,dIndex));
  _mesh->getTriangles().push_back(gloost::TriangleFace(aIndex,eIndex,fIndex));
  _mesh->getTriangles().push_back(gloost::TriangleFace(aIndex,fIndex,bIndex));
  _mesh->getTriangles().push_back(gloost::TriangleFace(gIndex,cIndex,dIndex));
  _mesh->getTriangles().push_back(gloost::TriangleFace(gIndex,dIndex,hIndex));
  _mesh->getTriangles().push_back(gloost::TriangleFace(dIndex,bIndex,fIndex));
  _mesh->getTriangles().push_back(gloost::TriangleFace(dIndex,fIndex,hIndex));
  _mesh->getTriangles().push_back(gloost::TriangleFace(hIndex,fIndex,eIndex));
  _mesh->getTriangles().push_back(gloost::TriangleFace(hIndex,eIndex,gIndex));
#else
  _mesh->getQuads().push_back(gloost::QuadFace(eIndex,aIndex,cIndex,gIndex));
  _mesh->getQuads().push_back(gloost::QuadFace(eIndex,fIndex,bIndex,aIndex));
  _mesh->getQuads().push_back(gloost::QuadFace(aIndex,bIndex,dIndex,cIndex));
  _mesh->getQuads().push_back(gloost::QuadFace(cIndex,dIndex,gIndex,hIndex));
  _mesh->getQuads().push_back(gloost::QuadFace(bIndex,fIndex,hIndex,dIndex));
  _mesh->getQuads().push_back(gloost::QuadFace(fIndex,eIndex,gIndex,hIndex));
#endif

}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief draws the svo visualization
  \param ...
  \remarks ...
*/

void
SvoVisualizer::draw()
{
  _vbo->bind();
  _vbo->draw();
  _vbo->disable();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief returns the mesh created by the visualizer
  \param ...
  \remarks ...
*/

gloost::Mesh*
SvoVisualizer::getMesh()
{
  return _mesh;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief rites the visualization as gbm file
  \param ...
  \remarks ...
*/

bool
SvoVisualizer::saveAsGbm(const std::string& filePath)
{
  gloost::GbmWriter gbmWriter;
  return gbmWriter.write(filePath, _mesh);
}


////////////////////////////////////////////////////////////////////////////////





} // namespace svo


