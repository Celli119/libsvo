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
gloost::TextureText* g_texter = 0;
double g_timePerFrame = 0;

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

#include <CpuRaycasterSingleRay3.h>

#include <gloost/InterleavedAttributes.h>
gloost::InterleavedAttributes* g_voxelAttributes = 0;
gloost::InterleavedAttributes* g_shadowBuffer    = 0;
gloost::InterleavedAttributes* g_shadowSampleCountBuffer    = 0;

#include <gloost/Shader.h>
#include <gloost/UniformSet.h>
gloost::Shader*  g_SvoVisualizingShader = 0;


// setup for rendering into frame buffer
#include <gloost/TextureManager.h>
#include <gloost/Texture.h>

unsigned g_framebufferTextureId = 0;
#include <gloost/gloostHelper.h>


#include <boost/thread.hpp>


// info
bool        g_showTextInfo     = true;
bool        g_showRayCastImage = true;
unsigned    g_viewMode         = 0;
std::string g_viewModeText     = "color";


std::vector<gloost::Vector2> g_screenCoords;
std::vector<gloost::Ray>     g_cameraRays;
std::vector<gloost::Vector3> g_randomShadowRayOffsets;
std::vector<gloost::Vector3> g_randomAORays;


float* g_renderBuffer = 0;

boost::thread_group g_threadGroup;
bool                g_toggle_run_raycasting = true;
boost::mutex        g_bufferAccessMutex;

bool g_allJobsDone = false;
bool g_jobsReset   = true;


bool     g_toggle_renderer    = false;
unsigned g_num_render_Threads = 0;
unsigned g_num_jobsPerThread  = 1;
unsigned g_linesPerJob        = 2;
bool     g_join_threads       = true;
unsigned g_numShadowRays      = 0;


std::vector<svo::CpuRaycasterSingleRay3> g_raycasters;


void init();
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


  #define USE_THREADED_RENDERING
  g_bufferWidth        = g_screenWidth/4.0;
  g_bufferHeight       = g_screenHeight/4.0;
//  g_bufferWidth        = 1920;
//  g_bufferHeight       = 1200;
  g_num_render_Threads = 4;
  g_num_jobsPerThread  = 2;
  g_linesPerJob        = 4; // power of 2
  g_join_threads       = true;


  g_numShadowRays      = 2;


  g_raycasters.resize(g_num_render_Threads, svo::CpuRaycasterSingleRay3());



  // load svo
  const std::string svo_dir_path = "/home/otaco/Desktop/SVO_DATA/";

//  const std::string svoBaseName = "dragon_vrip_9";
//  const std::string svoBaseName = "frog2_seperated_11";
  const std::string svoBaseName = "david_2mm_final_ao_12";
//  const std::string svoBaseName = "dental_scan_11";
//  const std::string svoBaseName = "wacky_planet_11";
//  const std::string svoBaseName = "Decimated_Head_11";
//  const std::string svoBaseName = "Decimated_Head_8";
//  const std::string svoBaseName = "frog_landscape_11";
//  const std::string svoBaseName = "women_11";
//  const std::string svoBaseName = "frog_anglerfish_11";
//  const std::string svoBaseName = "fancy_art_high_11";
//  const std::string svoBaseName = "malaysia_11";
//  const std::string svoBaseName = "victoria-standing2_11";
//  const std::string svoBaseName = "hammer_11";
//  const std::string svoBaseName = "fancy_art_floor_11";
//  const std::string svoBaseName = "flunder_11";
//  const std::string svoBaseName = "terrain_05_11";
//  const std::string svoBaseName = "ring_11";
//  const std::string svoBaseName = "frog_anglerfish_11";




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
  g_shadowBuffer->fill(0.0f);

  g_shadowSampleCountBuffer = new gloost::InterleavedAttributes();
  g_shadowSampleCountBuffer->addAttribute(1, 4, "shadowSampleCount");
  g_shadowSampleCountBuffer->resize(g_voxelAttributes->getNumPackages());
  g_shadowSampleCountBuffer->fill(0);


  // create screencoords
  g_screenCoords = std::vector<gloost::Vector2>(g_bufferWidth*g_bufferHeight);

  for (unsigned y=0; y!=g_bufferHeight; ++y)
    for (unsigned x=0; x!=g_bufferWidth; ++x)
    {
      unsigned index = y*g_bufferWidth +x;
      g_screenCoords[index] = gloost::Vector2(x,y);
    }

  // mix screen coords
  for (unsigned i=0; i!=g_screenCoords.size()*4; ++i)
  {
    unsigned index1 = (unsigned)(gloost::frand()*g_screenCoords.size());
    unsigned index2 = (unsigned)(gloost::frand()*g_screenCoords.size());

    gloost::Vector2 tmp    = g_screenCoords[index1];
    g_screenCoords[index1] = g_screenCoords[index2];
    g_screenCoords[index2] = tmp;
  }


  // create random shadow ray directions
  for (unsigned i=0; i!=1024; ++i)
  {
    g_randomShadowRayOffsets.push_back(gloost::Vector3(gloost::crand(),
                                                       gloost::crand(),
                                                       gloost::crand())*0.035);
  }

  // create random AO rays
  for (unsigned i=0; i!=1024; ++i)
  {
    g_randomAORays.push_back(gloost::Vector3(gloost::crand(),
                                             gloost::crand(),
                                             gloost::crand()).normalized());
  }




  g_texter = new gloost::TextureText(g_gloostFolder + "/data/fonts/gloost_Fixedsys_16_gui.png");


  // setup framebuffer
  gloost::Texture* texture = new gloost::Texture( g_bufferWidth,
                                                  g_bufferHeight,
                                                  1,
                                                  0,
                                                  12,
                                                  GL_TEXTURE_2D,
                                                  GL_RGB,
                                                  GL_RGB,
                                                  GL_FLOAT);

  g_framebufferTextureId = gloost::TextureManager::get()->addTexture(texture);

  gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->setTexParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->setTexParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);


  g_renderBuffer = (float*)texture->getPixels();

  g_camera = new gloost::PerspectiveCamera(65.0,
                                           (float)g_screenWidth/(float)g_screenHeight,
                                           0.01,
                                           20.0);

  float xmax = /*g_camera->getNear()**/  tan(g_camera->getFov() * gloost::PI / 360.0) * g_camera->getAspect();
  g_tScaleRatio = xmax / /*g_camera->getNear() /*/ (g_bufferWidth*0.5);
  std::cerr << std::endl << "g_tScaleRatio: " << g_tScaleRatio;
}


//////////////////////////////////////////////////////////////////////////////////////////


void initWithContext()
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
  }
  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_2 ))
  {
    g_cameraDistance += g_mouse.getSpeed()[1]*-0.005;
    g_cameraDistance = gloost::clamp(g_cameraDistance, 0.05f, 20.0f);
  }
  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_3 ))
  {
    gloost::Matrix viewMatrixInv = g_camera->getViewMatrix().inverted();
    viewMatrixInv.setTranslate(0.0,0.0,0.0);

    g_mouse.getSpeed()[2] = 0.0;

    gloost::Vector3 offsetAdd = viewMatrixInv*(g_mouse.getSpeed()*-0.0005);

    g_modelOffset += offsetAdd;
  }

  g_camera->lookAt(gloost::Vector3(-sin(g_cameraRotateY),
                                   g_cameraRotateX,
                                   cos(g_cameraRotateY)).normalized() * g_cameraDistance,
                                   gloost::Point3(0.0f, 0.0f, 0.0f),
                                   gloost::Vector3(0.0f, 1.0f, 0.0f));


  boost::thread_group threadGroup;
  if (g_showRayCastImage)
  {
    for (unsigned i=0; i!=g_num_render_Threads; ++i)
    {
      threadGroup.create_thread( boost::bind( raycastIntoFrameBuffer, i) );
    }
    if (g_join_threads)
    {
      threadGroup.join_all();
    }
  }


  if (g_allJobsDone)
  {
    g_allJobsDone = false;
  }

  gloost::TextureManager::get()->getTextureWithoutRefcount(g_framebufferTextureId)->removeFromContext();


}



//////////////////////////////////////////////////////////////////////////////////////////


static boost::mutex g_jobMutex;
static unsigned     currentIndex = 0;
static unsigned     jobNumber    = 0;


bool getJob(unsigned& startIndex, unsigned& count)
{
  boost::mutex::scoped_lock lock(g_jobMutex);

  const unsigned pixelsPerJob = g_bufferWidth*g_linesPerJob;

  if (g_allJobsDone)
  {
    return false;
  }

  if (currentIndex >= g_bufferWidth*g_bufferHeight)
  {
    g_allJobsDone = true;
    currentIndex  = 0;
    jobNumber     = 0;
    return false;
  }

  startIndex = currentIndex;
  count      = pixelsPerJob;

  currentIndex += pixelsPerJob;
  ++jobNumber;

  return true;
}



//////////////////////////////////////////////////////////////////////////////////////////


bool
raycastIntoFrameBuffer(unsigned threadId)
{
  svo::CpuRaycasterSingleRay3::ResultStruct result;
  svo::CpuRaycasterSingleRay3::ResultStruct resultSecond;


  const gloost::Frustum& frustum = g_camera->getFrustum();

  gloost::Vector3 frustumH_vec         = frustum.far_lower_right - frustum.far_lower_left;
  gloost::Vector3 frustumOnePixelWidth = frustumH_vec/g_bufferWidth;
  gloost::Vector3 frustumV_vec          = frustum.far_upper_left - frustum.far_lower_left;
  gloost::Vector3 frustumOnePixelHeight = frustumV_vec/g_bufferHeight;




  static float whileLoopNormalizer = 280.0;

  unsigned jobCounter = 0;

  unsigned startIndex;
  unsigned count;

  while (jobCounter < g_num_jobsPerThread)
  {
    if (!getJob(startIndex, count))
    {
      return true;
    }

    ++jobCounter;

    for (unsigned i=startIndex; i!=startIndex+count; ++i)
    {
      int x = g_screenCoords[i][0];
      int y = g_screenCoords[i][1];



#if 0 // enable proxy ray
      gloost::Ray ray(gloost::Point3(0.0,0.0,10.0),
                      gloost::Vector3(0.0,0.0,-1.0));
#else

//      gloost::Ray ray = g_camera->getPickRay( g_bufferWidth,
//                                              g_bufferHeight,
//                                              x,
//                                              (int)g_bufferHeight - y);


      gloost::Point3 pickPointOnFarPlane = frustum.far_lower_left
                                         + (frustumOnePixelWidth * x)
                                         + (frustumOnePixelHeight * ((int)g_bufferHeight - y));

      gloost::Vector3 pickDir = (pickPointOnFarPlane - g_camera->getPosition()).normalized();

      gloost::Ray ray(g_camera->getPosition(), pickDir);

#endif
      ray.getOrigin() += g_modelOffset;


      {
        unsigned pixelIndex = (y*g_bufferWidth + x) * 3;

#if 0 // disable actual raycasting

        bool hit           = true;
        result.attribIndex = 0;
#else

        bool hit = g_raycasters[threadId].start(ray,
                                                g_tScaleRatio*g_tScaleRatioMultiplyer,
                                                g_svo,
                                                result);
#endif

        if (hit)
        {
        //          boost::mutex::scoped_lock(g_bufferAccessMutex);

        static const gloost::Vector3 lightDirection = gloost::Vector3(-1.0,2.0,1.0).normalized();

         switch(g_viewMode)
         {
            case 0:
            {
              //                // uncompressed color
              //                unsigned attribIndex = g_voxelAttributes->getPackageIndex(result.attribIndex);
              //                g_renderBuffer[pixelIndex++] = g_voxelAttributes->getVector()[attribIndex+3]; // <-- red
              //                g_renderBuffer[pixelIndex++] = g_voxelAttributes->getVector()[attribIndex+4]; // <-- green
              //                g_renderBuffer[pixelIndex++] = g_voxelAttributes->getVector()[attribIndex+5]; // <-- blue

              // compressed color
              unsigned attribIndex     = g_voxelAttributes->getPackageIndex(result.attribIndex);
              unsigned compressedColor = gloost::float_as_unsigned(g_voxelAttributes->getVector()[attribIndex+1]);
              unsigned char r,g,b,empty;
              gloost::unpackRgbaFromUnsigned(compressedColor,r,g,b,empty);
              g_renderBuffer[pixelIndex++] = r*0.003921569; // <-- red
              g_renderBuffer[pixelIndex++] = g*0.003921569; // <-- green
              g_renderBuffer[pixelIndex++] = b*0.003921569; // <-- blue

              break;
            }
            case 1:
            {

              //                // uncompressed normal
              //                unsigned attribIndex = g_voxelAttributes->getPackageIndex(result.attribIndex);
              //                g_renderBuffer[pixelIndex++] = g_voxelAttributes->getVector()[attribIndex]; // <-- red
              //                g_renderBuffer[pixelIndex++] = g_voxelAttributes->getVector()[attribIndex+1]; // <-- green
              //                g_renderBuffer[pixelIndex++] = g_voxelAttributes->getVector()[attribIndex+2]; // <-- blue


              // compressed normal
              unsigned attribIndex      = g_voxelAttributes->getPackageIndex(result.attribIndex);
              unsigned compressedNormal = gloost::float_as_unsigned(g_voxelAttributes->getVector()[attribIndex]);
              unsigned char nx,ny,nz,empty;
              gloost::unpackRgbaFromUnsigned(compressedNormal,nx,ny,nz,empty);
              g_renderBuffer[pixelIndex++] = ((nx*0.003921569)-0.5) * 2.0; // <-- red
              g_renderBuffer[pixelIndex++] = ((ny*0.003921569)-0.5) * 2.0; // <-- green
              g_renderBuffer[pixelIndex++] = ((nz*0.003921569)-0.5) * 2.0; // <-- blue

              break;
            }
            case 2:
            {
              // SVO Depth
              float depth = result.depth/(float)g_svo->getMaxDepth();
              g_renderBuffer[pixelIndex++] = depth; // <-- red
              g_renderBuffer[pixelIndex++] = depth; // <-- green
              g_renderBuffer[pixelIndex++] = depth; // <-- blue

              break;
            }
            case 3:
            {
              // Z depth
              float tValue = result.t;
              g_renderBuffer[pixelIndex++] = tValue; // <-- red
              g_renderBuffer[pixelIndex++] = tValue; // <-- green
              g_renderBuffer[pixelIndex++] = tValue; // <-- blue

              break;
            }
            case 4:
            {
              // while counter indicator
              float whileIndicatorColor = 1.0 - result.numWhileLoops/whileLoopNormalizer;
              g_renderBuffer[pixelIndex++] = whileIndicatorColor; // <-- red
              g_renderBuffer[pixelIndex++] = whileIndicatorColor; // <-- green
              g_renderBuffer[pixelIndex++] = whileIndicatorColor; // <-- blue

              break;
            }
            case 5:
            {
              //  thread id indicator
              float threadIndicatorColor = threadId/(float)g_num_render_Threads;
              g_renderBuffer[pixelIndex++] = threadIndicatorColor; // <-- red
              g_renderBuffer[pixelIndex++] = threadIndicatorColor; // <-- green
              g_renderBuffer[pixelIndex++] = threadIndicatorColor; // <-- blue

              break;
            }
            case 6:
            {
              // shaded compressed color
              unsigned attribIndex     = g_voxelAttributes->getPackageIndex(result.attribIndex);

              unsigned compressedNormal = gloost::float_as_unsigned(g_voxelAttributes->getVector()[attribIndex]);
              unsigned char nxc,nyc,nzc,emptyc;
              gloost::unpackRgbaFromUnsigned(compressedNormal,nxc,nyc,nzc,emptyc);
              float nx = ((nxc*0.003921569)-0.5) * 2.0; // <-- red
              float ny = ((nyc*0.003921569)-0.5) * 2.0; // <-- green
              float nz = ((nzc*0.003921569)-0.5) * 2.0; // <-- blue

              unsigned compressedColor = gloost::float_as_unsigned(g_voxelAttributes->getVector()[attribIndex+1]);
              unsigned char r,g,b,empty;
              gloost::unpackRgbaFromUnsigned(compressedColor,r,g,b,empty);

              // diffuse shading
              float nDotL = gloost::max(0.0f, (float)(lightDirection * gloost::Vector3(nx,ny,nz)));

              // shadow
              float shadow              = g_shadowBuffer->getVector()[result.attribIndex];
              unsigned numStoredSamples = g_shadowSampleCountBuffer->getVector()[result.attribIndex];
              float newShadowIndicator  = 0.0;
#if 1
              unsigned randStart = gloost::frand()*g_randomShadowRayOffsets.size();
              if (numStoredSamples < 768)
              {
                gloost::Ray shadowRay(result.nodeCenter+gloost::Vector3(nx,ny,nz)*0.01, lightDirection);

                for (unsigned i=0; i!=g_numShadowRays; ++i)
                {
                  shadowRay.setDirection(lightDirection + g_randomShadowRayOffsets[(unsigned)(i+randStart)%g_randomShadowRayOffsets.size()] );
//                  shadowRay.setDirection();
                  shadowRay.normalize();
  //                shadowRay.setOrigin(result.nodeCenter + shadowRay.getDirection()*0.01);

                  shadow += !g_raycasters[threadId].start(shadowRay,
                                                           g_tScaleRatio,
                                                           g_svo,
                                                           resultSecond);
                }
                numStoredSamples += g_numShadowRays;


//                shadow /= numStoredSamples;
//                std::cerr << std::endl << "shadow: " << shadow;
                g_shadowBuffer->getVector()[result.attribIndex]            = shadow;
                g_shadowSampleCountBuffer->getVector()[result.attribIndex] = numStoredSamples;
//                newShadowIndicator = 1.0f;
              }
#else
              shadow = 1;
#endif


             float shading = 0.003921569*shadow/numStoredSamples*nDotL;

              g_renderBuffer[pixelIndex++] = r*0.0006 + r*shading + newShadowIndicator; // <-- red
              g_renderBuffer[pixelIndex++] = g*0.0006 + g*shading; // <-- green
              g_renderBuffer[pixelIndex++] = b*0.0006 + b*shading; // <-- blue


              break;
            }
          } // switch
        }
        else
        {
          g_renderBuffer[pixelIndex++] = 0.1;
          g_renderBuffer[pixelIndex++] = 0.1;
          g_renderBuffer[pixelIndex++] = 0.25;
        }
      }

    } // for startIndex ... count

  }// while job

  return true;

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
          g_texter->renderTextLine(20, g_screenHeight -20, "raycast serialized cpu test");
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
        g_viewModeText = "thread indication";
        break;

      case '6':
        g_viewMode = 6;
        g_viewModeText = "shaded";
        break;



      case 'R':
        reloadShaders();
        break;

      case ' ':
          g_toggle_renderer = !g_toggle_renderer;
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
//  glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 16);
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

