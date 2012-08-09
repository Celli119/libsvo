/**

  Last change 30.04.24 by Felix Weißig

  Email: thesleeper@gmx.net
  skype: otacocato
  URL:   http://www.otaco.de

**/

/**
  Step 00: glut template

  This is a simple glfw template we will use in the tutorials.

**/


////////////////////////////////////////////////////////////////////////////////



/// for glut win32
#define _STDCALL_SUPPORTED
#define _M_IX86



/// c++ includes
#include <GL/glew.h>
#include <GL/glfw.h>
#include <iostream>
#include <cmath>
#include <stdlib.h>


////////////////////////////////////////////////////////////////////////////////


/// general setup
static unsigned int g_screenWidth  = 1024;
static unsigned int g_screenHeight = 1024;

static unsigned int g_bufferWidth  = g_screenWidth;
static unsigned int g_bufferHeight = g_screenHeight;

/// frameCounter
unsigned int g_frameCounter = 0;


/// includes and globals for this demo /////////////////////////////////////////

#include <gloost/gloostRenderGoodies.h>


#include <gloost/TextureText.h>
gloost::TextureText* g_texter = 0;
double g_timePerFrame = 0;

std::string g_gloostFolder = "../../gloost/";
std::string g_dataPath     = "../data/";

#include <gloost/Matrix.h>
#include <gloost/Mouse.h>
gloost::Mouse g_mouse;

#include <gloost/PerspectiveCamera.h>
gloost::PerspectiveCamera*   g_camera;


gloost::Point3 g_modelOffset;
float          g_cameraRotateY  = 0;
float          g_cameraRotateX  = 0;
float          g_cameraDistance = 1.0;


gloost::Ray g_ray;
#include <gloost/GbmLoader.h>
#include <gloost/PlyLoader.h>
#include <gloost/Mesh.h>
#include <gloost/Vbo.h>
gloost::Vbo* g_drawRayVbo  = 0;
gloost::Vbo* g_drawAxisVbo = 0;

// SVO
#include <Svo.h>
svo::Svo* g_svo = 0;
#include <SvoNode.h>

#include <SvoVisualizer.h>
svo::SvoVisualizer* g_svoVisualizerNodes     = 0;
svo::SvoVisualizer* g_svoVisualizerLeaves    = 0;
#include <CpuRaycasterSingleRay.h>
#include <CpuRaycasterSingleRay2.h>
#include <CpuRaycasterSingleRay3.h>
int g_maxRayCastDepth = 1;


#include <SvoBuilderVertices.h>
#include <SvoBuilderFaces.h>
#include <attribute_generators/Ag_colorAndNormalsTriangles.h>
#include <gloost/InterleavedAttributes.h>
gloost::InterleavedAttributes* g_voxelAttributes = 0;

#include <gloost/Shader.h>
#include <gloost/UniformSet.h>
gloost::Shader*     g_SvoVisualizingShader = 0;


// setup for rendering into frame buffer
#include <gloost/TextureManager.h>
#include <gloost/Texture.h>

unsigned g_framebufferTextureId = 0;
#include <gloost/gloostHelper.h>


#include <boost/thread.hpp>


// info
bool g_showTextInfo     = true;
bool g_showOctreeNodes  = true;
bool g_showOctreeLeaves = false;
bool g_showRayCastImage = true;

void init();
void buildSvo(gloost::Mesh* mesh, unsigned int maxSvoDepth);
void buildSvoVisualization();
void draw3d(void);
void draw2d();
void cleanup();
void mouseFunc(int button, int state, int mouse_h, int mouse_v);
void reloadShaders();
void raycastIntoFrameBuffer( unsigned startIndex,
                             unsigned count,
                             unsigned threadId);
void resize(int width, int height);
void key(unsigned char key, int x, int y);
void motionFunc(int mouse_h, int mouse_v);
void idle(void);


gloost::Mesh* g_testmesh = 0;
gloost::Vbo*  g_testVbo  = 0;


std::vector<gloost::Vector2> g_screenCoords;


float* g_renderBuffer = 0;

boost::thread_group g_threadGroup;
bool  g_toggle_run_raycasting = true;
boost::mutex g_bufferAccessMutex;


bool     g_toggle_renderer = false;
unsigned g_num_building_Threads = 8;
unsigned g_num_render_Threads   = 8;



////////////////////////////////////////////////////////////////////////////////



void init()
{

#define OPEN_GL_WINDOW
//#define DRAW_MESH

#define BUILD_SVO
//#define BUILD_VISUALIZATION_NODES
//#define BUILD_VISUALIZATION_LEAVES

#define USE_THREADED_RENDERING
  g_num_render_Threads = 8;

  g_bufferWidth  = g_screenWidth/8;
  g_bufferHeight = g_screenHeight/8;

  const unsigned maxDepth = 6;


  // create screencoords
  g_screenCoords = std::vector<gloost::Vector2>(g_bufferWidth*g_bufferHeight);

  for (unsigned y=0; y!=g_bufferHeight; ++y)
    for (unsigned x=0; x!=g_bufferWidth; ++x)
    {
      unsigned index = y*g_bufferWidth +x;
      g_screenCoords[index] = gloost::Vector2(x,y);
    }

  // mix screen coords
//  for (unsigned i=0; i!=g_screenCoords.size()*12; ++i)
//  {
//    unsigned index1 = (unsigned)(gloost::frand()*g_screenCoords.size());
//    unsigned index2 = (unsigned)(gloost::frand()*g_screenCoords.size());
//
//    gloost::Vector2 tmp    = g_screenCoords[index1];
//    g_screenCoords[index1] = g_screenCoords[index2];
//    g_screenCoords[index2] = tmp;
//  }

  g_texter = new gloost::TextureText(g_gloostFolder + "/data/fonts/gloost_Fixedsys_16_gui.png");


  // shaders
  reloadShaders();


  // points to build an example svo

//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/baahm_toroid.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/Hitachi_FH200.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/lucy_0.25.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/NissanPathfinder.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/sappho_low.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/Skull_low.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/sphere.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/terrain_05.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/dental_scan.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/teeth_5mp.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/Porsche996.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/Women_hair_undressed_low.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/dragon_vrip.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/triplane.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/fancy_art.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/NissanPathfinder.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/women.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/frog2_vertex_ao.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/Women_hair_undressed.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/box.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/throttle.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/alligator_head.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/Decimated_Head.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/Decimated_Head_low.ply");
//  gloost::PlyLoader loader("/home/otaco/Desktop/ply/human/shf--01.ply");


  gloost::PlyLoader loader("/home/otaco/Desktop/ply/frog2_seperated.ply");
  gloost::PlyLoader loader1("/home/otaco/Desktop/ply/frog2_seperated_ao.ply");

  for (unsigned i=0; i!=loader.getMesh()->getVertices().size(); ++i)
  {
    loader.getMesh()->getColors()[i].r = loader.getMesh()->getColors()[i].r * loader1.getMesh()->getColors()[i].r;
    loader.getMesh()->getColors()[i].g = loader.getMesh()->getColors()[i].g * loader1.getMesh()->getColors()[i].r;
    loader.getMesh()->getColors()[i].b = loader.getMesh()->getColors()[i].b * loader1.getMesh()->getColors()[i].r;
  }


  gloost::Mesh* mesh = loader.getMesh();
  mesh->center();
  mesh->scaleToSize(1.0);
  mesh->takeReference();



#ifdef BUILD_SVO
  buildSvo(mesh, maxDepth);
  buildSvoVisualization();
#endif



  // setup framebuffer
  unsigned int hostsideBufferSize = g_bufferWidth * g_bufferHeight * 3;
  g_renderBuffer                  = new float [hostsideBufferSize];
  float*       hostside_buffer    = new float [hostsideBufferSize];

  for (unsigned i=0; i<hostsideBufferSize; ++i)
  {
    g_renderBuffer[i]  = 0.2;
    hostside_buffer[i] = 0.2;
  }

  g_framebufferTextureId = gloost::TextureManager::get()->addTexture(new gloost::Texture( g_bufferWidth,
                                                                                          g_bufferHeight,
                                                                                          1,
                                                                                          (unsigned char*) hostside_buffer,
                                                                                          16,
                                                                                          GL_TEXTURE_2D,
                                                                                          GL_RGB16F,
                                                                                          GL_RGB,
                                                                                          GL_FLOAT));

  gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->setTexParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->setTexParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);


  // axis
  gloost::Mesh* axisMesh = new gloost::Mesh();
  axisMesh->getVertices().push_back(gloost::Point3(0.0, 0.0, 0.0));
  axisMesh->getColors().push_back(gloost::vec4(1.0, 0.0, 0.0, 0.8));
  axisMesh->getVertices().push_back(gloost::Point3(2.0, 0.0, 0.0));
  axisMesh->getColors().push_back(gloost::vec4(1.0, 0.0, 0.0, 0.8));
  axisMesh->getLines().push_back(gloost::Line(0, 1));

  axisMesh->getVertices().push_back(gloost::Point3(0.0, 0.0, 0.0));
  axisMesh->getColors().push_back(gloost::vec4(0.0, 1.0, 0.0, 0.8));
  axisMesh->getVertices().push_back(gloost::Point3(0.0, 2.0, 0.0));
  axisMesh->getColors().push_back(gloost::vec4(0.0, 1.0, 0.0, 0.8));
  axisMesh->getLines().push_back(gloost::Line(2, 3));

  axisMesh->getVertices().push_back(gloost::Point3(0.0, 0.0, 0.0));
  axisMesh->getColors().push_back(gloost::vec4(0.0, 0.0, 1.0, 0.8));
  axisMesh->getVertices().push_back(gloost::Point3(0.0, 0.0, 2.0));
  axisMesh->getColors().push_back(gloost::vec4(0.0, 0.0, 1.0, 0.8));
  axisMesh->getLines().push_back(gloost::Line(4, 5));

  g_drawAxisVbo = new gloost::Vbo(axisMesh);



  g_camera = new gloost::PerspectiveCamera(65.0,
                                           (float)g_screenWidth/(float)g_screenHeight,
                                           0.01,
                                           20.0);


  // raycast into framebuffer

//  static unsigned currentScreenCoordIndex      = 0;
//  static unsigned samplesPerThreadAnditeration = (g_bufferWidth*g_bufferHeight)/g_num_render_Threads;
//
//  std::cerr << std::endl << "g_num_render_Threads:                 " << g_num_render_Threads;
//  std::cerr << std::endl << "samplesPerThreadAnditeration: " << samplesPerThreadAnditeration;
//  std::cerr << std::endl << "g_bufferWidth*g_bufferHeight: " << g_bufferWidth*g_bufferHeight;
//
////  if (g_showRayCastImage)
//  {
//
//    for (unsigned i=0; i!=g_num_render_Threads; ++i)
//    {
//      std::cerr << std::endl << "Thread :" << i;
//      std::cerr << std::endl << "  From: " << currentScreenCoordIndex;
//      std::cerr << std::endl << "  To:   " << currentScreenCoordIndex+samplesPerThreadAnditeration;
//
//
//      g_threadGroup.create_thread( boost::bind( raycastIntoFrameBuffer,
//                                                currentScreenCoordIndex,
//                                                samplesPerThreadAnditeration,
//                                                i) );
//
//      currentScreenCoordIndex += samplesPerThreadAnditeration;
//    }
//
////    g_threadGroup.join_all();
//  }


}


//////////////////////////////////////////////////////////////////////////////////////////


void initWithContext()
{

}


//////////////////////////////////////////////////////////////////////////////////////////


void buildSvo(gloost::Mesh* mesh, unsigned int maxSvoDepth)
{

  if (g_svo == 0)
  {
    g_svo = new svo::Svo(maxSvoDepth);
  }

  svo::SvoBuilderFaces builder(g_num_building_Threads);

  builder.build(g_svo, mesh);
  g_svo->serializeSvo();

  //
  svo::Ag_colorAndNormalsTriangles generator;
  generator.generate(g_svo, mesh,
                     new gloost::ObjMatFile());
  g_voxelAttributes = generator.getAttributeBuffer(0);
  g_voxelAttributes->takeReference();


}


//////////////////////////////////////////////////////////////////////////////////////////


void buildSvoVisualization()
{

  int maxDepth = 20;

#ifdef BUILD_VISUALIZATION_NODES

  if (g_svoVisualizerNodes == 0)
  {
    g_svoVisualizerNodes = new svo::SvoVisualizer(maxDepth, SVO_VISUALIZER_MODE_WIRED_NODES);
    g_svoVisualizerNodes->build(g_svo);
  }

#endif


#ifdef BUILD_VISUALIZATION_LEAVES
  if (g_svoVisualizerLeaves == 0)
  {
    g_svoVisualizerLeaves = new svo::SvoVisualizer(maxDepth, SVO_VISUALIZER_MODE_BOXED_LEAFES);
    g_svoVisualizerLeaves->build(g_svo);
  }
#endif

}


//////////////////////////////////////////////////////////////////////////////////////////


void frameStep()
{
  ++g_frameCounter;


  glfwPollEvents();

  int x,y;

  glfwGetMousePos(&x,&y);

  g_mouse.setSpeedToZero();
  g_mouse.setLoc(x,g_screenHeight-y,0);


  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_1 ))
  {
    g_cameraRotateY += g_mouse.getSpeed()[0]*0.005;
    g_cameraRotateX += g_mouse.getSpeed()[1]*-0.005;

    g_cameraRotateX = gloost::clamp(g_cameraRotateX, -3.0f, 3.0f);
  }
  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_2 ))
  {
    g_cameraDistance += g_mouse.getSpeed()[1]*-0.005;
    g_cameraDistance = gloost::clamp(g_cameraDistance, 0.05f, 20.0f);
  }
  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_3 ))
  {

    gloost::Matrix viewMatrix = g_camera->getViewMatrix();
    viewMatrix.setTranslate(0.0,0.0,0.0);

    g_mouse.getSpeed()[2] = 0.0;

    g_modelOffset += viewMatrix*(g_mouse.getSpeed()*0.005);

//    g_modelOffset[0] += g_mouse.getSpeed()[0]*-0.0005;
//    g_modelOffset[1] += g_mouse.getSpeed()[1]*-0.0005;
//
//    std::cerr << std::endl << "g_modelOffset: " << g_modelOffset;
  }


  static bool mouse3Down = false;

  static unsigned rayCounter = 0;

//  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_3 ) && !mouse3Down)
//  {
//
//    mouse3Down = true;
//    ++rayCounter;
//
//    g_ray = g_camera->getPickRay( g_screenWidth,
//                                  g_screenHeight,
//                                  x,
//                                  g_screenHeight - y);
//    g_ray.setT(1000.0);
//
//
////    gloost::Vector3 sideVec = g_camera->getModelViewMatrix() * gloost::Vector3(0.1, 0.0, 0.0);
////    gloost::Vector3 upVec   = cross(sideVec, g_ray.getDestination()).normalized() * 0.1;
//
//    std::cerr << std::endl << "------------------------------------------------: ";
//    svo::CpuRaycasterSingleRay raycaster(true);
//    svo::SvoNode* node = raycaster.start(g_ray, g_svo);
//
//    if (node)
//    {
//      std::cerr << std::endl << "At " << rayCounter << " hit leaf node: " << node;
//    }
//
//
////    std::cerr << std::endl << "g_ray: " << g_ray;
//
//
//    // ray visualization
//    gloost::Mesh* lineMesh = new gloost::Mesh();
//
//    lineMesh->getVertices().push_back(g_ray.getOrigin());
//    lineMesh->getColors().push_back(gloost::vec4(1.0, 1.0, 0.0, 1.0));
//
//    lineMesh->getVertices().push_back(g_ray.getDestination());
//    lineMesh->getColors().push_back(gloost::vec4(1.0, 1.0, 1.0, 1.0));
//
//    lineMesh->getLines().push_back(gloost::Line(0, 1));
//
//
//    if (g_drawRayVbo)
//    {
//      g_drawRayVbo->dropReference();
//    }
//
//    g_drawRayVbo = new gloost::Vbo(lineMesh);
//    g_drawRayVbo->takeReference();
//  }
//  else
//  {
//    mouse3Down = false;
//  }


  unsigned currentScreenCoordIndex      = 0;
  unsigned samplesPerThreadAnditeration = (g_bufferHeight*g_bufferWidth)/g_num_render_Threads;

//  std::cerr << std::endl << "g_num_render_Threads:                 " << g_num_render_Threads;
//  std::cerr << std::endl << "samplesPerThreadAnditeration: " << samplesPerThreadAnditeration;
//  std::cerr << std::endl << "g_bufferWidth*g_bufferHeight: " << g_bufferWidth*g_bufferHeight;


  boost::thread_group threadGroup;


#ifdef USE_THREADED_RENDERING

  if (g_showRayCastImage)
  {

    for (unsigned i=0; i!=g_num_render_Threads; ++i)
    {
      threadGroup.create_thread( boost::bind( raycastIntoFrameBuffer,
                                                currentScreenCoordIndex,
                                                samplesPerThreadAnditeration,
                                                i) );

      currentScreenCoordIndex += samplesPerThreadAnditeration;
    }

    threadGroup.join_all();
  }
#else

  raycastIntoFrameBuffer(0,g_bufferHeight*g_bufferWidth,0);

#endif


//  if (g_frameCounter % 3 == 0)
  {
    {
      boost::mutex::scoped_lock(g_bufferAccessMutex);
      memcpy(gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->getPixels(),
             g_renderBuffer,
             g_bufferWidth*g_bufferHeight*3*sizeof(float));
    }

    gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->removeFromContext();
    gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->initInContext();
  }


}



//////////////////////////////////////////////////////////////////////////////////////////

/// main loop function, render with 3D setup

void
raycastIntoFrameBuffer(unsigned startIndex,
                       unsigned count,
                       unsigned threadId)
{
  svo::CpuRaycasterSingleRay  raycaster1;
  svo::CpuRaycasterSingleRay2 raycaster2;
  //    float* pixels =  (float*)(gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->getPixels());
//  std::vector<float>&  attribs = g_svo->getAttributeBuffer();

  gloost::Matrix offset;
  offset.setIdentity();
  offset.setTranslate(g_modelOffset);



//  while (g_toggle_run_raycasting)
  {
    for (unsigned i=startIndex; i!=startIndex+count; ++i)
    {
      int x = g_screenCoords[i][0];
      int y = g_screenCoords[i][1];

      gloost::Ray ray = g_camera->getPickRay( g_bufferWidth,
                                              g_bufferHeight,
                                              x,
                                              (int)g_bufferHeight - y);

//      ray = offset * ray;
      ray.getOrigin() += g_modelOffset;
      svo::SvoNode* node = 0;

      unsigned pixelIndex = (y*g_bufferWidth + x) * 3;

      if (g_toggle_renderer == 0)
      {
        node = raycaster1.start(ray, g_svo);


        if (node)
        {

//          boost::mutex::scoped_lock(g_bufferAccessMutex);
          unsigned attribPos   = node->getAttribPosition();
          unsigned attribIndex = g_voxelAttributes->getPackageIndex(attribPos);
          g_renderBuffer[pixelIndex++] = g_voxelAttributes->getVector()[attribIndex+3]; // <-- red
          g_renderBuffer[pixelIndex++] = g_voxelAttributes->getVector()[attribIndex+4]; // <-- green
          g_renderBuffer[pixelIndex++] = g_voxelAttributes->getVector()[attribIndex+5]; // <-- blue
        }
        else
        {
//          boost::mutex::scoped_lock(g_bufferAccessMutex);
          g_renderBuffer[pixelIndex++] = 0.3;
          g_renderBuffer[pixelIndex++] = 0.2;
          g_renderBuffer[pixelIndex++] = 0.2;
        }
      }
      else // raycaster2
      {
        node = raycaster2.start(ray, g_svo);

        if (node)
        {
//          boost::mutex::scoped_lock(g_bufferAccessMutex);
          unsigned attribPos   = node->getAttribPosition();
          unsigned attribIndex = g_voxelAttributes->getPackageIndex(attribPos);
          g_renderBuffer[pixelIndex++] = g_voxelAttributes->getVector()[attribIndex+3]; // <-- red
          g_renderBuffer[pixelIndex++] = g_voxelAttributes->getVector()[attribIndex+4]; // <-- green
          g_renderBuffer[pixelIndex++] = g_voxelAttributes->getVector()[attribIndex+5]; // <-- blue
        }
        else
        {
//          boost::mutex::scoped_lock(g_bufferAccessMutex);
          g_renderBuffer[pixelIndex++] = 0.2;
          g_renderBuffer[pixelIndex++] = 0.3;
          g_renderBuffer[pixelIndex++] = 0.2;
        }
      }



    }

  }




//  if (g_toggle_renderer == 0)
//  {
//////    std::cerr << std::endl << "raycaster 1 pushes: " << raycaster1._pushCounter;
//////    std::cerr << std::endl << "raycaster 1 pop:    " << raycaster1._popCounter;
//////    std::cerr << std::endl << "raycaster 1 while:  " << raycaster1._whileCounter;
////    std::cerr << std::endl << "raycaster 1 max stack depth:  " << raycaster2._max_stack_size;
//////    std::cerr << std::endl;
//////    raycaster1._pushCounter  = 0;
//////    raycaster1._popCounter   = 0;
//////    raycaster1._whileCounter = 0;
////    raycaster2._max_stack_size = 0;
//  }
//  else
//  {
//    std::cerr << std::endl << "raycaster 2 pushes: " << raycaster3._pushCounter;
//    std::cerr << std::endl << "raycaster 2 pop:    " << raycaster3._popCounter;
//    std::cerr << std::endl << "raycaster 2 while:  " << raycaster3._whileCounter;
//    std::cerr << std::endl;
//    raycaster3._pushCounter  = 0;
//    raycaster3._popCounter   = 0;
//    raycaster3._whileCounter = 0;
//  }



}


//////////////////////////////////////////////////////////////////////////////////////////


/// main loop function, render with 3D setup

void draw3d(void)
{
  /// clear the frame buffer and the depth buffer
  glClearColor(0.25, 0.25, 0.25, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  /// set the viewport
  glViewport(0, 0, g_screenWidth, g_screenHeight);


  g_camera->lookAt(gloost::Vector3(-sin(g_cameraRotateY),
                                   g_cameraRotateX,
                                   cos(g_cameraRotateY)).normalized() * g_cameraDistance,
                                   gloost::Point3(0.0f, 0.0f, 0.0f),
                                   gloost::Vector3(0.0f, 1.0f, 0.0f));

  g_camera->set();


//  gloost::Matrix offset;
//  offset.setIdentity();
//  offset.setTranslate(g_modelOffset);


  // draw model
  glPushMatrix();
  {

    // axis
    if (g_drawAxisVbo && g_showTextInfo)
    {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glPushMatrix();
      {

        glLineWidth(2.0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        g_SvoVisualizingShader->set();
          g_drawAxisVbo->bind();
          g_drawAxisVbo->draw();
          g_drawAxisVbo->disable();
        g_SvoVisualizingShader->disable();
      }
      glPopMatrix();
      glPopAttrib();
    }

    // draw visualization nodes
    if (g_showOctreeNodes && g_svoVisualizerNodes)
    {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glPushMatrix();
      {

        glLineWidth(2.0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        g_SvoVisualizingShader->set();
          g_svoVisualizerNodes->draw();
        g_SvoVisualizingShader->disable();
      }
      glPopMatrix();
      glPopAttrib();
    }

    // draw visualization leaves
    if (g_showOctreeLeaves && g_svoVisualizerLeaves)
    {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glPushMatrix();
      {

        glLineWidth(2.0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        g_SvoVisualizingShader->set();
          g_svoVisualizerLeaves->draw();
        g_SvoVisualizingShader->disable();
      }
      glPopMatrix();
      glPopAttrib();
    }

    // draw ray
    if (g_drawRayVbo)
    {

      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glPushMatrix();
      {

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glLineWidth(1.5);

        g_SvoVisualizingShader->set();
          g_drawRayVbo->bind();
            g_drawRayVbo->draw();
          g_drawRayVbo->disable();
        g_SvoVisualizingShader->disable();
      }
      glPopMatrix();
      glPopAttrib();
    }
  }
  glPopMatrix();



}


//////////////////////////////////////////////////////////////////////////////////////////


///

void draw2d()
{

  glPushMatrix();
  {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    {

      glViewport(0, 0, g_screenWidth, g_screenHeight);

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, (float) g_screenWidth, 0, (float) g_screenHeight, 0.1, 10.0);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      gluLookAt(0.0, 0.0, 5.0,
                0.0, 0.0, 0.0,
                0.0, 1.0, 0.0);


      glEnable(GL_COLOR_MATERIAL);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_LIGHTING);

      // color
      glColor4f(1.0f, 1.0f, 1.0f, 1.0);


      if (g_showRayCastImage)
      {
        gloost::Texture* frameBufferTexture = gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId);

        glDisable(GL_CULL_FACE);


        frameBufferTexture->bind();


        glPushMatrix();
        {
          glTranslated(0.0, g_screenHeight, 0.0);
          glScaled(g_screenWidth, -(int)g_screenHeight, 1.0);
          gloost::drawQuad();
        }
        glPopMatrix();
        frameBufferTexture->unbind();

      }

      static double timeAccum           = 0;
      static double avarageTimePerFrame = 1.0;

      timeAccum += g_timePerFrame;

      if (g_frameCounter % 20 == 0)
      {
        avarageTimePerFrame = timeAccum / 20.0;
        timeAccum = 0;
      }



      glColor4f(1.0f, 1.0f, 1.0f, 1.0);

      // info text
      if (g_showTextInfo)
      {
        g_texter->begin();
        {
          g_texter->renderTextLine(20, g_screenHeight -20, "glfw + VBO test");
          g_texter->renderFreeLine();
          g_texter->renderTextLine("Time per frame: " + gloost::toString(avarageTimePerFrame));
          g_texter->renderTextLine("fps: " + gloost::toString(1.0/avarageTimePerFrame));
          g_texter->renderFreeLine();
          glColor4f(1.0f, 1.0f, 1.0f, 0.5);
          g_texter->renderTextLine("(h) Show text info:      " + gloost::toString( g_showTextInfo )) ;
          g_texter->renderTextLine("(3) Show octree nodes:   " + gloost::toString( g_showOctreeNodes )) ;
          g_texter->renderTextLine("(4) Show octree leaves:  " + gloost::toString( g_showOctreeLeaves )) ;
          g_texter->renderFreeLine();
          g_texter->renderFreeLine();
          glColor4f(0.8f, 0.8f, 1.0f, 1.0);
          g_texter->renderTextLine("g_maxRayCastDepth:       " + gloost::toString(g_maxRayCastDepth));
          g_texter->renderTextLine("raycaster:               " + gloost::toString(g_toggle_renderer+1));
        }
        g_texter->end();

      }

    }
    glPopAttrib();
  }
  glPopMatrix();
}


////////////////////////////////////////////////////////////////////////////////

void reloadShaders()
{

  if (g_SvoVisualizingShader)
  {//  glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    delete g_SvoVisualizingShader;
  }

  g_SvoVisualizingShader = new gloost::Shader(g_dataPath + "shaders/color_only.vs",
                                              g_dataPath + "shaders/color_only.fs");




}

////////////////////////////////////////////////////////////////////////////////


  /// free all resources we created

void cleanup()
{

}


////////////////////////////////////////////////////////////////////////////////


/// this function is triggered when the screen is resized

void resize(int width, int height)
{
  /// update the global screen size
  g_screenWidth  = width;
  g_screenHeight = height;

  /// recalc the aspect ratio
  const float ar = (float) width / (float) height;

  if (g_camera)
  {
    g_camera->setAspect(ar);
  }

  /// recalc the frustum
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-ar, ar, -1.0, 1.0, 0.1, 100.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity() ;
}


//////////////////////////////////////////////////////////////////////////////////////////


/// this function is called by glut when a key press occured

void key(int key, int state)
{
  if (state)
  {

    switch (key)
    {
      /// press ESC or q to quit
      case GLFW_KEY_ESC:
        cleanup();
        exit(0);
        break;
      case 'Q':
        cleanup();
        exit(0);
        break;
      case 'H':
        g_showTextInfo = !g_showTextInfo;
        break;
      case '3':
        g_showOctreeNodes = !g_showOctreeNodes;
        break;
      case '4':
        g_showOctreeLeaves = !g_showOctreeLeaves;
        break;
      case 'R':
        reloadShaders();
        break;

      case ' ':
          g_toggle_renderer = !g_toggle_renderer;
        break;



      case 'B':
        --g_maxRayCastDepth;
        g_maxRayCastDepth = gloost::clamp(g_maxRayCastDepth, 1, 1000);
        break;
      case 'F':
        g_showRayCastImage = !g_showRayCastImage;
        break;


      case '0':
        std::cerr << std::endl;
        std::cerr << std::endl << "CLEARING ALL MESH DATA: ";
        std::cerr << std::endl;
        break;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[])
{

  /// load and intialize stuff for our demo
  init();


  // init glfw
  if ( glfwInit( ) == GL_FALSE)
  {
    std::cerr << std::endl;
    std::cerr << std::endl << "ERROR while calling glfwInit()";
    return EXIT_FAILURE;
  }

  std::cerr << std::endl;
  std::cerr << std::endl << "glfw initialised...";
  std::cerr << std::endl;
  std::flush(std::cout);



  // Open an OpenGL window
//  glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_FALSE);
  glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 16);
//  glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);



  if( !glfwOpenWindow( g_screenWidth, g_screenHeight, 0,0,0,0,0,0, GLFW_WINDOW ) )
  {
    std::cerr << std::endl << "ERROR while calling glfwOpenWindow(...)";
    glfwTerminate();
    return EXIT_FAILURE;
  }


  glfwSetWindowPos( 600, 100);
  glfwSetWindowSizeCallback(resize);
  glfwSetKeyCallback(key);



  /// initialize GLEW
  if (GLEW_OK != glewInit())
  {
    /// ... or die trying
    std::cout << "'glewInit()' failed." << std::endl;
    exit(0);
  }


  /// set some gl states
  glEnable(GL_TEXTURE_2D);   /// enable texture support
  glEnable(GL_DEPTH_TEST);   /// enable z-buffering
  glDepthFunc(GL_LEQUAL);

//  glfwSwapInterval( 1 );


  /// load and intialize stuff for our demo
  initWithContext();

//  glfwCreateThread( pollEvents, NULL);


  // Main loop
  int running = GL_TRUE;
  while( running )
  {
    double startTime = glfwGetTime();

    // OpenGL rendering goes here...
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    frameStep();
    draw3d();
    draw2d();

    // Swap front and back rendering buffers
    glfwSwapBuffers();

    // Check if ESC key was pressed or window was closed
    running = glfwGetWindowParam( GLFW_OPENED );

    g_timePerFrame = glfwGetTime() - startTime;
  }


  glfwCloseWindow();
  glfwTerminate();

  /// cleanup all resources we created for the demo
  cleanup();


  return EXIT_SUCCESS;
}


