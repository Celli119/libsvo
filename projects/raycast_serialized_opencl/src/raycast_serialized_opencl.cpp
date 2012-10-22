/**

  Last change 30.04.24 by Felix Weißig

  Email: thesleeper@gmx.net
  skype: otacocato
  URL:   http://www.otaco.de

**/

/**


**/


////////////////////////////////////////////////////////////////////////////////


/// c++ includes
#include <GL/glew.h>
#include <GL/glfw.h>
#include <iostream>
#include <cmath>
#include <stdlib.h>


////////////////////////////////////////////////////////////////////////////////


/// general setup
static unsigned int g_screenWidth  = 1024;
static unsigned int g_screenHeight = 768;

static unsigned int g_bufferWidth   = g_screenWidth;
static unsigned int g_bufferHeight  = g_bufferHeight;



/// frameCounter
unsigned int g_frameCounter = 0;


/// includes and globals for this demo /////////////////////////////////////////

#include <gloost/gloostRenderGoodies.h>


#include <gloost/TextureText.h>
gloost::TextureText* g_texter = 0;
double g_timePerFrame         = 0;

std::string g_gloostFolder = "../../gloost/";
std::string g_dataPath     = "../data/";

#include <gloost/Matrix.h>
#include <gloost/Mouse.h>
gloost::Mouse g_mouse;

#include <gloost/PerspectiveCamera.h>
gloost::PerspectiveCamera*   g_camera                = 0;;
float                        g_tScaleRatio           = 1.0;
int                          g_tScaleRatioMultiplyer = 1.0;


gloost::Point3 g_modelOffset;
float          g_cameraRotateY  = 0;
float          g_cameraRotateX  = 0;
float          g_cameraDistance = 1.0;

// SVO
#include <Svo.h>
svo::Svo* g_svo = 0;
#include <SvoNode.h>

#include <gloost/InterleavedAttributes.h>
gloost::InterleavedAttributes* g_voxelAttributes = 0;
gloost::InterleavedAttributes* g_shadowBuffer    = 0;


// setup for rendering into frame buffer
#include <gloost/TextureManager.h>
#include <gloost/Texture.h>
unsigned g_framebufferTextureId = 0;

#include <gloost/gloostHelper.h>

// info
bool        g_showTextInfo     = true;
bool        g_showRayCastImage = true;
unsigned    g_viewMode         = 0;
std::string g_viewModeText     = "color";
bool        g_frameDirty       = true;
bool        g_raycastEveryFrame = false;


std::vector<gloost::Vector2> g_screenCoords;
std::vector<gloost::Ray>     g_cameraRays;


//bencl stuff
#include <CL/opencl.h>
#include <gloost/bencl/ocl.h>
#include <gloost/bencl/ClContext.h>

gloost::bencl::ClContext* g_context   = 0;
unsigned                  g_deviceGid = 0;

void init();
void initCl();
void draw2d();
void cleanup();
void mouseFunc(int button, int state, int mouse_h, int mouse_v);
void reloadShaders();
bool raycastIntoFrameBuffer(unsigned threadId);
void resize(int width, int height);
void key(unsigned char key, int x, int y);
void motionFunc(int mouse_h, int mouse_v);
void idle(void);



////////////////////////////////////////////////////////////////////////////////

#include <limits>

void init()
{
  g_bufferWidth        = g_screenWidth /4.0;
  g_bufferHeight       = g_screenHeight/4.0;

  // load svo
  const std::string svo_dir_path = "/home/otaco/Desktop/SVO_DATA/";

//  const std::string svoBaseName = "flunder_11";
//  const std::string svoBaseName = "david_2mm_final_ao_12";
//  const std::string svoBaseName = "lucy_11";
//  const std::string svoBaseName = "frog2_vertex_ao_8";
//  const std::string svoBaseName = "frog2_mean_7";
//  const std::string svoBaseName = "bridge_14";
//  const std::string svoBaseName = "conference2_11";
//  const std::string svoBaseName = "dragon_vrip_11";
//  const std::string svoBaseName = "Decimated_Head_high_11";
//  const std::string svoBaseName = "alligator_head_11";
//  const std::string svoBaseName = "anteater_1m_12";
  const std::string svoBaseName = "frog2_vertex_ao_7";

  // loading svo and attributes
  g_svo = new svo::Svo(svo_dir_path + svoBaseName + ".svo");

  const std::string attributesFileName = svo_dir_path + svoBaseName + "c.ia";
  std::cerr << std::endl << "Loading Attributes: " << attributesFileName;
  g_voxelAttributes = new gloost::InterleavedAttributes(attributesFileName);
  std::cerr << std::endl << "num packages: " << g_voxelAttributes->getNumPackages();
  std::cerr << std::endl << "num elements: " << g_voxelAttributes->getNumElementsPerPackage();
  std::cerr << " ... done.";

  // shadow buffer
  g_shadowBuffer = new gloost::InterleavedAttributes();
  g_shadowBuffer->addAttribute(1, 4, "shadow");
  g_shadowBuffer->resize(g_voxelAttributes->getNumPackages());
  g_shadowBuffer->fill(-1.0f);


  g_texter = new gloost::TextureText(g_gloostFolder + "/data/fonts/gloost_Fixedsys_16_gui.png");


  // setup framebuffer
  std::cerr << std::endl << "setting up framebuffer: ";
  gloost::Texture* texture = new gloost::Texture( g_bufferWidth,
                                                  g_bufferHeight,
                                                  1,
                                                  0,//(unsigned char*)&pixelData->getVector().front(),
                                                  16,
                                                  GL_TEXTURE_2D,
                                                  GL_RGBA,
                                                  GL_RGBA,
                                                  GL_FLOAT);

  g_framebufferTextureId = gloost::TextureManager::get()->addTexture(texture);

  gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->setTexParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->setTexParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->setTexParameter(GL_TEXTURE_BASE_LEVEL, 0);
  gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->setTexParameter(GL_TEXTURE_MAX_LEVEL, 0);
  gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->initInContext();

  g_camera = new gloost::PerspectiveCamera(65.0,
                                           (float)g_screenWidth/(float)g_screenHeight,
                                           0.01,
                                           20.0);

  float xmax = /*g_camera->getNear()**/  tan(g_camera->getFov() * gloost::PI / 360.0) * g_camera->getAspect();
  g_tScaleRatio = xmax / /*g_camera->getNear() /*/ (g_bufferWidth*0.5);
  std::cerr << std::endl << "g_tScaleRatio: " << g_tScaleRatio;


  initCl();
}


//////////////////////////////////////////////////////////////////////////////////////////


void initCl()
{

  // opencl init
  gloost::bencl::ocl::init();

  std::cerr << std::endl << "Platforms: " << gloost::bencl::ocl::getPlatformsAsString();

  g_context = new gloost::bencl::ClContext(1);

  // change Device here!
  g_deviceGid = 1;

  g_context->acquireDevice(g_deviceGid);
  g_context->createContextClFromGlContext();

  g_context->loadProgram("../opencl/fillFrameBuffer_rgba.cl");


  // assign render buffer
  gloost::gloostId  renderbufferGid = g_context->createClImage(CL_MEM_WRITE_ONLY,
                                                       gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->getTarget(),
                                                       gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->getTextureHandle());

  g_context->setKernelArgBuffer("renderToBuffer", 0, renderbufferGid);


  // assign svo data
  gloost::gloostId svoDataGid = g_context->createClBuffer(CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                                          (char*)&g_svo->getSerializedNodes().front(),
                                                          g_svo->getSerializedNodes().size()*sizeof(svo::CpuSvoNode));

  g_context->setKernelArgBuffer("renderToBuffer", 1, svoDataGid);

  // assign attribute indices
  gloost::gloostId attribIndicesDataGid = g_context->createClBuffer(CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                                          (char*)&g_svo->getSerializedAttributeIndices().front(),
                                                          g_svo->getSerializedNodes().size()*sizeof(unsigned));

  g_context->setKernelArgBuffer("renderToBuffer", 2, attribIndicesDataGid);


  // assign attrib data
  gloost::gloostId attribDataGid = g_context->createClBuffer(CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                                            (char*)g_voxelAttributes->getData(),
                                                            g_voxelAttributes->getNumPackages()*g_voxelAttributes->getPackageStride());

  g_context->setKernelArgBuffer("renderToBuffer", 3, attribDataGid);



}


/////////////////////////////////////////////////////////////////////////////////////////


  /// free all resources we created

void cleanup()
{

}

//////////////////////////////////////////////////////////////////////////////////////////


void frameStep()
{
  ++g_frameCounter;

  int x,y;
  glfwGetMousePos(&x,&y);

  g_mouse.setSpeedToZero();
  g_mouse.setLoc(x,g_screenHeight-y,0);

  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_1 ))
  {
    g_cameraRotateY += g_mouse.getSpeed()[0]*0.005;
    g_cameraRotateX += g_mouse.getSpeed()[1]*-0.005;

    g_cameraRotateX = gloost::clamp(g_cameraRotateX, -3.0f, 3.0f);

    g_frameDirty = true;
  }
  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_2 ))
  {
    g_cameraDistance += g_mouse.getSpeed()[1]*-0.005;
    g_cameraDistance = gloost::clamp(g_cameraDistance, 0.05f, 20.0f);

    g_frameDirty = true;
  }
  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_3 ))
  {
    gloost::Matrix viewMatrixInv = g_camera->getViewMatrix().inverted();
    viewMatrixInv.setTranslate(0.0,0.0,0.0);

    g_mouse.getSpeed()[2] = 0.0;

    gloost::Vector3 offsetAdd = viewMatrixInv*(g_mouse.getSpeed()*-0.0005);

    g_modelOffset += offsetAdd;

    g_frameDirty = true;
  }


  if (g_raycastEveryFrame || g_frameDirty)
  {
    g_camera->lookAt(gloost::Vector3(-sin(g_cameraRotateY),
                                   g_cameraRotateX,
                                   cos(g_cameraRotateY)).normalized() * g_cameraDistance,
                                   gloost::Point3(0.0f, 0.0f, 0.0f),
                                   gloost::Vector3(0.0f, 1.0f, 0.0f));


    // start raycasting
    const gloost::Frustum& frustum = g_camera->getFrustum();

    gloost::Vector3 frustumH_vec         = frustum.far_lower_right - frustum.far_lower_left;
    gloost::Vector3 frustumOnePixelWidth = frustumH_vec/g_bufferWidth;
    gloost::Vector3 frustumV_vec          = frustum.far_upper_left - frustum.far_lower_left;
    gloost::Vector3 frustumOnePixelHeight = frustumV_vec/g_bufferHeight;


    g_context->setKernelArgFloat4("renderToBuffer", 4, gloost::Vector3(g_bufferWidth, g_bufferHeight, g_tScaleRatioMultiplyer*g_tScaleRatio));
    g_context->setKernelArgFloat4("renderToBuffer", 5, frustumOnePixelWidth);
    g_context->setKernelArgFloat4("renderToBuffer", 6, frustumOnePixelHeight);
    g_context->setKernelArgFloat4("renderToBuffer", 7, frustum.far_lower_left);
    g_context->setKernelArgFloat4("renderToBuffer", 8, g_modelOffset + g_camera->getPosition());
    g_context->setKernelArgFloat4("renderToBuffer", 9, gloost::vec4(g_viewMode, 0.0,0.0,0.0));

    g_context->acquireGlObjects(g_deviceGid, "renderToBuffer");
    {
      g_context->enqueueKernel(g_deviceGid,
                               "renderToBuffer",
                               2,
                               gloost::Vector3(g_bufferWidth, g_bufferHeight, 1),
                               gloost::Vector3(8, 8, 0));
    }
    g_context->releaseGlObjects(g_deviceGid);
  }

  g_frameDirty = false;


}


//////////////////////////////////////////////////////////////////////////////////////////


///

void draw2d()
{
  /// set the viewport
  glViewport(0, 0, g_screenWidth, g_screenHeight);

  glClearColor(0.25, 0.25, 0.25, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

      if (g_frameCounter % 5 == 0)
      {
        avarageTimePerFrame = timeAccum / 5.0;
        timeAccum = 0;
      }



      glColor4f(1.0f, 1.0f, 1.0f, 1.0);

      // info text
      if (g_showTextInfo)
      {
        g_texter->begin();
        {
          g_texter->renderTextLine(20, g_screenHeight -20, "SVO raycast opencl");
          g_texter->renderFreeLine();
          g_texter->renderTextLine("frame counter:      " + gloost::toString(g_frameCounter));
          g_texter->renderTextLine("Time per frame:     " + gloost::toString(avarageTimePerFrame));
          g_texter->renderTextLine("fps:                " + gloost::toString(1.0/avarageTimePerFrame));
          g_texter->renderTextLine("view mode (1...5):  " + g_viewModeText);
          g_texter->renderFreeLine();
          glColor4f(1.0f, 1.0f, 1.0f, 0.5);
          g_texter->renderTextLine("(h) Show text info: " + gloost::toString( g_showTextInfo )) ;
          g_texter->renderFreeLine();
          g_texter->renderFreeLine();
          glColor4f(0.8f, 0.8f, 1.0f, 1.0);
          g_texter->renderTextLine("g_maxRayCastDepth:  " + gloost::toString(g_svo->getMaxDepth()));
          g_texter->renderTextLine("LOD multiplier:     " + gloost::toString(g_tScaleRatioMultiplyer));
        }
        g_texter->end();

      }

    }
    glPopAttrib();
  }
  glPopMatrix();
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

    g_frameDirty = true;

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


      case '0':
        g_viewMode = 0;
        g_viewModeText = "color";
        break;

      case '1':
        g_viewMode = 1;
        g_viewModeText = "normals";
        break;

      case '2':
        g_viewMode = 2;
        g_viewModeText = "svo depth";
        break;

      case '3':
        g_viewMode = 3;
        g_viewModeText = "ray t";
        break;

      case '4':
        g_viewMode = 4;
        g_viewModeText = "iterations";
        break;

      case '5':
        g_viewMode = 5;
        g_viewModeText = "diffuse + reflection + shadow";
        break;

      case '6':
        g_viewMode = 6;
        g_viewModeText = "shaded";
        break;

      case 'R':
        g_context->reloadProgram("../opencl/fillFrameBuffer_rgba.cl");
        break;

      case 'V':
        g_raycastEveryFrame = !g_raycastEveryFrame;
        break;


      case '+':
        ++g_tScaleRatioMultiplyer;
        break;
      case '-':
        --g_tScaleRatioMultiplyer;
        g_tScaleRatioMultiplyer = gloost::max(g_tScaleRatioMultiplyer, 1);
        break;

      case 'F':
        g_showRayCastImage = !g_showRayCastImage;
        break;

      case 'C':
        g_modelOffset = gloost::Vector3(0.0,0.0,0.0);
        std::cerr << std::endl << "centering object";
        std::cerr << std::endl;
        break;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[])
{

//
//  for (unsigned i=0; i!=100; ++i)
//  {
//    bool bla = bool(i&4)
//  }
//
//
//
//  exit(0);




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
//  glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 16);
//  glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);



  if( !glfwOpenWindow( g_screenWidth, g_screenHeight, 0,0,0,0,0,0, GLFW_WINDOW ) )
  {
    std::cerr << std::endl << "ERROR while calling glfwOpenWindow(...)";
    glfwTerminate();
    return EXIT_FAILURE;
  }


  glfwSetWindowPos( 160, 0);
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


  glfwSwapBuffers();

  /// load and intialize stuff for our demo
  init();


  // Main loop
  int running = GL_TRUE;
  while( running )
  {
    double startTime = glfwGetTime();

    // OpenGL rendering goes here...
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    frameStep();
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

