
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



#ifndef H_SVO_SVO_BUILDER_FACES
#define H_SVO_SVO_BUILDER_FACES



/// svo system includes


/// gloost system includes
#include <gloostConfig.h>
#include <gloostMath.h>
#include <Mesh.h>
#include <Vector3.h>
#include <Point3.h>
#include <UniformTypes.h>
#include <MatrixStack.h>
#include <DrawableCompound.h>


/// cpp includes
#include <string>

namespace svo
{

  class Svo;
  class SvoNode;




  //  generates the svo structure by discretising faces

class SvoBuilderFaces
{
	public:


    /// Stores a triangle with position, normals and colors for faster access

    struct BuilderTriFace
    {
      BuilderTriFace (gloost::Mesh* mesh, gloost::TriangleFace& triangle)
      {
        pos0 = mesh->getVertices()[triangle.vertexIndices[0]];
        pos1 = mesh->getVertices()[triangle.vertexIndices[1]];
        pos2 = mesh->getVertices()[triangle.vertexIndices[2]];

        normal0 = mesh->getNormals()[triangle.vertexIndices[0]];
        normal1 = mesh->getNormals()[triangle.vertexIndices[1]];
        normal2 = mesh->getNormals()[triangle.vertexIndices[2]];

        color0 = mesh->getColors()[triangle.vertexIndices[0]];
        color1 = mesh->getColors()[triangle.vertexIndices[1]];
        color2 = mesh->getColors()[triangle.vertexIndices[2]];
      }

      gloost::Point3 pos0;
      gloost::Point3 pos1;
      gloost::Point3 pos2;

      gloost::Point3 normal0;
      gloost::Point3 normal1;
      gloost::Point3 normal2;

      gloost::vec4 color0;
      gloost::vec4 color1;
      gloost::vec4 color2;
    };


    // class constructor
    SvoBuilderFaces();

    // class destructor
	  virtual ~SvoBuilderFaces();


    // generates the svo by discretising faces
	  Svo* build(Svo* svo, gloost::Mesh* mesh);


//	  gloost::DrawableCompound* _raysDrawable;


	protected:

    // builds the svo recursive from triangle faces
    void buildRecursive(unsigned int           currentDepth,
                        const BuilderTriFace&  triangle);

    // wrapper for triBoxOverlap(boxcenter, boxhalfsize, triverts)
    bool intersectTriangleAABB(const gloost::BoundingBox& aabb,
                               gloost::TriangleFace& triangle);


    // wrapper for triBoxOverlap(boxcenter, boxhalfsize, triverts)
    bool intersectTriangleAABB(const gloost::BoundingBox& aabb,
                               const BuilderTriFace& triangle);

    // tries to intersect the triangle using vertices
    bool findIntersection(const BuilderTriFace& triangle,
                          const gloost::BoundingBox& aabb,
                          float& u,
                          float& v) const;

    //
    bool intersectRayTriangle( const gloost::Ray& ray,
                               const BuilderTriFace& triangle,
                               float& u,
                               float& v) const;

    //
    gloost::Vector3 interpolateNormal( gloost::mathType u,
                                       gloost::mathType v,
                                       const BuilderTriFace& triangle) const;

    //
    gloost::Vector3 interpolateColor( gloost::mathType u,
                                      gloost::mathType v,
                                      const BuilderTriFace& triangle) const;


    Svo*                _svo;
    gloost::Mesh*       _mesh;
    gloost::MatrixStack _matrixStack;

};


} // namespace svo


#endif // H_SVO_SVO_BUILDER_FACES


