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
static unsigned int g_screenWidth  = 1280;
static unsigned int g_screenHeight = 800;

/// frameCounter
unsigned int g_frameCounter = 0;

#include <gloostRenderGoodies.h>
#include <Vbo.h>

gloost::Vbo* g_vbo = 0;


#include <PointLight.h>
gloost::PointLight* g_pointLight = 0;


#include <TextureText.h>
gloost::TextureText* g_texter = 0;


double g_timePerFrame = 0;

#include <Shader.h>
#include <UniformSet.h>
gloost::Shader*     g_modelShader      = 0;
gloost::UniformSet* g_modelUniforms    = 0;

gloost::Shader*     g_SvoTextureShader   = 0;
gloost::UniformSet* g_SvoTextureUniforms = 0;


gloost::Shader*     g_SvoVisualizingShader      = 0;


std::string g_gloostFolder = "../../gloost/";
std::string g_dataPath     = "../data/";
std::string g_plyPath      = "/home/otaco/Desktop/ply/";
std::string g_gbmPath      = "/home/otaco/Desktop/gbm/";
std::string g_meshFilename;

#include <Matrix.h>
gloost::Matrix g_sizeAndCenterMatrix;

#include <Mouse.h>
gloost::Mouse g_mouse;


gloost::Point3 g_modelOffset;
float          g_cameraRotateY  = 0;
float          g_cameraRotateX  = 0;
float          g_cameraDistance = 2.0;


#include <PerspectiveCamera.h>
gloost::PerspectiveCamera*   g_camera;


#include <PlyLoader.h>
#include <GbmWriter.h>
#include <GbmLoader.h>
#include <Mesh.h>
gloost::Mesh* g_mesh = 0;

// SVO
#include <Svo.h>
svo::Svo* g_svo = 0;
#include <SvoNode.h>
#include <SvoBuilderHeightmap.h>
#include <SvoBuilderVertices.h>
#include <SvoBuilderFaces.h>

#include <SvoVisualizer.h>
svo::SvoVisualizer* g_svoVisualizerNodes  = 0;
svo::SvoVisualizer* g_svoVisualizerLeaves = 0;

#include <chrono>



// info
bool g_showTextInfo                = true;
bool g_showModel                   = false;
bool g_showOctreeNodes             = false;
bool g_showOctreeLeaves            = true;
bool g_showSerializedSvoTexture    = false;
bool g_showSerializedAttribTexture = false;


void init();
void buildSvo(gloost::Mesh* mesh, unsigned int maxSvoDepth);
void buildSvoVisualization();
void draw3d(void);
void draw2d();
void cleanup();
void mouseFunc(int button, int state, int mouse_h, int mouse_v);
void reloadShaders();
void resize(int width, int height);
void key(unsigned char key, int x, int y);
void motionFunc(int mouse_h, int mouse_v);
void idle(void);


gloost::DrawableCompound* g_rayVisDrawable = 0;


////////////////////////////////////////////////////////////////////////////////


  /// load and initialize resources for our demo

void init()
{

//  #define USE_GBM_FILES
  #define OPEN_GL_WINDOW
  #define DRAW_MESH

  #define BUILD_SVO
//  #define USE_VERTICES_ONLY
  #define BUILD_VISUALIZATION_NODES
  #define BUILD_VISUALIZATION_LEAVES
//  #define WRITE_VISUALIZATIONS

  #define SERIALIZE_BUFFERS
  //#define WRITE_SERIALIZED_BUFFERS



  unsigned int maxSvoDepth = 7;


//  g_meshFilename = "bogenschuetze-01.ply";
//  g_meshFilename = "leaves.ply";
//  g_meshFilename = "vcg_david_1M_ao.ply";
//  g_meshFilename = "david_2mm_final_ao.ply";
//  g_meshFilename = "Hitachi_FH200.ply";
//  g_meshFilename = "xyzrgb_statuette.ply";
//  g_meshFilename = "xyzrgb_dragon_low.ply";
//  g_meshFilename = "xyzrgb_dragon.ply";
//  g_meshFilename = "dragon_vrip_verylow.ply";
//  g_meshFilename = "dragon_vrip_low.ply";
//  g_meshFilename = "dragon_vrip.ply";
//  g_meshFilename = "david_angel_low.ply";
//  g_meshFilename = "david_angel.ply";
//  g_meshFilename = "Decimated_Head_low.ply";
//  g_meshFilename = "Decimated_Head.ply";
//  g_meshFilename = "skelet.ply";
//  g_meshFilename = "Decimated_Head_high.ply";
//  g_meshFilename = "alligator_head.ply";
//  g_meshFilename = "furniture_leg_color.ply";
//  g_meshFilename = "GlenRoseTrack.ply";
//  g_meshFilename = "throttle_low.ply";
//  g_meshFilename = "throttle.ply";
//  g_meshFilename = "face_p.ply";
//  g_meshFilename = "stego_color.ply";
//  g_meshFilename = "female02.ply";
//  g_meshFilename = "women.ply";
//  g_meshFilename = "Alfa_Romeo_159.ply";
//  g_meshFilename = "lambo.ply";
//  g_meshFilename = "dental_scan.ply";
//  g_meshFilename = "GlenRoseTrack_high.ply";
//  g_meshFilename = "monster.ply";
//  g_meshFilename = "monster_verylow.ply";
//  g_meshFilename = "malaysia.ply";
//  g_meshFilename = "incendia.ply";
//  g_meshFilename = "julia.ply";
//  g_meshFilename = "quaternion_julia.ply";
//  g_meshFilename = "venus.ply";
//  g_meshFilename = "face_figurine.ply";
//  g_meshFilename = "Infinite-Level_02.ply";
//  g_meshFilename = "teeth_5mp.ply";
//  g_meshFilename = "dental_crown.ply";
//  g_meshFilename = "dental_scan.ply";
//  g_meshFilename = "Sam_Body_Hat.ply";
//  g_meshFilename = "3d_face.ply";
//  g_meshFilename = "full_body.ply";
//  g_meshFilename = "lucy.ply";
//  g_meshFilename = "lucy_0.5.ply";
//  g_meshFilename = "lucy_0.25.ply";
//  g_meshFilename = "lucy_0.125.ply";
//  g_meshFilename = "male02.ply";
//  g_meshFilename = "happy.ply";
//  g_meshFilename = "bunny_1100k.ply";
//  g_meshFilename = "bunny_55k.ply";
//  g_meshFilename = "Women_hair.ply";
//  g_meshFilename = "Women_hair_low.ply";
//  g_meshFilename = "box.ply";
//  g_meshFilename = "Women_hair_undressed_low.ply";
//  g_meshFilename = "sphere.ply";
//  g_meshFilename = "plane.ply";
//  g_meshFilename = "baahm_toroid.ply";
//  g_meshFilename = "gg_logo.ply";
//  g_meshFilename = "fancy_art.ply";
//  g_meshFilename = "fancy_art_high.ply";
  g_meshFilename = "frog2_vertex_ao.ply";
//  g_meshFilename = "two_triangles.ply";
//  g_meshFilename = "human/secretary_low.ply";
//  g_meshFilename = "human/Girl N270309.ply";
//  g_meshFilename = "triplane.ply";
//  g_meshFilename = "Infinite-Level_02.ply";
//  g_meshFilename = "steppos_kueche_01.ply";


#ifdef USE_GBM_FILES
  gloost::GbmLoader gbmLoader(g_gbmPath + gloost::pathToBasename(g_meshFilename) + ".gbm");
  g_mesh = gbmLoader.getMesh();
#else
  gloost::PlyLoader ply(g_plyPath + g_meshFilename);
  g_mesh = ply.getMesh();
#endif


  /// transform
  gloost::Matrix rotateMat;
  rotateMat.setIdentity();
//  rotateMat.setRotateX(gloost::deg2rad(-90.0));

  gloost::Matrix translateMat;
  translateMat.setIdentity();
  translateMat.setTranslate(g_mesh->getOffsetToCenter());

  gloost::Matrix scaleMat;
  scaleMat.setIdentity();
  scaleMat.setScale(g_mesh->getScaleFactorToSize(1.0));

  g_sizeAndCenterMatrix.setIdentity();
  g_sizeAndCenterMatrix =  scaleMat * (rotateMat * translateMat);

  g_mesh->transform(g_sizeAndCenterMatrix, true);
//  g_mesh->center();
  g_mesh->normalizeNormals();


#ifdef OPEN_GL_WINDOW
#ifdef DRAW_MESH
  std::cerr << std::endl << "vbo: " << "created";
  g_vbo = new gloost::Vbo(g_mesh);
#endif

    /// create a light and set position and components
  g_pointLight = new gloost::PointLight();

  g_pointLight->setPosition(5.0f, 5.0f, 15.0f);
  g_pointLight->setAmbient(0.2f, 0.2f, 0.2f, 1.0f);
  g_pointLight->setDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
  g_pointLight->setSpecular(1.0f, 1.0f, 1.0f, 1.0f);


//  g_texter = new gloost::TextureText(g_gloostFolder + "/data/fonts/gloost_Fixedsys_16_gui.png");
  g_texter = new gloost::TextureText(g_dataPath + "/fonts/gloost_Fixedsys_16_gui.png");


  // shaders
  reloadShaders();


  g_modelUniforms = new gloost::UniformSet();
  g_modelUniforms->set_sampler2D("environmentMap",
//                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/bensFrontyard_blured.jpg"));
//                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/probe.jpg"));
                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/cedarCity.jpg"));
//                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/skies.jpg"));
//                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/uni-washington.jpg"));
//                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/christmas.jpg"));
//                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/scanner.jpg"));
  g_modelUniforms->set_float("reflection", 0.1);
  g_modelUniforms->set_float("shininess", 40.0);
  g_modelUniforms->set_float("specular",  0.05);


  g_SvoTextureUniforms = new gloost::UniformSet();

  g_camera = new gloost::PerspectiveCamera(50.0,
                                         (float)g_screenWidth/(float)g_screenHeight,
                                         0.01,
                                         20.0);

#endif


#ifdef BUILD_SVO
  buildSvo(g_mesh, maxSvoDepth);
  buildSvoVisualization();
#endif

//  g_mesh->interleave(true);

}


//////////////////////////////////////////////////////////////////////////////////////////


void buildSvo(gloost::Mesh* mesh, unsigned int maxSvoDepth)
{

  if (g_svo == 0)
  {
    g_svo = new svo::Svo(maxSvoDepth);
  }


#ifdef USE_VERTICES_ONLY
  svo::SvoBuilderVertices fromVertexBuilder;
  fromVertexBuilder.build(g_svo, mesh);
#else
  svo::SvoBuilderFaces fromFaceBuilder;
  fromFaceBuilder.build(g_svo, mesh);
//  g_rayVisDrawable = fromFaceBuilder._raysDrawable;
#endif


#ifdef SERIALIZE_BUFFERS
  g_svo->serializeSvo();

#ifdef OPEN_GL_WINDOW
  g_SvoTextureUniforms->set_sampler2D("svoTexture" ,g_svo->getSvoBufferTextureId());
  g_SvoTextureUniforms->set_float("numNodes"       ,g_svo->getNumNodes());
  g_SvoTextureUniforms->set_float("numAttribs"     ,g_svo->getCurrentAttribPosition()/2.0);
#endif

  g_svo->serializeAttributeBuffer();

#ifdef WRITE_SERIALIZED_BUFFERS
  g_svo->writeSerialBuffersToFile(g_dataPath + "gbi/", gloost::pathToBasename(g_meshFilename));
#endif
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////


void buildSvoVisualization()
{

  int maxDepth = 80;

#ifdef BUILD_VISUALIZATION_NODES

  if (g_svoVisualizerNodes == 0)
  {
    g_svoVisualizerNodes = new svo::SvoVisualizer(maxDepth, SVO_VISUALIZER_MODE_WIRED_NODES);
    g_svoVisualizerNodes->build(g_svo);

#ifdef WRITE_VISUALIZATIONS
    g_svoVisualizerNodes->saveAsGbm("/home/otaco/Desktop/gbm/nodes.gbm");
#endif

  }

#endif


#ifdef BUILD_VISUALIZATION_LEAVES

  if (g_svoVisualizerLeaves == 0)
  {
    g_svoVisualizerLeaves = new svo::SvoVisualizer(maxDepth, SVO_VISUALIZER_MODE_BOXED_LEAFES);
    g_svoVisualizerLeaves->build(g_svo);

#ifdef WRITE_VISUALIZATIONS
    g_svoVisualizerLeaves->saveAsGbm("/home/otaco/Desktop/gbm/leaves.gbm");
#endif

  }

#endif

}


////////////////////////////////////////////////////////////////////////////////


void reloadShaders()
{

  if (g_modelShader)
  {
    delete g_modelShader;
  }

#if 1
    g_modelShader = new gloost::Shader(g_dataPath + "shaders/envmap_and_material.vs",
                                       g_dataPath + "shaders/envmap_and_material.fs");
#else
  g_modelShader = new gloost::Shader(g_dataPath + "shaders/color_only.vs",
                                     g_dataPath + "shaders/color_only.fs");
#endif

  if (g_SvoTextureShader)
  {
    delete g_SvoTextureShader;
  }

  g_SvoTextureShader = new gloost::Shader(g_dataPath + "shaders/showSvoTexture.vs",
                                          g_dataPath + "shaders/showSvoTexture.fs");


  if (g_SvoVisualizingShader)
  {
    delete g_SvoVisualizingShader;
  }

  g_SvoVisualizingShader = new gloost::Shader(g_dataPath + "shaders/color_only.vs",
                                              g_dataPath + "shaders/color_only.fs");
}


//////////////////////////////////////////////////////////////////////////////////////////

void frameStep()
{
  glfwPollEvents();

  int x,y;

  glfwGetMousePos(&x,&y);

  g_mouse.setSpeedToZero();
  g_mouse.setLoc(x,g_screenHeight-y,0);


  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_3 ))
  {
    gloost::Vector3 offset(g_mouse.getSpeed()[0]*0.004,
                           g_mouse.getSpeed()[1]*0.004,
                           0.0);

    offset = g_camera->getModelViewMatrix().inverted() * offset;

    g_modelOffset +=  offset;

  }


  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_1 ))
  {
    g_cameraRotateY += g_mouse.getSpeed()[0]*0.005;
    g_cameraRotateX += g_mouse.getSpeed()[1]*-0.005;

    g_cameraRotateX = gloost::clamp(g_cameraRotateX, -3.0f, 3.0f);
  }
  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_2 ))
  {
    g_cameraDistance += g_mouse.getSpeed()[1]*-0.005;
    g_cameraDistance = gloost::clamp(g_cameraDistance, 0.1f, 4.0f);
  }





}



//////////////////////////////////////////////////////////////////////////////////////////


/// main loop function, render with 3D setup

void draw3d(void)
{

  /// increment the frame counter
  ++g_frameCounter;


  /// clear the frame buffer and the depth buffer
  glClearColor(0.25, 0.25, 0.25, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  /// set the viewport
  glViewport(0, 0, g_screenWidth, g_screenHeight);


  g_camera->lookAt(gloost::Vector3(-sin(g_cameraRotateY), g_cameraRotateX, cos(g_cameraRotateY)).normalized() * g_cameraDistance,
                                     gloost::Point3(0.0f, 0.0f, 0.0f),
                                     gloost::Vector3(0.0f, 1.0f, 0.0f));

  g_camera->set();


  g_pointLight->setPosition(g_camera->getModelViewMatrix().inverted() * gloost::Point3(5,5,15));
  g_pointLight->set();


  gloost::Matrix offset;
  offset.setIdentity();
  offset.setTranslate(g_modelOffset);


  // draw model
  glPushMatrix();
  {
    gloostMultMatrix(offset.data());


#ifdef DRAW_MESH
    if (g_showModel && g_vbo)
    {
      g_modelShader->set();
      g_modelUniforms->applyToShader(g_modelShader);
        g_vbo->bind();
          g_vbo->draw();
        g_vbo->disable();
      g_modelShader->disable();
    }
#endif

    // draw visualization leafs
    if (g_showOctreeLeaves && g_svoVisualizerLeaves)
    {
      glPushAttrib(GL_ALL_ATTRIB_BITS);

      glPushMatrix();
      {
        g_modelShader->set();
        g_modelUniforms->applyToShader(g_modelShader);
        {
          g_svoVisualizerLeaves->draw();
        }
        g_modelShader->disable();
      }
      glPopMatrix();

      glPopAttrib();
    }


    // draw visualization nodes
    if (g_showOctreeNodes && g_svoVisualizerNodes)
    {
      glPushAttrib(GL_ALL_ATTRIB_BITS);

      glDepthFunc(GL_LEQUAL);

      glPushMatrix();
      {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        g_SvoVisualizingShader->set();
        {
          g_svoVisualizerNodes->draw();
        }
        g_SvoVisualizingShader->disable();
      }
      glPopMatrix();

      glPopAttrib();
    }


    // draw ray visualization
//    if (g_rayVisDrawable)
//    {
//      glPushAttrib(GL_ALL_ATTRIB_BITS);
//      glPushMatrix();
//      {
//        glEnable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//        glEnable(GL_COLOR_MATERIAL);
//
//
//        g_rayVisDrawable->updateAndDraw();
//      }
//      glPopMatrix();
//
//      glPopAttrib();
//    }



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


      // show svo texture
      if (g_svo && g_showSerializedSvoTexture)
      {
        g_SvoTextureShader->set();
        g_SvoTextureUniforms->applyToShader(g_SvoTextureShader);

        gloost::Texture* svoBuffer = gloost::TextureManager::getInstance()->getTextureWithoutRefcount(g_svo->getSvoBufferTextureId());

        svoBuffer->bind();
        glPushMatrix();
        {
//          glTranslatef(100.0, 100.0, 0.0);
          glScalef(svoBuffer->getWidth()*1.0,
                   svoBuffer->getHeight()*1.0,
                   1.0f);
          gloost::drawQuad();
        }
        glPopMatrix();
        svoBuffer->unbind();
        g_SvoTextureShader->disable();
      }


      // show attribute texture
      if (g_svo && g_showSerializedAttribTexture)
      {
        gloost::Texture* attributeBuffer = gloost::TextureManager::getInstance()->getTextureWithoutRefcount(g_svo->getAttributeBufferTextureId());

        attributeBuffer->bind();
        glPushMatrix();
        {
          glTranslatef(g_screenWidth-attributeBuffer->getWidth()*1.0, 0.0, 0.0);
          glScalef(attributeBuffer->getWidth()*1.0,
                   attributeBuffer->getHeight()*1.0,
                   1.0f);
          gloost::drawQuad();
        }
        glPopMatrix();
        attributeBuffer->unbind();
      }

      // info text
      if (g_showTextInfo)
      {
        g_texter->begin();
        {
          g_texter->renderTextLine(20, g_screenHeight -20, "glfw + VBO test");
          g_texter->renderFreeLine();
          g_texter->renderTextLine("Time per frame: " + gloost::toString(g_timePerFrame));
          g_texter->renderTextLine("fps: " + gloost::toString(1.0/g_timePerFrame));
          g_texter->renderFreeLine();
          glColor4f(0.8f, 0.8f, 1.0f, 0.75);
          g_texter->renderTextLine("(h) Show text info:      " + gloost::toString( g_showTextInfo )) ;
          g_texter->renderTextLine("(2) Show Mesh:           " + gloost::toString( g_showModel )) ;
          g_texter->renderTextLine("(3) Show octree nodes:   " + gloost::toString( g_showOctreeNodes )) ;
          g_texter->renderTextLine("(4) Show octree leaves:  " + gloost::toString( g_showOctreeLeaves )) ;
          g_texter->renderTextLine("(5) Show SVO texture:    " + gloost::toString( g_showSerializedSvoTexture )) ;
          g_texter->renderTextLine("(6) Show Attrib texture: " + gloost::toString( g_showSerializedAttribTexture)) ;
          g_texter->renderFreeLine();
          glColor4f(0.8f, 1.0f, 0.8f, 0.75);
          g_texter->renderTextLine("(R) Reload shaders");
          g_texter->renderTextLine("(C) Center model");
          g_texter->renderTextLine("(P) Render Mesh as GL_POINTS");
          g_texter->renderTextLine("(T) Render Mesh as GL_TRIANGLES");
        }
        g_texter->end();

      }

    }
    glPopAttrib();
  }
  glPopMatrix();
}


////////////////////////////////////////////////////////////////////////////////


  /// free all resources we created

void cleanup()
{
  if (g_vbo)
  {
    g_vbo->dropReference();
  }
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

  g_camera->setAspect(ar);

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

  // drawmode
  if (glfwGetKey('P') == GLFW_PRESS)
  {
    g_vbo->setDrawMode(GL_POINTS);
    glPointSize(1);
  }
  else if (glfwGetKey('T') == GLFW_PRESS)
  {
    g_vbo->setDrawMode(GL_TRIANGLES);
  }
  else if (glfwGetKey('Q') == GLFW_PRESS)
  {
    g_vbo->setDrawMode(GL_QUADS);
  }
  else if (glfwGetKey('L') == GLFW_PRESS)
  {
    glLineWidth(1);
    g_vbo->setDrawMode(GL_LINES);
  }

  if (state)
  {

    switch (key)
    {
      /// press ESC or q to quit
      case GLFW_KEY_ESC:
        cleanup();
        exit(0);
        break;
      case 'H':
        g_showTextInfo = !g_showTextInfo;
        break;
      case '2':
        g_showModel = !g_showModel;
        break;
      case '3':
        g_showOctreeNodes = !g_showOctreeNodes;
        break;
      case '4':
        g_showOctreeLeaves = !g_showOctreeLeaves;
        break;
      case '5':
        g_showSerializedSvoTexture = !g_showSerializedSvoTexture;
        break;
      case '6':
        g_showSerializedAttribTexture = !g_showSerializedAttribTexture;
        break;
      case 'R':
        reloadShaders();
        break;
      case 'C':
        g_modelOffset = gloost::Point3();
        break;
      case '0':
        std::cerr << std::endl;
        std::cerr << std::endl << "CLEARING ALL MESH DATA: ";
        std::cerr << std::endl;
        g_mesh->clear();
//        g_svoVisualizerNodes->getMesh()->clear();
//        g_svoVisualizerLeaves->getMesh()->clear();
        break;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[])
{
//  glfwCreateThread( pollEvents, NULL);

  /// enable sync to vblank on linux to control the demo fps
#ifdef LINUX
//  setenv("__GL_SYNC_TO_VBLANK","1",true);
#endif



  /// load and intialize stuff for our demo
  init();


#ifdef OPEN_GL_WINDOW

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
  glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
//  glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);



  if( !glfwOpenWindow( g_screenWidth, g_screenHeight, 0,0,0,0,0,0, GLFW_WINDOW ) )
  {
    std::cerr << std::endl << "ERROR while calling glfwOpenWindow(...)";
    glfwTerminate();
    return EXIT_FAILURE;
  }


  glfwSetWindowPos( 700, 100);
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

#endif

  /// cleanup all resources we created for the demo
  cleanup();


  return EXIT_SUCCESS;
}


