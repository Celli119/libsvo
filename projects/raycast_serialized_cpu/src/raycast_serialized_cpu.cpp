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
float          g_cameraDistance = 2.0;


gloost::Ray g_ray;
#include <gloost/Mesh.h>
#include <gloost/Vbo.h>
gloost::Vbo* g_drawRayVbo  = 0;


#include <gloost/Shader.h>
#include <gloost/UniformSet.h>
gloost::Shader*     g_SvoVisualizingShader = 0;


// setup for rendering into frame buffer
#include <gloost/TextureManager.h>
#include <gloost/Texture.h>

unsigned g_framebufferTextureId = 0;

#include <RaycasterSerialSvo.h>
#include <SvoLoader.h>
svo::SvoLoader* g_svoData = 0;


// info
bool g_showTextInfo     = true;
bool g_showRayCastImage = false;

void init();
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



std::vector<gloost::Vector2> g_screenCoords;


float* g_renderBuffer = 0;

#include <boost/thread.hpp>
boost::thread_group g_threadGroup;
boost::mutex g_bufferAccessMutex;

bool  g_toggle_run_raycasting = true;




////////////////////////////////////////////////////////////////////////////////


  /// load and initialize resources for our demo

void init()
{


  const std::string svo_data_path = "/home/otaco/Desktop/SVO_DATA/";
  g_svoData = new svo::SvoLoader(svo_data_path + "frog2_seperated_8");


  g_bufferWidth  = g_screenWidth/4;
  g_bufferHeight = g_screenHeight/4;

  // create screencoords
  g_screenCoords = std::vector<gloost::Vector2>(g_bufferWidth*g_bufferHeight);

  for (unsigned y=0; y!=g_bufferHeight; ++y)
  for (unsigned x=0; x!=g_bufferWidth; ++x)
  {
    unsigned index = y*g_bufferWidth +x;
    g_screenCoords[index] = gloost::Vector2(x,y);
  }

  // mix screen coords
  for (unsigned i=0; i!=g_screenCoords.size()*10; ++i)
  {
    unsigned index1 = (unsigned)(gloost::frand()*g_screenCoords.size());
    unsigned index2 = (unsigned)(gloost::frand()*g_screenCoords.size());

    gloost::Vector2 tmp    = g_screenCoords[index1];
    g_screenCoords[index1] = g_screenCoords[index2];
    g_screenCoords[index2] = tmp;
  }

  g_texter = new gloost::TextureText(g_gloostFolder + "/data/fonts/gloost_Fixedsys_16_gui.png");



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
                                                                                          GL_RGB32F,
                                                                                          GL_RGB,
                                                                                          GL_FLOAT));

  gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->setTexParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->setTexParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);


  g_camera = new gloost::PerspectiveCamera(50.0,
                                           (float)g_screenWidth/(float)g_screenHeight,
                                           0.01,
                                           20.0);

  g_camera->lookAt(gloost::Point3(0.0f, 1.0f, 3.0f),
                   gloost::Point3(0.0f, 0.0f, 0.0f),
                   gloost::Vector3(0.0f, 1.0f, 0.0f));





  // raycast into framebuffer

  static unsigned g_numThreads                 = 12;
  static unsigned currentScreenCoordIndex      = 0;
  static unsigned samplesPerThreadAnditeration = (g_bufferHeight*g_bufferWidth)/g_numThreads;

  std::cerr << std::endl << "g_numThreads:                 " << g_numThreads;
  std::cerr << std::endl << "samplesPerThreadAnditeration: " << samplesPerThreadAnditeration;
  std::cerr << std::endl << "g_bufferWidth*g_bufferHeight: " << g_bufferWidth*g_bufferHeight;

//  if (g_showRayCastImage)
  {

    for (unsigned i=0; i!=g_numThreads; ++i)
    {

      std::cerr << std::endl << "Thread :" << i;
      std::cerr << std::endl << "  From: " << currentScreenCoordIndex;
      std::cerr << std::endl << "  To:   " << currentScreenCoordIndex+samplesPerThreadAnditeration;


      g_threadGroup.create_thread( boost::bind( raycastIntoFrameBuffer,
                                                currentScreenCoordIndex,
                                                samplesPerThreadAnditeration,
                                                i) );

      currentScreenCoordIndex += samplesPerThreadAnditeration;
    }

//    g_threadGroup.join_all();
  }
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
    g_cameraDistance = gloost::clamp(g_cameraDistance, 0.15f, 20.0f);
  }


  static bool mouse3Down = false;

  static unsigned rayCounter = 0;

  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_3 ) && !mouse3Down)
  {

    mouse3Down = true;
    ++rayCounter;

    g_ray = g_camera->getPickRay( g_screenWidth,
                                  g_screenHeight,
                                  x,
                                  g_screenHeight - y);
    g_ray.setT(1000.0);

    std::cerr << std::endl << "------------------------------------------------: ";
//    svo::RaycasterSerialSvo raycaster(true);
//    unsigned hitNodeIndex = raycaster.start_with_SvoData(g_ray,
//                                                         g_svoData->getSerializedSvo());
    unsigned hitNodeIndex = 0;
    std::cerr << std::endl << "At " << rayCounter << " hit leaf node: " << hitNodeIndex;
  }
  else
  {
    mouse3Down = false;
  }


  if (g_frameCounter % 3 == 0)
  {

    memcpy(gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->getPixels(),
           g_renderBuffer,
           g_bufferWidth*g_bufferHeight*3*sizeof(float));

    gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->setDirty();
  }


}



//////////////////////////////////////////////////////////////////////////////////////////

/// main loop function, render with 3D setup

void
raycastIntoFrameBuffer(unsigned startIndex,
                       unsigned count,
                       unsigned threadId)
{
//  svo::RaycasterSerialSvo raycaster;




  while (g_toggle_run_raycasting)
  {
    for (unsigned i=startIndex; i!=startIndex+count; ++i)
    {
      int x = g_screenCoords[i][0];
      int y = g_screenCoords[i][1];

      gloost::Ray ray = g_camera->getPickRay( g_bufferWidth,
                                              g_bufferHeight,
                                              x,
                                              (int)g_bufferHeight - y);
  //      ray.setT(1.0);
//      unsigned nodeIndex = raycaster.start_with_SvoData(ray, g_svoData->getSerializedSvo());
      unsigned nodeIndex = 0;

      unsigned pixelIndex = (y*g_bufferWidth + x) * 3;

      if (nodeIndex)
      {

        unsigned attribIndex = g_svoData->getSerializedSvo()[nodeIndex].getAttribPosition() + 3;

        g_renderBuffer[pixelIndex++] = 1.0;
        g_renderBuffer[pixelIndex++] = 1.0;
        g_renderBuffer[pixelIndex++] = 1.0;
      }
      else
      {
        g_renderBuffer[pixelIndex++] = 0.2;
        g_renderBuffer[pixelIndex++] = 0.2;
        g_renderBuffer[pixelIndex++] = 0.25;
      }
    }

  }
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


      glColor4f(1.0f, 1.0f, 1.0f, 1.0);

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
          glColor4f(1.0f, 1.0f, 1.0f, 0.5);
          g_texter->renderTextLine("(h) Show text info:      " + gloost::toString( g_showTextInfo )) ;
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
      case 'R':
        reloadShaders();
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
//  glfwCreateThread( pollEvents, NULL);

  /// enable sync to vblank on linux to control the demo fps
#ifdef GLOOST_GNULINUX
  setenv("__GL_SYNC_TO_VBLANK","1",true);
#endif



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

