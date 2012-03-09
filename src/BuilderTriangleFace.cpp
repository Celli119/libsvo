
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


/// gloost system includes
#include <Mesh.h>
#include <BoundingBox.h>


/// svo includes
#include <BuilderTriangleFace.h>
#include <trinangleAABBIntersect.h>


/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   BuilderTriangleFace

  \brief   Stores a triangle with its standart attributes

  \author  Felix Weiszig
  \date    Feb 2012
  \remarks Lazy mans way to create a new class ;-)
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

BuilderTriangleFace::BuilderTriangleFace(gloost::Mesh* mesh, unsigned triangleIndex):
    _pos0(),
    _pos1(),
    _pos2(),
    _normal0(),
    _normal1(),
    _normal2(),
    _color0(),
    _color1(),
    _color2(),
    _texCoord0(),
    _texCoord1(),
    _texCoord2(),
    _materialId(0),
    _id(triangleIndex)

{
  gloost::TriangleFace& triangle = mesh->getTriangles()[_id];

  _pos0 = mesh->getVertices()[triangle.vertexIndices[0]];
  _pos1 = mesh->getVertices()[triangle.vertexIndices[1]];
  _pos2 = mesh->getVertices()[triangle.vertexIndices[2]];

  if (mesh->getNormals().size())
  {
    _normal0 = mesh->getNormals()[triangle.vertexIndices[0]];
    _normal1 = mesh->getNormals()[triangle.vertexIndices[1]];
    _normal2 = mesh->getNormals()[triangle.vertexIndices[2]];
  }

  if (mesh->getColors().size())
  {
    _color0 = mesh->getColors()[triangle.vertexIndices[0]];
    _color1 = mesh->getColors()[triangle.vertexIndices[1]];
    _color2 = mesh->getColors()[triangle.vertexIndices[2]];
  }

  if (mesh->getTexCoords().size())
  {
    _texCoord0 = mesh->getTexCoords()[triangle.vertexIndices[0]];
    _texCoord1 = mesh->getTexCoords()[triangle.vertexIndices[1]];
    _texCoord2 = mesh->getTexCoords()[triangle.vertexIndices[2]];
  }

  if (mesh->getMaterialIds().size())
  {
    _materialId = mesh->getMaterialIds()[_id];
  }

}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

BuilderTriangleFace::~BuilderTriangleFace()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief wrapper for triBoxOverlap(boxcenter, boxhalfsize, triverts)
  \param ...
  \remarks ...
*/

//bool
//BuilderTriangleFace::intersectTriangleAABB(const gloost::BoundingBox& aabb,
//                                       gloost::TriangleFace& triangle)
//{
//  gloost::Point3 p0 = _mesh->getVertices()[triangle.vertexIndices[0]];
//  gloost::Point3 p1 = _mesh->getVertices()[triangle.vertexIndices[1]];
//  gloost::Point3 p2 = _mesh->getVertices()[triangle.vertexIndices[2]];
//
//
//  gloost::Point3 bbCenter = aabb.getCenter();
//  float boxcenter[3] = {bbCenter[0],bbCenter[1],bbCenter[2]};
//
//  gloost::Vector3 bbHalfSize = aabb.getHalfSize();
//  float boxhalfsize[3] = {/*epsilon +*/ bbHalfSize[0], /*epsilon +*/ bbHalfSize[1], /*epsilon +*/ bbHalfSize[2]};
//
//  float triverts[3][3] = {	{p0[0], p0[1],p0[2]},
//                            {p1[0], p1[1], p1[2]},
//                            {p2[0], p2[1], p2[2]}};
//
//  return (bool) triBoxOverlap(boxcenter, boxhalfsize, triverts);
//}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief wrapper for triBoxOverlap(boxcenter, boxhalfsize, triverts)
  \param ...
  \remarks ...
*/

bool
BuilderTriangleFace::intersectAABB(const gloost::BoundingBox& aabb) const
{
  gloost::Point3 bbCenter = aabb.getCenter();
  float boxcenter[3] = {bbCenter[0],bbCenter[1],bbCenter[2]};

  gloost::Vector3 bbHalfSize = aabb.getHalfSize();
  float boxhalfsize[3] = {/*epsilon +*/ bbHalfSize[0], /*epsilon +*/ bbHalfSize[1], /*epsilon +*/ bbHalfSize[2]};

  float triverts[3][3] = {	{_pos0[0], _pos0[1],_pos0[2]},
                            {_pos1[0], _pos1[1], _pos1[2]},
                            {_pos2[0], _pos2[1], _pos2[2]}};

  return (bool) triBoxOverlap(boxcenter, boxhalfsize, triverts);
}


///////////////////////////////////////////////////////////////////////////////


/**
  \brief delivers u and v by shooting a ray along triangles normal through point
  \param ...
  \remarks ...
*/

//
bool
BuilderTriangleFace::calculateUAndV(const gloost::Point3& point,
                                    float& u,
                                    float& v) const
{
  gloost::Vector3 avNormal = (_normal0 + _normal1 + _normal2)/3.0;
  avNormal.normalize();

  // test center
  gloost::Ray ray(point, avNormal, 1.0);

  return intersectRay(ray, u, v);
}


////////////////////////////////////////////////////////////////////////////////

bool
BuilderTriangleFace::intersectRay( const gloost::Ray& ray,
                                   float& u,
                                   float& v) const
{
  gloost::mathType EPSILON = 0.000000000001;

  gloost::Vector3 edge1, edge2, tvec, pvec, qvec;
  gloost::mathType det, invDet, t;

  // nur reference
  const gloost::Point3& v0 = _pos0;
  const gloost::Point3& v1 = _pos1;
  const gloost::Point3& v2 = _pos2;

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
BuilderTriangleFace::interpolateNormal(gloost::mathType u,
                                       gloost::mathType v) const
{
//  gloost::mathType w = 1.0 - u - v;
  gloost::Vector3 interpolNormal = _normal0*(1.0 - u - v) + _normal1*u + _normal2*v;
  interpolNormal.normalize();
  return interpolNormal;
}


///////////////////////////////////////////////////////////////////////////////


gloost::Vector3
BuilderTriangleFace::interpolateColor(gloost::mathType u,
                                      gloost::mathType v) const
{
  gloost::Vector3 color0(_color0.r, _color0.g, _color0.b);
  gloost::Vector3 color1(_color1.r, _color1.g, _color1.b);
  gloost::Vector3 color2(_color2.r, _color2.g, _color2.b);

//  gloost::mathType w = 1.0 - u - v;
  gloost::Vector3 interpolColor = color0*(1.0 - u - v) + color1*u + color2*v;
  return interpolColor;
}



////////////////////////////////////////////////////////////////////////////////





} // namespace svo

