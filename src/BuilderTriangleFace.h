
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



#ifndef H_SVO_BUILDERTRIANGLEFACE
#define H_SVO_BUILDERTRIANGLEFACE



// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>


// cpp includes
#include <string>
#include <memory>


namespace gloost
{
  class Mesh;
  class TriangleFace;
  class BoundingBox;
  class Point3;
}



namespace svo
{


  //  Stores a triangle with its standart attributes

class BuilderTriangleFace
{
	public:

    // class constructor
    BuilderTriangleFace(const std::shared_ptr<gloost::Mesh>& mesh, unsigned triangleIndex);

    // class destructor
	  virtual ~BuilderTriangleFace();


    // returns the center of the triangle
    gloost::Point3 getCenter() const;

    // returns the average normal of the three vertices
    gloost::Vector3 getFaceNormal() const;


    // wrapper for triBoxOverlap(boxcenter, boxhalfsize, triverts)
    bool intersectAABB(const gloost::BoundingBox& aabb) const;

    //
    bool intersectRay( const gloost::Ray& ray,
                       float& u,
                       float& v) const;

    // delivers u and v by shooting a ray along triangles normal through point
    bool calculateUAndV(const gloost::Point3& point,
                        float& u,
                        float& v) const;

    //
    gloost::Vector3 interpolateNormal( gloost::mathType u,
                                       gloost::mathType v) const;

    //
    gloost::Vector3 interpolateColor( gloost::mathType u,
                                      gloost::mathType v) const;
    //
    gloost::Point3 interpolateTexCoord( gloost::mathType u,
                                        gloost::mathType v) const;


    gloost::Point3 _pos0;
    gloost::Point3 _pos1;
    gloost::Point3 _pos2;

    gloost::Vector3 _normal0;
    gloost::Vector3 _normal1;
    gloost::Vector3 _normal2;

    gloost::vec4 _color0;
    gloost::vec4 _color1;
    gloost::vec4 _color2;

    gloost::Point3 _texCoord0;
    gloost::Point3 _texCoord1;
    gloost::Point3 _texCoord2;

    unsigned _materialId;
    unsigned _id;

	private:


};


} // namespace gloost


#endif // H_SVO_BUILDERTRIANGLEFACE


