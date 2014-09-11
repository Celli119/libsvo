
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



// gloost system includes
#include <gloost/Mesh.h>
#include <gloost/Image.h>
#include <gloost/PerspectiveCamera.h>
#include <gloost/BinaryBundle.h>
#include <gloost/BinaryFile.h>


#include <ImageToPointcloud.h>

// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   ImageToPointcloud

  \brief   converts an depth image to a point cloud

  \author  Felix Weiszig
  \date    March 2011
  \remarks ...
*/

//////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

ImageToPointcloud::ImageToPointcloud()
{
	// insert your code here
}


//////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

ImageToPointcloud::~ImageToPointcloud()
{
	// insert your code here
}


//////////////////////////////////////////////////////


/**
  \brief   generates a mesh from a depth image and a color image
  \param   ...
  \remarks ...
*/

/* static*/
std::shared_ptr<gloost::Mesh>
ImageToPointcloud::generate(gloost::mathType      fieldOfView,
                            const gloost::Matrix& viewMatrix,
                            const std::string& depthImagePath,
                            const std::string& normalImagePath,
                            const std::string& colorImagePath)
{

  // generate svo from images
  gloost::Image image_depth(depthImagePath);
  gloost::Image image_normal(normalImagePath);
  gloost::Image image_color(colorImagePath);
  unsigned numPixels = image_depth.getWidth()*image_depth.getHeight();

  // camera with parameters from lightwave
  gloost::Point3 standardLookAt(0.0,0.0,-1.0);
  gloost::PerspectiveCamera lightwaveCamera(fieldOfView,
                                            (float)image_depth.getWidth()/(float)image_depth.getHeight(),
                                            0.1, 1000);

  gloost::Point3 cameraPosition = viewMatrix * gloost::Point3(0.0,0.0,0.0);

  gloost::Point3 cameraLookAt = viewMatrix * standardLookAt;

  std::cerr << std::endl << "cameraPosition: "  << cameraPosition;
  std::cerr << std::endl << "cameraLookAt: " << cameraLookAt;

  lightwaveCamera.lookAt(gloost::Point3(cameraPosition),
                         gloost::Point3(cameraLookAt),
                         gloost::Vector3(0.0,1.0,0.0));


  gloost::Vector3 cameraDirection = gloost::Vector3(cameraPosition-cameraLookAt).normalized();

//  lightwaveCamera.setViewMatrix(viewMatrix);


  // mesh to store point indices
  auto pointMesh = gloost::Mesh::create();
//  pointMesh->takeReference();
  pointMesh->getPoints().resize(numPixels);
  pointMesh->getVertices().resize(numPixels);
  pointMesh->getColors().resize(numPixels);
  pointMesh->getNormals().resize(numPixels);

  // Bundle storing 96 bit image data
  gloost::BinaryBundle::shared_ptr imageData_depth = gloost::BinaryBundle::create(image_depth.getPixelData(),
                                                                                   numPixels*image_depth.getBitsPerPixel()/8);

  gloost::BinaryBundle::shared_ptr imageData_normal = gloost::BinaryBundle::create(image_normal.getPixelData(),
                                                                                   numPixels*image_normal.getBitsPerPixel()/8);

  gloost::BinaryBundle::shared_ptr imageData_color = gloost::BinaryBundle::create(image_color.getPixelData(),
                                                                                  numPixels*image_depth.getBitsPerPixel()/8);

  // file reader to access the bundle
  gloost::BinaryFile imageReader_depth;
  imageReader_depth.openAndLoad(imageData_depth);

  gloost::BinaryFile imageReader_normal;
  imageReader_normal.openAndLoad(imageData_normal);

  gloost::BinaryFile imageReader_color;
  imageReader_color.openAndLoad(imageData_color);

  unsigned pointIndex = 0;
  for (unsigned y=0; y!=image_depth.getHeight(); ++y)
    for (unsigned x=0; x!=image_depth.getWidth(); ++x)
    {
      unsigned indexInBytes = (y*image_depth.getWidth()+x)*12;

      // depth value of lightwave is given as distance from a view plane, not from the view point
      // (depth rays are orthogonal)
      imageReader_depth.setCurrentReadPosition(indexInBytes);
      float lightwaveDepth = imageReader_depth.readFloat32();

//      std::cerr << std::endl << "depth: " << d;

//      std::cerr << std::endl << "b: " << b;

      // determine pixels position in 3d
      gloost::Ray pickRay = lightwaveCamera.getPickRay(image_depth.getWidth(),
                                                       image_depth.getHeight(),
                                                       x,
                                                       image_depth.getHeight()-y);
      pickRay.normalize();

      // calculate the real depth from camera point
      // with:         cos(alpha) = dot(viewDirection, pixelInWorldSpaceDirection)
      // and           realDepth  = lightWaveDepth / cos(alpha)
      // resulting in: depthFromCamera = lightwaveDepth /  dot(viewDirection, pixelInWorldSpaceDirection)
      gloost::mathType depthFromCamera = lightwaveDepth/(cameraDirection*pickRay.getDirection());

      pickRay.setT(depthFromCamera);

      gloost::Point3 samplePosition = pickRay.getDestination();
      samplePosition[1] *= -1;


      imageReader_color.setCurrentReadPosition(indexInBytes);
      float r = imageReader_color.readFloat32();
      float g = imageReader_color.readFloat32();
      float b = imageReader_color.readFloat32();

      imageReader_normal.setCurrentReadPosition(indexInBytes);
      float nx = imageReader_normal.readFloat32();
      float ny = imageReader_normal.readFloat32();
      float nz = imageReader_normal.readFloat32();


      pointMesh->getVertices()[pointIndex] = samplePosition;
      pointMesh->getColors()[pointIndex]   = gloost::vec4(r,g,b,1.0);
      pointMesh->getNormals()[pointIndex]  = gloost::Vector3(nx, -ny, nz);
      pointMesh->getPoints()[pointIndex]   = pointIndex;

//      std::cerr << std::endl << "word: " << pointMesh->getVertices()[pointIndex];

      ++pointIndex;
    }


  pointMesh->recalcBoundingBox();

  return pointMesh;
}


//////////////////////////////////////////////////////





} // namespace svo


