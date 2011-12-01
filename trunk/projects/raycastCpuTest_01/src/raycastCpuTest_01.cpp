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


gloost::Point3 g_modelOffset;
float          g_cameraRotateY  = 0;
float          g_cameraRotateX  = 0;
float          g_cameraDistance = 2.0;


gloost::Ray g_ray;
#include <GbmLoader.h>
#include <Mesh.h>
#include <Vbo.h>
gloost::Vbo* g_drawRayVbo  = 0;
gloost::Vbo* g_drawAxisVbo = 0;




// SVO
#include <Svo.h>
svo::Svo* g_svo = 0;
#include <SvoNode.h>

#include <SvoVisualizer.h>
svo::SvoVisualizer* g_svoVisualizerNodes     = 0;
svo::SvoVisualizer* g_svoVisualizerLeaves    = 0;
svo::SvoVisualizer* g_svoVisualizerTraversal = 0;
#include <CpuRaycasterSingleRay.h>
int g_maxRayCastDepth = 1;


#include <SvoBuilderVertices.h>


#include <Shader.h>
#include <UniformSet.h>
gloost::Shader*     g_SvoVisualizingShader = 0;

// info
bool g_showTextInfo     = true;
bool g_showOctreeNodes  = true;
bool g_showOctreeLeaves = false;

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
//#define LOAD_MESH_AS_TESTDATA

#define BUILD_SVO
#define BUILD_VISUALIZATION_NODES
#define BUILD_VISUALIZATION_LEAVES

  unsigned int maxDepth = 4;


  g_texter = new gloost::TextureText(g_gloostFolder + "/data/fonts/gloost_Fixedsys_16_gui.png");


  // shaders
  reloadShaders();


  // points to build an example svo

#ifdef LOAD_MESH_AS_TESTDATA
  gloost::GbmLoader gbmLoader("/home/otaco/Desktop/gbm/david_angel.gbm");
  gloost::Mesh* mesh = gbmLoader.getMesh();
  mesh->center();
  mesh->scaleToSize(1.0);
#else

  gloost::Mesh* mesh = new gloost::Mesh();
//  mesh->getVertices().push_back(gloost::Point3(0.25, 0.25, 0.25));
//  mesh->getVertices().push_back(gloost::Point3(0.25, 0.25, -0.25));
//  mesh->getVertices().push_back(gloost::Point3(0.25, -0.25, 0.25));
//  mesh->getVertices().push_back(gloost::Point3(0.25, -0.25, -0.25));
//  mesh->getVertices().push_back(gloost::Point3(-0.25, 0.25, 0.25));
//  mesh->getVertices().push_back(gloost::Point3(-0.25, 0.25, -0.25));
  mesh->getVertices().push_back(gloost::Point3(-0.3, -0.3, 0.2));
//  mesh->getVertices().push_back(gloost::Point3(-0.25, -0.25, -0.25));


//  mesh->getNormals().push_back(gloost::Vector3(0.25, 0.25, 0.25));
//  mesh->getNormals().push_back(gloost::Vector3(0.25, 0.25, -0.25));
//  mesh->getNormals().push_back(gloost::Vector3(0.25, -0.25, 0.25));
//  mesh->getNormals().push_back(gloost::Vector3(0.25, -0.25, -0.25));
//  mesh->getNormals().push_back(gloost::Vector3(-0.25, 0.25, 0.25));
//  mesh->getNormals().push_back(gloost::Vector3(-0.25, 0.25, -0.25));
  mesh->getNormals().push_back(gloost::Vector3(-0.25, -0.25, 0.25));
//  mesh->getNormals().push_back(gloost::Vector3(-0.25, -0.25, -0.25));


//  mesh->getColors().push_back(gloost::vec4(0.0,0.0,0.0,1.0));
//  mesh->getColors().push_back(gloost::vec4(0.0,0.0,1.0,1.0));
//  mesh->getColors().push_back(gloost::vec4(0.0,1.0,0.0,1.0));
//  mesh->getColors().push_back(gloost::vec4(0.0,1.0,1.0,1.0));
//  mesh->getColors().push_back(gloost::vec4(1.0,0.0,0.0,1.0));
//  mesh->getColors().push_back(gloost::vec4(1.0,0.0,1.0,1.0));
  mesh->getColors().push_back(gloost::vec4(1.0,1.0,0.0,1.0));
//  mesh->getColors().push_back(gloost::vec4(1.0,1.0,1.0,1.0));

#endif

  mesh->takeReference();


#ifdef BUILD_SVO
  buildSvo(mesh, maxDepth);
  buildSvoVisualization();
#endif



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
}


//////////////////////////////////////////////////////////////////////////////////////////


void initWithContext()
{
  g_camera = new gloost::PerspectiveCamera(50.0,
                                           (float)g_screenWidth/(float)g_screenHeight,
                                           0.01,
                                           20.0);

  g_camera->lookAt(gloost::Point3(0.0f, 1.0f, 3.0f),
                   gloost::Point3(0.0f, 0.0f, 0.0f),
                   gloost::Vector3(0.0f, 1.0f, 0.0f));
}


//////////////////////////////////////////////////////////////////////////////////////////


void buildSvo(gloost::Mesh* mesh, unsigned int maxSvoDepth)
{

  if (g_svo == 0)
  {
    g_svo = new svo::Svo(maxSvoDepth);
  }

  svo::SvoBuilderVertices fromVerticesBuilder;

  fromVerticesBuilder.build(g_svo, mesh);
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


  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_3 ))
  {

    g_ray = g_camera->getPickRay( g_screenWidth,
                                  g_screenHeight,
                                  x,
                                  g_screenHeight - y);
    g_ray.setT(1000.0);


    gloost::Vector3 sideVec = g_camera->getModelViewMatrix() * gloost::Vector3(0.1, 0.0, 0.0);
    gloost::Vector3 upVec   = cross(sideVec, g_ray.getDestination()).normalized() * 0.1;


    svo::CpuRaycasterSingleRay raycaster;
    raycaster._db_maxDepth = g_maxRayCastDepth;
    raycaster.start(g_ray, g_svo);

    if (g_svoVisualizerTraversal)
    {
      delete g_svoVisualizerTraversal;
    }

    g_svoVisualizerTraversal = raycaster.getVisualizer();


//    std::cerr << std::endl << "g_ray: " << g_ray;


    // ray visualization
    gloost::Mesh* lineMesh = new gloost::Mesh();

    lineMesh->getVertices().push_back(g_ray.getOrigin());
    lineMesh->getColors().push_back(gloost::vec4(1.0, 1.0, 0.0, 1.0));

    lineMesh->getVertices().push_back(g_ray.getDestination());
    lineMesh->getColors().push_back(gloost::vec4(1.0, 1.0, 1.0, 1.0));

    lineMesh->getLines().push_back(gloost::Line(0, 1));


    if (g_drawRayVbo)
    {
      g_drawRayVbo->dropReference();
    }


    g_drawRayVbo = new gloost::Vbo(lineMesh);
    g_drawRayVbo->takeReference();
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
    if (g_drawAxisVbo)
    {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glPushMatrix();
      {

        glLineWidth(3.0);

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

    // draw visualization traversal
    if (1 && g_svoVisualizerTraversal)
    {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glPushMatrix();
      {

        glLineWidth(2.0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        g_SvoVisualizingShader->set();
          g_svoVisualizerTraversal->draw();
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
          g_texter->renderTextLine("(3) Show octree nodes:   " + gloost::toString( g_showOctreeNodes )) ;
          g_texter->renderTextLine("(4) Show octree leaves:  " + gloost::toString( g_showOctreeLeaves )) ;
          g_texter->renderFreeLine();
          g_texter->renderFreeLine();
          glColor4f(0.8f, 0.8f, 1.0f, 1.0);
          g_texter->renderTextLine("g_maxRayCastDepth:       " + gloost::toString(g_maxRayCastDepth));
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
        ++g_maxRayCastDepth;
        break;
      case 'B':
        --g_maxRayCastDepth;
        g_maxRayCastDepth = gloost::clamp(g_maxRayCastDepth, 1, 1000);

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

  glfwSwapInterval( 1 );


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


