
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
#include <SvoBuilderFaces.h>
#include <Svo.h>
#include <trinangleAABBIntersect.h>


/// gloost system includes
#include <TextureManager.h>
#include <MatrixStack.h>


/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class SvoBuilderFaces

  \brief generates the svo structure by discretising faces

  \author Felix Weiszig
  \date   March 2011
  \remarks
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

SvoBuilderFaces::SvoBuilderFaces():
    _svo(0),
    _mesh(0),
    _matrixStack()
//    _raysDrawable(new gloost::DrawableCompound())
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

SvoBuilderFaces::~SvoBuilderFaces()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief builds the svo structure from a mesh using the vertices
  \param mesh gloost::Mesh with unique normal and color for each vertice
  \remarks ...
*/

Svo*
SvoBuilderFaces::build(Svo* svo, gloost::Mesh* mesh)
{

  _svo  = svo;
  _mesh = mesh;


  std::vector<gloost::Point3>&       vertices  = mesh->getVertices();
  std::vector<gloost::Vector3>&      normals   = mesh->getNormals();
  std::vector<gloost::vec4>&         colors    = mesh->getColors();
  std::vector<gloost::TriangleFace>& triangles = mesh->getTriangles();

#ifndef GLOOST_SYSTEM_DISABLE_OUTPUT_MESSAGES
    std::cerr << std::endl;
    std::cerr << std::endl << "Message from SvoBuilderFaces::build(Svo* svo, gloost::Mesh* mesh):";
    std::cerr << std::endl << "             Building Octree from triangle faces:";
    std::cerr << std::endl << "               max depth                  " << svo->getMaxDepth();
    std::cerr << std::endl << "               min voxelsize              " << pow(2, -(int)_svo->getMaxDepth());
    std::cerr << std::endl << "               triangles.size():          " << triangles.size();
#endif



  // for seperate component data
  for (unsigned int i=0; i!=triangles.size(); ++i)
  {
    const BuilderTriFace triFace(_mesh, triangles[i]);
    if (intersectTriangleAABB(svo->getBoundingBox(), triFace))
    {
      buildRecursive(0, triFace);
    }
  }


  svo->normalizeAttribs();
  svo->generateInnerNodesAttributes(svo->getRootNode());


#ifndef GLOOST_SYSTEM_DISABLE_OUTPUT_MESSAGES
  std::cerr << std::endl << "               Number of leaves:          " << _svo->getNumLeaves();
  std::cerr << std::endl << "               Number of nodes:           " << _svo->getNumNodes();
  std::cerr << std::endl << "               Number of OOB Points:      " << _svo->getNumOutOfBoundPoints();
  std::cerr << std::endl << "               Number of double Points:   " << _svo->getNumDoublePoints();
  std::cerr << std::endl << "               Octree memory real CPU:    " << _svo->getNumNodes()*sizeof(svo::SvoNode)/1024.0/1024.0 << " MB";
  std::cerr << std::endl;
  std::cerr << std::endl << "             Creating attributes for inner nodes: ";
  std::cerr << std::endl << "               Octree memory serialized:  " << _svo->getNumNodes()*svo::SvoNode::getSerializedNodeSize()/1024.0/1024.0 << " MB";
  std::cerr << std::endl << "               Attribs memory serialized: " << _svo->getCurrentAttribPosition()*sizeof(float)/1024.0/1024.0 << " MB";
  std::cerr << std::endl << "               Number of one-child-nodes: " << _svo->getNumOneChildNodes() << " ( " << (100.0f*_svo->getNumOneChildNodes())/(float)_svo->getNumNodes() << " % )";
  std::cerr << std::endl;
#endif


}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief builds the svo recursive from triangle faces
  \param ...
  \remarks ...
*/

void
SvoBuilderFaces::buildRecursive(unsigned int           currentDepth,
                                const BuilderTriFace&  triangle)
{

  // max depth reached
  if ( currentDepth  == _svo->getMaxDepth())
  {

    gloost::BoundingBox bbox(gloost::Point3(-0.5,-0.5,-0.5),
                             gloost::Point3( 0.5, 0.5, 0.5));

    bbox.transform(_matrixStack.top());

    gloost::Point3 voxelCenter = _matrixStack.top() * gloost::Point3(0.0,0.0,0.0);


    // validate voxel by fireing a ray in the direction of the triangles normal throu the center
    // of the AABB. Only if it hits the triangle (and get a normal and a color)
    // the voxel will be inserted
    float u   = 0.0;
    float v   = 0.0;

    SvoNode* leafNode = 0;

    if (findIntersection( triangle, bbox, u, v))
    {
      leafNode = _svo->insert(voxelCenter);
    }


    if (leafNode)
    {
      gloost::Vector3 normal = interpolateNormal(u, v, triangle);
      gloost::Vector3 color  = interpolateColor(u, v, triangle);

      // push attribs for a new leaf node
      if (leafNode->getAttribPosition() == 0)
      {
        leafNode->setAttribPosition(_svo->getCurrentAttribPosition());

        _svo->pushNormalizer();

        _svo->pushAttributeComponent(normal[0]);
        _svo->pushAttributeComponent(normal[1]);
        _svo->pushAttributeComponent(normal[2]);

        _svo->pushAttributeComponent(color[0]);
        _svo->pushAttributeComponent(color[1]);
        _svo->pushAttributeComponent(color[2]);
      }
      // accumulate attribs for an existing node
      else
      {
        unsigned attribPos = leafNode->getAttribPosition();

        _svo->addDoubleNodeToNormalizer(attribPos/6);

        std::vector<float>& attribs = _svo->getAttributeBuffer();

        attribs[attribPos++] += normal[0];
        attribs[attribPos++] += normal[1];
        attribs[attribPos++] += normal[2];

        attribs[attribPos++] += color[0];
        attribs[attribPos++] += color[1];
        attribs[attribPos++] += color[2];
      }
    }
    return;
  }


  const float offset = 0.25;

  for (int z=0; z!=2; ++z)
  {
    for (int y=0; y!=2; ++y)
    {
      for (int x=0; x!=2; ++x)
      {
        int choosenChildIndex = 4*x+2*y+z;

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

          if (intersectTriangleAABB(bbox, triangle))
          {
            buildRecursive(currentDepth+1,
                           triangle);
          }
        }
        _matrixStack.pop();

      }
    }
  }

}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief wrapper for triBoxOverlap(boxcenter, boxhalfsize, triverts)
  \param ...
  \remarks ...
*/

bool
SvoBuilderFaces::intersectTriangleAABB(const gloost::BoundingBox& aabb,
                                       gloost::TriangleFace& triangle)
{
  gloost::Point3 p0 = _mesh->getVertices()[triangle.vertexIndices[0]];
  gloost::Point3 p1 = _mesh->getVertices()[triangle.vertexIndices[1]];
  gloost::Point3 p2 = _mesh->getVertices()[triangle.vertexIndices[2]];


  gloost::Point3 bbCenter = aabb.getCenter();
  float boxcenter[3] = {bbCenter[0],bbCenter[1],bbCenter[2]};

  gloost::Vector3 bbHalfSize = aabb.getHalfSize();
  float boxhalfsize[3] = {/*epsilon +*/ bbHalfSize[0], /*epsilon +*/ bbHalfSize[1], /*epsilon +*/ bbHalfSize[2]};

  float triverts[3][3] = {	{p0[0], p0[1],p0[2]},
                            {p1[0], p1[1], p1[2]},
                            {p2[0], p2[1], p2[2]}};

  return (bool) triBoxOverlap(boxcenter, boxhalfsize, triverts);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief wrapper for triBoxOverlap(boxcenter, boxhalfsize, triverts)
  \param ...
  \remarks ...
*/

bool
SvoBuilderFaces::intersectTriangleAABB(const gloost::BoundingBox& aabb,
                                       const BuilderTriFace& triangle)
{
  gloost::Point3 bbCenter = aabb.getCenter();
  float boxcenter[3] = {bbCenter[0],bbCenter[1],bbCenter[2]};

  gloost::Vector3 bbHalfSize = aabb.getHalfSize();
  float boxhalfsize[3] = {/*epsilon +*/ bbHalfSize[0], /*epsilon +*/ bbHalfSize[1], /*epsilon +*/ bbHalfSize[2]};

  float triverts[3][3] = {	{triangle.pos0[0], triangle.pos0[1],triangle.pos0[2]},
                            {triangle.pos1[0], triangle.pos1[1], triangle.pos1[2]},
                            {triangle.pos2[0], triangle.pos2[1], triangle.pos2[2]}};

  return (bool) triBoxOverlap(boxcenter, boxhalfsize, triverts);
}


///////////////////////////////////////////////////////////////////////////////


/**
  \brief tries to intersect the triangle using vertices
  \param ...
  \remarks ...
*/

//
bool
SvoBuilderFaces::findIntersection(const BuilderTriFace& triangle,
                                  const gloost::BoundingBox& aabb,
                                  float& u,
                                  float& v) const
{
  gloost::Point3  voxelCenter    = aabb.getCenter();
  gloost::Vector3 avNormal       = (triangle.normal0 + triangle.normal1 + triangle.normal2)/3.0;
  avNormal.normalize();

  gloost::Point3  triangleCenter = (triangle.pos0 + triangle.pos1 + triangle.pos2)/3.0;

  // test center
  gloost::Ray ray(voxelCenter, avNormal, 1.0);

  return intersectRayTriangle(ray, triangle, u, v);
}


////////////////////////////////////////////////////////////////////////////////

bool
SvoBuilderFaces::intersectRayTriangle( const gloost::Ray& ray,
                                       const BuilderTriFace& triangle,
                                       float& u,
                                       float& v) const
{
  gloost::mathType EPSILON = 0.000000000001;

  gloost::Vector3 edge1, edge2, tvec, pvec, qvec;
  gloost::mathType det, invDet, t;

  // nur reference
  const gloost::Point3& v0 = triangle.pos0;
  const gloost::Point3& v1 = triangle.pos1;
  const gloost::Point3& v2 = triangle.pos2;

  // find vectors for two edges sharing v[0]
  edge1 = v1 - v0;
  edge2 = v2 - v0;

  // begin calculating determinant - also used to calculate U parameter
  pvec = cross(ray.getDirection(), edge2);

  // if determinant is near zero, ray lies in plane of triangle
  det = edge1 * pvec;

  if(det > -EPSILON && det < EPSILON)
  {
    return false;
  }

  invDet = 1.0 / det;

  // calculate distance from v[0] to ray origin
  tvec = ray.getOrigin() - v0;

  // calculate U parameter and test bounds
  u = (tvec * pvec) * invDet;

//  if(u < 0.0 || u > 1.0)
//  {
//    return false;
//  }

  // prepare tor test V parameter
  qvec = cross(tvec, edge1);

  // calculate V parameter and test bounds
  v = (ray.getDirection() * qvec) * invDet;

//  if (v < 0.0 || u + v > 1.0)
//  {
//    return false;
//  }

  u = gloost::clamp(u, 0.0f, 1.0f);
  v = gloost::clamp(v, 0.0f, 1.0f);

  // calculate t, scale parameters, ray intersects triangle
//  t = (edge2 * qvec) * invDet;


//  if(t<0.0)
//  {
//    ray.triId = triId;
//    ray.t = t;
//    ray.u = u;
//    ray.v = v;
//    std::cerr << std::endl << "t: " << ;
//  }

    return true;
//  return false;
}

///////////////////////////////////////////////////////////////////////////////


gloost::Vector3
SvoBuilderFaces::interpolateNormal(gloost::mathType u,
                                   gloost::mathType v,
                                   const BuilderTriFace& triangle) const
{
//  gloost::mathType w = 1.0 - u - v;
  gloost::Vector3 interpolNormal = triangle.normal0*(1.0 - u - v) + triangle.normal1*u + triangle.normal2*v;
  interpolNormal.normalize();
  return interpolNormal;
}


///////////////////////////////////////////////////////////////////////////////


gloost::Vector3
SvoBuilderFaces::interpolateColor(gloost::mathType u,
                                  gloost::mathType v,
                                  const BuilderTriFace& triangle) const
{
  gloost::Vector3 color0(triangle.color0.r, triangle.color0.g, triangle.color0.b);
  gloost::Vector3 color1(triangle.color1.r, triangle.color1.g, triangle.color1.b);
  gloost::Vector3 color2(triangle.color2.r, triangle.color2.g, triangle.color2.b);

//  gloost::mathType w = 1.0 - u - v;
  gloost::Vector3 interpolColor = color0*(1.0 - u - v) + color1*u + color2*v;
  return interpolColor;
}


///////////////////////////////////////////////////////////////////////////////









} // namespace svo


