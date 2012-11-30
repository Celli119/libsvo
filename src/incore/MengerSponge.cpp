
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
#include <MengerSponge.h>

/// gloost system includes
#include <Mesh.h>
#include <Vbo.h>
#include <GbmWriter.h>


/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class MengerSponge

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

MengerSponge::MengerSponge(unsigned int maxDepth):
  _maxDepth(maxDepth),
  _mesh(new gloost::Mesh()),
  _vbo(new gloost::Vbo(_mesh)),
  _vertexIndex(0)
{
	_mesh->takeReference();

  _mesh->setSupportedInterleavedAttribute(GLOOST_MESH_POSITIONS, true);
  _mesh->setSupportedInterleavedAttribute(GLOOST_MESH_NORMALS,   true);


	std::vector<gloost::Point3> points = std::vector<gloost::Point3>(1, gloost::Point3(0.0,0.0,0.0));

  float offset = 2.0/6.0;

  for (int depth=1; depth!=maxDepth; ++depth)
  {
     std::vector<gloost::Point3> morePoints;

     float scale = pow(3, -depth+1);

     for (unsigned int p=0; p!=points.size(); ++p)
     {
       for (int z=-1; z!=2; ++z)
       {
         for (int y=-1; y!=2; ++y)
         {
           for (int x=-1; x!=2; ++x)
           {
             if (!((x == 0 && y == 0 && z == -1) | (x == 0 && y == 0 && z == 1) |
                   (x == 0 && z == 0 && y == -1) | (x == 0 && z == 0 && y == 1) |
                   (z == 0 && y == 0 && x == -1) | (z == 0 && y == 0 && x == 1) |
                   (z == 0 && y == 0 && x == 0) ))
             {
               morePoints.push_back(points[p] + gloost::Point3(x*offset*scale,
                                                               y*offset*scale,
                                                               z*offset*scale));
             }
           }
         }
       }
     }


     points.clear();

     points = morePoints;
     morePoints.clear();
  }

  std::cerr << std::endl << "points.size(): " << points.size();


  float scale = pow(3, -(int)maxDepth+1);



  for (unsigned int i=0; i<points.size(); ++i)
  {
    pushSolidCubeToMesh(points[i], scale);
  }

  _vbo->setDrawMode(GL_QUADS);



}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

MengerSponge::~MengerSponge()
{
	_mesh->dropReference();
	_vbo->dropReference();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief adds a solid cube to the mesh
  \param ...
  \remarks ...
*/

void
MengerSponge::pushSolidCubeToMesh(const gloost::Point3& center,
                                  float scale)
{



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

  // +x
  {
    gloost::Point3 x0 = gloost::Point3( 0.5, -0.5,  0.5) * scale + center;
    gloost::Point3 x1 = gloost::Point3( 0.5, -0.5, -0.5) * scale + center;
    gloost::Point3 x2 = gloost::Point3( 0.5,  0.5, -0.5) * scale + center;
    gloost::Point3 x3 = gloost::Point3( 0.5,  0.5,  0.5) * scale + center;

    _mesh->pushInterleavedComponent(x0);
    _mesh->pushInterleavedComponent(gloost::Vector3(1.0, 0.0, 0.0));
    _mesh->pushInterleavedComponent(x1);
    _mesh->pushInterleavedComponent(gloost::Vector3(1.0, 0.0, 0.0));
    _mesh->pushInterleavedComponent(x2);
    _mesh->pushInterleavedComponent(gloost::Vector3(1.0, 0.0, 0.0));
    _mesh->pushInterleavedComponent(x3);
    _mesh->pushInterleavedComponent(gloost::Vector3(1.0, 0.0, 0.0));

    _mesh->getQuads().push_back(gloost::QuadFace(_vertexIndex++,
                                                 _vertexIndex++,
                                                 _vertexIndex++,
                                                 _vertexIndex++));
  }


  // -x
  {
    gloost::Point3 x0 = gloost::Point3( -0.5, -0.5,  0.5) * scale + center;
    gloost::Point3 x1 = gloost::Point3( -0.5, -0.5, -0.5) * scale + center;
    gloost::Point3 x2 = gloost::Point3( -0.5,  0.5, -0.5) * scale + center;
    gloost::Point3 x3 = gloost::Point3( -0.5,  0.5,  0.5) * scale + center;

    _mesh->pushInterleavedComponent(x0);
    _mesh->pushInterleavedComponent(gloost::Vector3(-1.0, 0.0, 0.0));
    _mesh->pushInterleavedComponent(x1);
    _mesh->pushInterleavedComponent(gloost::Vector3(-1.0, 0.0, 0.0));
    _mesh->pushInterleavedComponent(x2);
    _mesh->pushInterleavedComponent(gloost::Vector3(-1.0, 0.0, 0.0));
    _mesh->pushInterleavedComponent(x3);
    _mesh->pushInterleavedComponent(gloost::Vector3(-1.0, 0.0, 0.0));

    _mesh->getQuads().push_back(gloost::QuadFace(_vertexIndex++, _vertexIndex++, _vertexIndex++, _vertexIndex++));
  }


  // +y
  {
    gloost::Point3 y0 = gloost::Point3( -0.5,  0.5,  0.5) * scale + center;
    gloost::Point3 y1 = gloost::Point3( -0.5,  0.5, -0.5) * scale + center;
    gloost::Point3 y2 = gloost::Point3(  0.5,  0.5, -0.5) * scale + center;
    gloost::Point3 y3 = gloost::Point3(  0.5,  0.5,  0.5) * scale + center;

    _mesh->pushInterleavedComponent(y0);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, 1.0, 0.0));
    _mesh->pushInterleavedComponent(y1);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, 1.0, 0.0));
    _mesh->pushInterleavedComponent(y2);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, 1.0, 0.0));
    _mesh->pushInterleavedComponent(y3);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, 1.0, 0.0));

    _mesh->getQuads().push_back(gloost::QuadFace(_vertexIndex++, _vertexIndex++, _vertexIndex++, _vertexIndex++));
  }

  // -y
  {
    gloost::Point3 y0 = gloost::Point3( -0.5, -0.5,  0.5) * scale + center;
    gloost::Point3 y1 = gloost::Point3( -0.5, -0.5, -0.5) * scale + center;
    gloost::Point3 y2 = gloost::Point3(  0.5, -0.5, -0.5) * scale + center;
    gloost::Point3 y3 = gloost::Point3(  0.5, -0.5,  0.5) * scale + center;

    _mesh->pushInterleavedComponent(y0);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, -1.0, 0.0));
    _mesh->pushInterleavedComponent(y1);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, -1.0, 0.0));
    _mesh->pushInterleavedComponent(y2);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, -1.0, 0.0));
    _mesh->pushInterleavedComponent(y3);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, -1.0, 0.0));

    _mesh->getQuads().push_back(gloost::QuadFace(_vertexIndex++, _vertexIndex++, _vertexIndex++, _vertexIndex++));
  }

  // +z
  {
    gloost::Point3 z0 = gloost::Point3( -0.5,  0.5,  0.5) * scale + center;
    gloost::Point3 z1 = gloost::Point3( -0.5, -0.5,  0.5) * scale + center;
    gloost::Point3 z2 = gloost::Point3(  0.5, -0.5,  0.5) * scale + center;
    gloost::Point3 z3 = gloost::Point3(  0.5,  0.5,  0.5) * scale + center;

    _mesh->pushInterleavedComponent(z0);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, 0.0, 1.0));
    _mesh->pushInterleavedComponent(z1);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, 0.0, 1.0));
    _mesh->pushInterleavedComponent(z2);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, 0.0, 1.0));
    _mesh->pushInterleavedComponent(z3);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, 0.0, 1.0));

    _mesh->getQuads().push_back(gloost::QuadFace(_vertexIndex++, _vertexIndex++, _vertexIndex++, _vertexIndex++));
  }


  // -z
  {
    gloost::Point3 z0 = gloost::Point3( -0.5,  0.5,  -0.5) * scale + center;
    gloost::Point3 z1 = gloost::Point3( -0.5, -0.5,  -0.5) * scale + center;
    gloost::Point3 z2 = gloost::Point3(  0.5, -0.5,  -0.5) * scale + center;
    gloost::Point3 z3 = gloost::Point3(  0.5,  0.5,  -0.5) * scale + center;

    _mesh->pushInterleavedComponent(z0);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, 0.0, -1.0));
    _mesh->pushInterleavedComponent(z1);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, 0.0, -1.0));
    _mesh->pushInterleavedComponent(z2);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, 0.0, -1.0));
    _mesh->pushInterleavedComponent(z3);
    _mesh->pushInterleavedComponent(gloost::Vector3(0.0, 0.0, -1.0));

    _mesh->getQuads().push_back(gloost::QuadFace(_vertexIndex++, _vertexIndex++, _vertexIndex++, _vertexIndex++));
  }
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief draws the svo visualization
  \param ...
  \remarks ...
*/

void
MengerSponge::draw()
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
MengerSponge::getMesh()
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
MengerSponge::saveAsGbm(const std::string& filePath)
{
  gloost::GbmWriter gbmWriter;
  gbmWriter.write(filePath, _mesh);
}


////////////////////////////////////////////////////////////////////////////////





} // namespace svo


