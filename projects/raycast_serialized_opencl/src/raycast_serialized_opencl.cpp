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
static unsigned int g_screenHeight = 1024;

static unsigned int g_bufferWidth   = g_screenWidth;
static unsigned int g_bufferHeight  = g_bufferHeight;



/// frameCounter
unsigned int g_frameCounter = 0;


/// includes and globals for this demo /////////////////////////////////////////

#include <gloost/gloostRenderGoodies.h>


#include <gloost/TextureText.h>
gloost::TextureText* g_texter       = 0;
double               g_timePerFrame = 0.0;

std::string g_gloostFolder = "../../gloost/";
std::string g_dataPath     = "../data/";

#include <gloost/Matrix.h>
#include <gloost/Mouse.h>
gloost::Mouse g_mouse;

#include <gloost/PerspectiveCamera.h>
float                        g_tScaleRatio           = 1.0f;
float                        g_tScaleRatioMultiplyer = 1.0f;


gloost::Point3 g_modelOffset;
float          g_cameraRotateY  = gloost::PI;
float          g_cameraRotateX  = 0.0f;
float          g_cameraDistance = 1.0f;

// SVO
#include <TreeletMemoryManagerCl.h>
svo::TreeletMemoryManagerCl* g_clMemoryManager         = 0;
#include <RenderPassAnalyse.h>
svo::RenderPassAnalyse*      g_renderPassAnalyse       = 0;
const float                  g_fbToAnalyseBufferDivide = 4.0f; // <---

#include <gloost/InterleavedAttributes.h>
gloost::InterleavedAttributes* g_voxelAttributes = 0;

#include <contrib/Timer.h>
#include <contrib/TimerLog.h>

// setup for rendering into frame buffer
#include <gloost/TextureManager.h>
#include <gloost/Texture.h>
unsigned g_framebufferTextureId = 0;

#include <gloost/gloostHelper.h>
#include <gloost/gloostGlUtil.h>

// info
bool        g_showTextInfo         = true;
bool        g_showRayCastImage     = true;
unsigned    g_viewMode             = 0u;
bool        g_frameDirty           = true;
bool        g_raycastEveryFrame    = true;
bool        g_enableDynamicLoading = false;
bool        g_interaction          = false;


bool g_do_benchmark = false;



//bencl stuff
#include <CL/opencl.h>
#include <gloost/bencl/ocl.h>
#include <gloost/bencl/ClContext.h>

gloost::bencl::ClContext* g_context    = 0;
gloost::gloostId          g_contextGid = 0; // <- will be reset in initCl
gloost::gloostId          g_deviceGid  = 0; // <- will be reset in initCl

void init();
void initCl();
void draw2d();
void cleanup();
void mouseFunc(int button, int state, int mouse_h, int mouse_v);
void reloadShaders();
bool raycastIntoFrameBuffer(unsigned threadId);
void resize(int width, int height);
void key(int key, int state);
void motionFunc(int mouse_h, int mouse_v);
void idle(void);
void showTreeletCounters();

std::string g_svoBaseName = "";


struct virtualEye
{
  virtualEye(unsigned screenWidth, unsigned screenHeight):
    _camPos(0.0f,0.0f, 1.0f),
    _rotation(0.0f, gloost::PI, 0.0f),
    _camSpeed(0.0f, 0.0f, 0.0f),
    _camera(65.0,
           (float)screenWidth/(float)screenHeight,
           0.01,
           20.0)
  {

  }

  void move (bool                   mouseState,
             const gloost::Vector3& mouseSpeed,
             bool key_up,    // w
             bool key_left,  // a
             bool key_down,  // s
             bool key_right, // d
             bool key_jump,  // space
             bool key_prone, //
             bool key_turbo = false,
             bool key_slow  = false)
  {


    // look around
    if (mouseState)
    {
      std::cerr << std::endl << "_rotation: " << _rotation;

      _camSpeed[0] += mouseSpeed[1]*-0.005;
      _camSpeed[1] += mouseSpeed[0]*0.005;

      _camSpeed[0] = gloost::clamp(_camSpeed[0], -3.4f, 3.4f);
    }

    static const gloost::Vector3 upVector    (0.0f, 1.0f, 0.0f);
    static const gloost::Vector3 frontVector (0.0f, 0.0f, -1.0f);
    static const gloost::Vector3 strafeVector(1.0f, 0.0f, 0.0f);

    _camSpeed *= 0.8;

    gloost::Point3 camDir = _camPos + gloost::Vector3(-sin(_rotation[1]),
                                                      -_camSpeed[0],
                                                      cos(_camSpeed[1])).normalized() * g_cameraDistance;

    _camera.lookAt(_camPos, camDir, gloost::Vector3(0.0f, 1.0f, 0.0f));


    gloost::Vector3 camSpaceSpeed(0.0f, 0.0f, 0.0f);
    float speedAdd = 0.03f*g_timePerFrame;

    if (key_turbo)
    {
      speedAdd *= 3.0f;
    }
    else if(key_slow)
    {
      speedAdd *= 0.25;
    }

    if (key_up)
    {
      camSpaceSpeed +=  frontVector * speedAdd;
    }
    else if (key_down)
    {
      camSpaceSpeed +=  frontVector * -speedAdd;
    }

    if (key_left)
    {
      camSpaceSpeed +=  strafeVector * -speedAdd;
    }
    else if (key_right)
    {
      camSpaceSpeed +=  strafeVector * speedAdd;
    }

    // up and down
    if (glfwGetKey(' '))
    {
      _camSpeed +=  upVector * speedAdd;
    }
    else if (glfwGetKey(GLFW_KEY_LSHIFT ))
    {
      _camSpeed +=  upVector * -speedAdd;
    }

    _camSpeed += _camera.getViewMatrix().inverted() * camSpaceSpeed;
    _camPos   += _camSpeed;


  }

  gloost::Vector3           _camPos;
  gloost::Vector3           _camSpeed;

  gloost::Vector3           _rotation;
  gloost::PerspectiveCamera _camera;

};

virtualEye g_eye(g_bufferWidth, g_bufferHeight);


////////////////////////////////////////////////////////////////////////////////

#include <limits>

void init()
{
  const unsigned screenDivide           = 4;
  const unsigned incoreBufferSizeInByte = 256/*MB*/ * 1024 * 1024;

  g_bufferWidth  = g_screenWidth  / (float)screenDivide;
  g_bufferHeight = g_screenHeight / (float)screenDivide;

  // load svo
//  const std::string svo_dir_path = "/home/fweissig/Desktop/SVO_DATA/";
  const std::string svo_dir_path = "/home/otaco/Desktop/SVO_DATA/";


  g_svoBaseName  = "TreeletBuildManager_out";
//  g_svoBaseName  = "david_face_s1_D13";
//  g_svoBaseName  = "david_face_s4_D13";

//  g_svoBaseName  = "lucy_s1_D13";
//  g_svoBaseName  = "lucy_s4_D13";

//  g_svoBaseName  = "xyzrgb_statuette_s1_D13";
//  g_svoBaseName  = "xyzrgb_statuette_s4_D13";


  g_texter = new gloost::TextureText(g_gloostFolder + "/data/fonts/gloost_Fixedsys_16_gui.png");


  // setup framebuffer
  std::cerr << std::endl;
  std::cerr << std::endl << "Setting up framebuffer: ";
  std::cerr << std::endl << "  width:  " << g_bufferWidth;
  std::cerr << std::endl << "  height: " << g_bufferHeight;
  std::cerr << std::endl;

  gloost::Texture* texture = new gloost::Texture( g_bufferWidth,
                                                  g_bufferHeight,
                                                  1,
                                                  0,//(unsigned char*)&pixelData->getVector().front(),
                                                  16,
                                                  GL_TEXTURE_2D,
                                                  GL_RGBA16F,
                                                  GL_RGBA,
                                                  GL_FLOAT);

  g_framebufferTextureId = gloost::TextureManager::get()->addTexture(texture);

  texture->setTexParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  texture->setTexParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  texture->setTexParameter(GL_TEXTURE_BASE_LEVEL, 0);
  texture->setTexParameter(GL_TEXTURE_MAX_LEVEL, 0);
  texture->initInContext();

//  g_camera = new gloost::PerspectiveCamera(65.0,
//                                           (float)g_screenWidth/(float)g_screenHeight,
//                                           0.01,
//                                           20.0);

  float xmax    = /*g_camera->getNear()**/  tan(g_eye._camera.getFov() * gloost::PI / 360.0f) * g_eye._camera.getAspect();
  g_tScaleRatio = xmax / /*g_camera->getNear() /*/ (g_bufferWidth*0.5);
  std::cerr << std::endl << "g_tScaleRatio: " << g_tScaleRatio;


  // init cl stuff
  initCl();


  g_clMemoryManager = new svo::TreeletMemoryManagerCl(svo_dir_path + g_svoBaseName,
                                                      incoreBufferSizeInByte,
                                                      g_context);

  // creates the incore ClBuffer
  g_clMemoryManager->initClBuffer();

  // assign incore buffer to kernel argument
  g_context->setKernelArgBuffer("renderToBuffer", 1, g_clMemoryManager->getClIncoreBufferGid());
  g_context->setKernelArgBuffer("renderToBuffer", 2, g_clMemoryManager->getClAttributeIncoreBufferGid());


  // analyse pass init
  g_renderPassAnalyse = new svo::RenderPassAnalyse(g_clMemoryManager,
                                                   g_bufferWidth/g_fbToAnalyseBufferDivide,
                                                   g_bufferHeight/g_fbToAnalyseBufferDivide);


  gloostTest::TimerLog::get()->setNumSamples(10);
}


//////////////////////////////////////////////////////////////////////////////////////////


void initCl()
{
  // change Device here!
  g_deviceGid = 0;

  // change context here
  g_contextGid = 0;

  // opencl init
  gloost::bencl::ocl::init();

  std::cerr << std::endl << gloost::bencl::ocl::getPlatformsAsString();

  g_context = new gloost::bencl::ClContext(g_contextGid);


  g_context->createDeviceInstance(g_deviceGid);
  g_context->createContextClFromGlContext();

  g_context->loadProgram("../opencl/treeletRenderer_rgba.cl");


  // assign render buffer
  gloost::Texture* frameBufferTexture = gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId);
  gloost::gloostId  renderbufferGid   = g_context->createClImage(CL_MEM_WRITE_ONLY,
                                                                 frameBufferTexture->getTarget(),
                                                                 frameBufferTexture->getTextureHandle());

  g_context->setKernelArgBuffer("renderToBuffer", 0, renderbufferGid);
}


/////////////////////////////////////////////////////////////////////////////////////////


  // free all resources

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


  g_eye.move(glfwGetMouseButton( GLFW_MOUSE_BUTTON_1 ),
             g_mouse.getSpeed(),
             glfwGetKey('W'),
             glfwGetKey('A'),
             glfwGetKey('S'),
             glfwGetKey('D'),
             glfwGetKey(' '),
             glfwGetKey(GLFW_KEY_LSHIFT ),
             glfwGetKey('V'),
             glfwGetKey(GLFW_KEY_LCTRL ));


#if 0
  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_1 ))
  {
    g_cameraRotateY += g_mouse.getSpeed()[0]*0.005;
    g_cameraRotateX += g_mouse.getSpeed()[1]*-0.005;

    g_cameraRotateX = gloost::clamp(g_cameraRotateX, -3.0f, 3.0f);

    g_frameDirty = true;
  }


  static const gloost::Vector3 upVector    (0.0f, 1.0f, 0.0f);
  static const gloost::Vector3 frontVector (0.0f, 0.0f, -1.0f);
  static const gloost::Vector3 strafeVector(1.0f, 0.0f, 0.0f);


  static gloost::Vector3 camSpeed(0.0f, 0.0f, 0.0f);
  static gloost::Point3  camPos = gloost::Vector3(0.0f,0.0f, 1.0f);
  camSpeed *= 0.8;

  gloost::Point3 camDir = camPos + gloost::Vector3(-sin(g_cameraRotateY),
                                                   -g_cameraRotateX,
                                                   cos(g_cameraRotateY)).normalized() * g_cameraDistance;


  g_camera->lookAt(camPos, camDir, gloost::Vector3(0.0f, 1.0f, 0.0f));


  gloost::Vector3 camSpaceSpeed(0.0f, 0.0f, 0.0f);
  float speedAdd = 0.03f*g_timePerFrame;

  if (glfwGetKey('V'))
  {
    speedAdd *= 3.0f;
  }
  else if(glfwGetKey(GLFW_KEY_LCTRL ))
  {
    speedAdd *= 0.25;
  }

  if (glfwGetKey('W' ))
  {
    camSpaceSpeed +=  frontVector * speedAdd;
  }
  else if (glfwGetKey('S' ))
  {
    camSpaceSpeed +=  frontVector * -speedAdd;
  }

  if (glfwGetKey('A' ))
  {
    camSpaceSpeed +=  strafeVector * -speedAdd;
  }
  else if (glfwGetKey('D' ))
  {
    camSpaceSpeed +=  strafeVector * speedAdd;
  }

  // up and down
  if (glfwGetKey(' '))
  {
    camSpeed +=  upVector * speedAdd;
  }
  else if (glfwGetKey(GLFW_KEY_LSHIFT ))
  {
    camSpeed +=  upVector * -speedAdd;
  }

  camSpeed += g_camera->getViewMatrix().inverted() * camSpaceSpeed;
  camPos +=camSpeed;
#endif


  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_3 ))
  {
    gloost::Matrix viewMatrixInv = g_eye._camera.getViewMatrix().inverted();
    viewMatrixInv.setTranslate(0.0,0.0,0.0);

    g_mouse.getSpeed()[2] = 0.0;

    gloost::Vector3 offsetAdd = viewMatrixInv*(g_mouse.getSpeed()*-0.0005);

    g_modelOffset += offsetAdd;

    g_frameDirty = true;
  }


  gloost::Matrix modelMatrix = gloost::Matrix::createTransMatrix(g_modelOffset);


 //////////////////////////////////////////////////////////////////////////////

#if 0
  // benchmark
  static bool   benchmarkInit                = true;
  static float  benchmark_camSpeed           = 0.01f;
  static int    benchmark_time               = 30000;
  static double benchmark_timeBetweenSamples = 250.0;

  static unsigned benchmark_stepCounter   = 0u;
  static unsigned benchmark_sampleCounter = 0u;
  static std::map<std::string, std::vector<double> > sampleAccumMap;
  static double lastSampleTime            = 0;


  static gloostTest::Timer benchmarkTime;

  if (g_do_benchmark)
  {

    static gloost::Point3    camerPos;


    if (benchmarkInit)
    {
      benchmarkTime = gloostTest::Timer();
      benchmarkTime.start();
      lastSampleTime = 0;

      sampleAccumMap = std::map<std::string, std::vector<double> >();

      benchmarkInit          = false;
      g_enableDynamicLoading = true;
      g_showRayCastImage     = false;
      g_showTextInfo         = false;
    }

    double time = benchmarkTime.getDurationInMilliseconds();

    g_camera->lookAt(gloost::Point3(sin(time*benchmark_camSpeed)*0.75f,
                                    0.0,
                                    cos(time*benchmark_camSpeed))*0.75f,
                     gloost::Point3(0.0,0.0,0.0),
                     gloost::Vector3(0.0,1.0,0.0));


    ++benchmark_stepCounter;

    if (benchmarkTime.getDurationInMilliseconds() - lastSampleTime > benchmark_timeBetweenSamples)
    {
      lastSampleTime = benchmarkTime.getDurationInMilliseconds();


      std::cerr << std::endl << "sample at: " << benchmarkTime.getDurationInMilliseconds()/1000.0;

      gloostTest::TimerLog::sampleMap::iterator timerIt    = gloostTest::TimerLog::get()->getTimes().begin();
      gloostTest::TimerLog::sampleMap::iterator timerEndIt = gloostTest::TimerLog::get()->getTimes().end();

      ++benchmark_sampleCounter;

      for (; timerIt!=timerEndIt; ++timerIt)
      {
//        std::cerr << std::endl << "timerIt->first: " << timerIt->first;
        sampleAccumMap[timerIt->first].push_back(gloostTest::TimerLog::get()->getCurrentAverage(timerIt->first));
      }

//      // reset sample timer
//      sampleTimer = gloostTest::Timer();
//      sampleTimer.start();
    }


    // end benchmark
    if (benchmarkTime.getDurationInMilliseconds() > benchmark_time)
    {
      std::map<std::string, std::vector<double> >::iterator timesIt    = sampleAccumMap.begin();
      std::map<std::string, std::vector<double> >::iterator timesEndIt = sampleAccumMap.end();


      std::cerr << std::endl << "sampleAccumMap: " << sampleAccumMap.size();

      gloost::BinaryFile csvOutFile;
      csvOutFile.openToWrite( g_svoBaseName + ".csv" );


      csvOutFile.writeString(g_svoBaseName);
      csvOutFile.writeChar8('\r');
      csvOutFile.writeString("Samples:; " + gloost::toString(benchmark_sampleCounter));
      csvOutFile.writeChar8('\r');

      std::map<std::string, double> averages;


      for (; timesIt!=timesEndIt; ++timesIt)
      {
        std::vector<double>& timesContainer = timesIt->second;
        std::cerr << std::endl;
        std::cerr << std::endl << "Name:;    " << timesIt->first << std::endl;
        std::cerr << std::endl << "samples:; " << timesContainer.size();
        std::cerr << std::endl;

        csvOutFile.writeChar8('\r');
        csvOutFile.writeString(timesIt->first+";");

        for (unsigned i=0; i!=timesContainer.size(); ++i)
        {
          std::cerr <<  timesContainer[i] << ";";
          csvOutFile.writeString(gloost::toString(timesContainer[i]) + ";");

          averages[timesIt->first] += timesContainer[i];
        }

        averages[timesIt->first] /= benchmark_sampleCounter;
      }



      csvOutFile.writeChar8('\r');
      csvOutFile.writeChar8('\r');
      csvOutFile.writeString("Averages:");
      csvOutFile.writeChar8('\r');

      // write averages
      std::map<std::string, double>::iterator averagesIt    = averages.begin();
      std::map<std::string, double>::iterator averagesEndIt = averages.end();


      for (; averagesIt!=averagesEndIt; ++averagesIt)
      {
        csvOutFile.writeChar8('\r');
        csvOutFile.writeString(averagesIt->first+"_av;");
        csvOutFile.writeString(gloost::toString(averagesIt->second) + ";");
      }




      csvOutFile.close();


      g_enableDynamicLoading = false;
      g_showRayCastImage     = true;

      g_do_benchmark = false;
      benchmarkInit  = true;
      benchmark_stepCounter   = 0u;
      benchmark_sampleCounter = 0u;
      g_showTextInfo = true;
    }



  }
  else
  {
    g_do_benchmark = false;
    benchmarkInit  = true;
  }
#endif


 //////////////////////////////////////////////////////////////////////////////



  // start raycasting
  const gloost::Frustum& frustum = g_eye._camera.getFrustum();

  gloost::Vector3 frustumH_vec          = frustum.far_lower_right - frustum.far_lower_left;
  gloost::Vector3 frustumOnePixelWidth  = frustumH_vec/g_bufferWidth;
  gloost::Vector3 frustumV_vec          = frustum.far_upper_left - frustum.far_lower_left;
  gloost::Vector3 frustumOnePixelHeight = frustumV_vec/g_bufferHeight;




  if (g_enableDynamicLoading)
  {
//    g_enableDynamicLoading = false;
#if 1
    g_renderPassAnalyse->performAnalysePass(g_deviceGid,
                                            &g_eye._camera,
                                            modelMatrix,
                                            g_tScaleRatio*g_tScaleRatioMultiplyer,
                                            frustumOnePixelWidth,
                                            frustumOnePixelHeight,
                                            g_fbToAnalyseBufferDivide);


    g_clMemoryManager->updateClientSideIncoreBuffer(g_renderPassAnalyse);

  }
  else
  {

    g_renderPassAnalyse->performAnalysePass(g_deviceGid,
                                            &g_eye._camera,
                                            modelMatrix,
                                            g_tScaleRatio*g_tScaleRatioMultiplyer,
                                            frustumOnePixelWidth,
                                            frustumOnePixelHeight,
                                            g_fbToAnalyseBufferDivide,
                                            true);
  }
#endif

  // timer
  gloostTest::Timer timerUpdateDevice;
  timerUpdateDevice.start();

  // update incore buffer
  g_clMemoryManager->updateDeviceMemory();

  // /timer
  timerUpdateDevice.stop();
  gloostTest::TimerLog::get()->putSample("memory_manager.update_incore_buffer_server", timerUpdateDevice.getDurationInMicroseconds()/1000.0);


  // timer
  gloostTest::Timer timerFillBuffer;
  timerFillBuffer.start();

  if (g_showRayCastImage)
  {
    g_context->setKernelArgFloat4("renderToBuffer", 3, gloost::Vector3(g_bufferWidth, g_bufferHeight, g_tScaleRatioMultiplyer*g_tScaleRatio));
    g_context->setKernelArgFloat4("renderToBuffer", 4, frustumOnePixelWidth);
    g_context->setKernelArgFloat4("renderToBuffer", 5, frustumOnePixelHeight);
    g_context->setKernelArgFloat4("renderToBuffer", 6, frustum.far_lower_left);
    g_context->setKernelArgFloat4("renderToBuffer", 7, modelMatrix * g_eye._camera.getPosition());
    g_context->setKernelArgMat4x4("renderToBuffer", 8, gloost::modelViewMatrixToNormalMatrix(g_eye._camera.getViewMatrix()));
    g_context->setKernelArgFloat4("renderToBuffer", 9, gloost::vec4(g_viewMode, 0.0,0.0,0.0));

    static bool flipFlop = false;
    flipFlop = !flipFlop;

    g_context->acquireGlObjects(g_deviceGid, "renderToBuffer");
    {
      g_context->enqueueKernel(g_deviceGid,
                               "renderToBuffer",
                               2u,
                               gloost::Vector3(g_bufferWidth, g_bufferHeight, 1),
                               true,
                               gloost::Vector3(8, 8, 0));
    }
    g_context->releaseGlObjects(g_deviceGid);

  }
  // /timer
  timerFillBuffer.stop();
  gloostTest::TimerLog::get()->putSample("render.enqueueKernel", timerFillBuffer.getDurationInMicroseconds()/1000.0);




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
      else
      {
        gloost::Texture* frameBufferTexture = gloost::TextureManager::get()->getTextureWithoutRefcount(g_renderPassAnalyse->getTestFrameBufferGid());
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

        //
        float weight = (g_renderPassAnalyse->getVisibleNewTreeletsGids().size()/1000.0f);
        static float maxWeight = 0.0f;

        glPushMatrix();
        {

          maxWeight = gloost::max(maxWeight, weight);
          glColor4f(0.1, 0.1, 0.2f, 0.4f);
          glTranslatef(0.0f, 0.0f, 0.0f);
          glScalef(maxWeight*1000.0f + 10, 30, 1.0);
          gloost::drawQuad();

          maxWeight *= 0.99;
        }
        glPopMatrix();


        glPushMatrix();
        {
          glColor4f(weight, 1.0f-weight, 0.0f, 1.0f);
          glTranslatef(0.0f, 0.0f, 0.0f);
          glScalef(weight*1000.0f + 10, 30, 1.0);
          gloost::drawQuad();
        }
        glPopMatrix();







        g_texter->begin();
        {
          glColor4f(1.0f, 1.0f, 1.0f, 1.0);
          g_texter->renderTextLine(20, g_screenHeight -20, "SVO raycasting opencl");
          g_texter->renderFreeLine();
          g_texter->renderTextLine("Time per frame:     " + gloost::toString(avarageTimePerFrame));
          g_texter->renderTextLine("fps:                " + gloost::toString(1.0/avarageTimePerFrame));
          g_texter->renderTextLine("view mode (1...7):  " + gloost::toString(g_viewMode));
          g_texter->renderFreeLine();
//          glColor4f(1.0f, 1.0f, 1.0f, 0.5);
//          g_texter->renderTextLine("(h) Show text info: " + gloost::toString( g_showTextInfo )) ;
//          g_texter->renderFreeLine();
          glColor4f(0.8f, 0.8f, 1.0f, 1.0);
          g_texter->renderTextLine("LOD multiplier:     " + gloost::toString(g_tScaleRatioMultiplyer));
          g_texter->renderFreeLine();

          glColor4f(0.8f, 1.0f, 0.8f, 1.0);
          g_texter->renderTextLine("new treelets visible:  " + gloost::toString(g_renderPassAnalyse->getVisibleNewTreeletsGids().size()));
          g_texter->renderTextLine("old treelets visible:  " + gloost::toString(g_renderPassAnalyse->getVisibleOldTreeletsGids().size()));

          g_texter->renderFreeLine();
          glColor4f(0.9f, 0.9f, 1.0f, 1.0);
          if (!g_clMemoryManager->getFreeIncoreSlotStack().size())
          {
            glColor4f(1.0f, 0.0f, 0.0f, 1.0);
          }

          unsigned freeIncoreSlots = g_clMemoryManager->getFreeIncoreSlotStack().size();
          float freeIncoreSlotsMb  = ((float)freeIncoreSlots * (float)sizeof(svo::CpuSvoNode)) / 1024.0f / 1024.0f;

          g_texter->renderTextLine("free incore slots:  " + gloost::toString(freeIncoreSlots) + " (" + gloost::toString(freeIncoreSlotsMb) + "MB)");
          g_texter->renderTextLine("slots to upload:    " + gloost::toString(g_renderPassAnalyse->getVisibleNewTreeletsGids().size()));






          // timer names
          glColor4f(0.9f, 0.9f, 0.0f, 1.0);
          g_texter->setWritePosition(5, 220);
          gloostTest::TimerLog::sampleMap::iterator timerIt    = gloostTest::TimerLog::get()->getTimes().begin();
          gloostTest::TimerLog::sampleMap::iterator timerEndIt = gloostTest::TimerLog::get()->getTimes().end();

          while (timerIt != timerEndIt)
          {
            g_texter->renderTextLine(timerIt->first + std::string(": "));
            ++timerIt;
          }
          g_texter->renderTextLine("everything");

          // timer current values
          glColor4f(1.0f, 1.0f, 0.0f, 1.0);
          g_texter->setWritePosition(385, 220);
          timerIt = gloostTest::TimerLog::get()->getTimes().begin();
          double sum = 0;
          while (timerIt != timerEndIt)
          {
            g_texter->renderTextLine(gloost::toString(gloostTest::TimerLog::get()->getCurrentAverage(timerIt->first) ));
            sum += gloostTest::TimerLog::get()->getCurrentAverage(timerIt->first);
            ++timerIt;
          }
          g_texter->renderTextLine(gloost::toString(sum));

          // timer current values
          glColor4f(1.0f, 1.0f, 0.0f, 1.0);
          g_texter->setWritePosition(485, 220);
          timerIt = gloostTest::TimerLog::get()->getTimes().begin();
          double sumLast = 0;
          while (timerIt != timerEndIt)
          {
            g_texter->renderTextLine(gloost::toString(gloostTest::TimerLog::get()->getLastAverage(timerIt->first) ));
            sumLast += gloostTest::TimerLog::get()->getLastAverage(timerIt->first);
            ++timerIt;
          }
          g_texter->renderTextLine(gloost::toString(sumLast));
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

  //
  g_frameDirty = true;


  /// update the global screen size
  g_screenWidth  = width;
  g_screenHeight = height;

  /// recalc the aspect ratio
  const float ar = (float) width / (float) height;


  g_eye._camera.setAspect(ar);

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

      case 'M':
        {
          std::cerr << std::endl << "g_clMemoryManager: ";
          g_clMemoryManager->removeTreeletFromIncoreBuffer(1);
        }
        break;

      case 'T':
        {
          g_do_benchmark = true;
        }
        break;


      case 'B':
        {
          g_enableDynamicLoading = !g_enableDynamicLoading;
          std::cerr << std::endl << "g_enableDynamicLoading: " << g_enableDynamicLoading;
        }
        break;

      case 'H':
        g_showTextInfo = !g_showTextInfo;
        break;

      case 'L':
        g_clMemoryManager->resetIncoreBuffer();;
        break;


      case '0':
        g_viewMode = 0;
//        g_viewModeText = "color";
        break;

      case '1':
        g_viewMode = 1;
//        g_viewModeText = "normals";
        break;

      case '2':
        g_viewMode = 2;
//        g_viewModeText = "svo depth";
        break;

      case '3':
        g_viewMode = 3;
//        g_viewModeText = "ray t";
        break;

      case '4':
        g_viewMode = 4;
//        g_viewModeText = "iterations";
        break;

      case '5':
        g_viewMode = 5;
//        g_viewModeText = "diffuse + reflection + shadow";
        break;

      case '6':
        g_viewMode = 6;
//        g_viewModeText = "shaded";
        break;

      case '7':
        g_viewMode = 7;
//        g_viewModeText = "quality to wavelength";
        break;

      case 'R':
        g_context->reloadProgram("../opencl/treeletRenderer_rgba.cl");
        g_frameDirty = true;
        break;

      case '+':
        g_tScaleRatioMultiplyer*=2.0f;
        break;
      case '-':
        g_tScaleRatioMultiplyer *=0.5;
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


  glfwSetWindowPos( 500, 0);
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


//  glfwSwapBuffers();

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

