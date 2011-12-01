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
static unsigned int g_screenHeight = 640;

/// frameCounter
unsigned int g_frameCounter = 0;


/// includes and globals for this demo /////////////////////////////////////////


/// this header includes many nice shapes to draw and helpers

#include <gloostRenderGoodies.h>


#include <TextureText.h>
gloost::TextureText* g_texter = 0;
double g_timePerFrame = 0;

std::string g_gloostFolder = "../../gloost/";
std::string g_dataPath     = "../data/";

#include <Matrix.h>
#include <Mouse.h>
gloost::Mouse g_mouse;

#include <PerspectiveCamera.h>
gloost::PerspectiveCamera*   g_camera;

#include <PointLight.h>
gloost::PointLight* g_pointLight = 0;

gloost::Point3 g_modelOffset;
float          g_cameraRotateY  = 0;
float          g_cameraRotateX  = 0;
float          g_cameraDistance = 4.0;


gloost::Ray g_ray;
#include <GbmLoader.h>
#include <PlyLoader.h>
#include <Mesh.h>
#include <Vbo.h>


#include <Shader.h>
#include <UniformSet.h>
gloost::Shader*     g_modelShader      = 0;
gloost::UniformSet* g_modelUniforms    = 0;


#include <MengerSponge.h>
svo::MengerSponge* g_mengerSponge = 0;



std::vector<gloost::Point3> g_points;
int g_maxDepth = 0;



// info
bool g_showTextInfo     = true;



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

////////////////////////////////////////////////////////////////////////////////


  /// load and initialize resources for our demo

void init()
{

#define OPEN_GL_WINDOW
#define DRAW_MESH

  g_maxDepth = 5;


  g_texter = new gloost::TextureText(g_gloostFolder + "/data/fonts/gloost_Fixedsys_16_gui.png");


  // shaders
  reloadShaders();


  g_mengerSponge = new svo::MengerSponge(g_maxDepth);


//  g_mengerSponge->saveAsGbm("/home/otaco/Desktop/mengerSponge_" + gloost::toString(g_maxDepth) + ".gbm");





    /// create a light and set position and components
  g_pointLight = new gloost::PointLight();

  g_pointLight->setPosition(5.0f, 5.0f, 15.0f);
  g_pointLight->setAmbient(0.2f, 0.2f, 0.2f, 1.0f);
  g_pointLight->setDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
  g_pointLight->setSpecular(1.0f, 1.0f, 1.0f, 1.0f);

  g_modelUniforms = new gloost::UniformSet();
  g_modelUniforms->set_sampler2D("environmentMap",
                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/bensFrontyard_blured.jpg"));
//                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/probe.jpg"));
//                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/cedarCity.jpg"));
//                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/skies.jpg"));
//                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/uni-washington.jpg"));
//                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/christmas.jpg"));
//                                 gloost::TextureManager::getInstance()->createTexture(g_dataPath + "environments/scanner.jpg"));
  g_modelUniforms->set_float("reflection", 0.1);
  g_modelUniforms->set_float("shininess", 60.0);
  g_modelUniforms->set_float("specular",  0.05);

  g_camera = new gloost::PerspectiveCamera(50.0,
                                         (float)g_screenWidth/(float)g_screenHeight,
                                         0.01,
                                         20.0);

}


//////////////////////////////////////////////////////////////////////////////////////////


void frameStep()
{
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
    g_cameraDistance = gloost::clamp(g_cameraDistance, 0.25f, 4.0f);
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

  g_pointLight->set();


  g_camera->set();


  gloost::Matrix offset;
  offset.setIdentity();
  offset.setTranslate(g_modelOffset);



  // draw model
  glPushMatrix();
  {
    gloostMultMatrix(offset.data());

    // draw visualization leafes
    if (g_mengerSponge)
    {
      glPushAttrib(GL_ALL_ATTRIB_BITS);

//      glEnable(GL_CULL_FACE);
//      glCullFace(GL_BACK);

      glPushMatrix();
      {
        g_modelShader->set();
        g_modelUniforms->applyToShader(g_modelShader);

        g_mengerSponge->draw();

        g_modelShader->disable();
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
#ifdef LINUX
//  setenv("__GL_SYNC_TO_VBLANK","1",true);
#else
  /// SDL console output hack  (or SDL will write all output in a file)
  freopen( "CON", "w", stdout );
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

  /// cleanup all resources we created for the demo
  cleanup();


  return EXIT_SUCCESS;
}


