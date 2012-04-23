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

#include <gloost/gloostRenderGoodies.h>


#include <gloost/TextureText.h>
gloost::TextureText* g_texter = 0;
double g_timePerFrame = 0;

std::string g_gloostFolder = "../../gloost/";
std::string g_dataPath     = "../data/";

#include <gloost/Matrix.h>
#include <gloost/Mouse.h>
gloost::Mouse g_mouse;

#include <gloost/FirstPersonCamera.h>
gloost::FirstPersonCamera*   g_camera;


gloost::Point3 g_modelOffset;
float          g_cameraRotateY  = 0;
float          g_cameraRotateX  = 0;
float          g_cameraDistance = 4.0;


gloost::Ray g_ray;
#include <gloost/BinaryFile.h>
#include <gloost/Mesh.h>
#include <gloost/GbmWriter.h>
#include <gloost/GbmLoader.h>
#include <gloost/Vbo.h>
gloost::Vbo* g_drawAxisVbo = 0;
gloost::Vbo* g_pointsVbo   = 0;


std::vector<gloost::Matrix> _offsetMatrices;

#include <gloost/Shader.h>
gloost::Shader*     g_SvoVisualizingShader = 0;

// info
bool g_showTextInfo = true;
bool g_showPoints   = false;

void init();
void loadOffsetMatrices (const std::vector<std::string>& fileNames);
gloost::Mesh* loadPointsGbm(const std::string& fileName);
gloost::Mesh* loadPointsTxt(const std::string& fileName, unsigned int offsetMatrixIndex);
gloost::Mesh* loadAndMergePointClouds(const std::vector<std::string>& fileNames);
float readStringValueToFloat (std::ifstream& infile);
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

void
init()
{

#define USE_GBM_FILES
#define OPEN_GL_WINDOW
#define DRAW_POINTS_VBO
//#define WRITE_MERGED_POINTS_TO_GBM


#define UNDER_SAMPLING 5

  // shaders
  reloadShaders();



  std::vector<std::string> filesNames;

//  std::string outFilePath = "/home/otaco/Desktop/pointclouds/schlachte1/schlachte1_merged_points_" + gloost::toString(UNDER_SAMPLING) + ".gbm";
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan001");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan002");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan003");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan004");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan005");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan006");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan007");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan008");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan009");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan010");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan011");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan012");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan013");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan014");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan015");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan016");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan017");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan018");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/schlachte1/scan019");


//  std::string outFilePath = "/home/otaco/Desktop/pointclouds/bremen_city/bremen_merged_points_" + gloost::toString(UNDER_SAMPLING) + ".gbm";
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/bremen_city/scan000");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/bremen_city/scan001");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/bremen_city/scan002");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/bremen_city/scan003");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/bremen_city/scan004");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/bremen_city/scan005");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/bremen_city/scan006");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/bremen_city/scan007");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/bremen_city/scan008");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/bremen_city/scan009");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/bremen_city/scan010");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/bremen_city/scan011");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/bremen_city/scan012");



  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan000");
  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan001");
  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan002");
  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan003");
  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan004");
  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan005");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan006");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan007");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan008");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan009");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan010");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan011");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan012");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan013");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan014");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan015");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan016");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan017");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan018");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan019");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan020");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan021");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan022");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan023");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan024");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan025");
//  filesNames.push_back("/home/otaco/Desktop/pointclouds/rescue_area/scan026");



//  filesNames.push_back("/home/otaco/Desktop/pointclouds/scans_von_steppo/besprechungsraum_xyzrgb_02");





  loadOffsetMatrices(filesNames);


#if 1

  //
  gloost::Mesh* mesh = loadAndMergePointClouds(filesNames);


  std::cerr << std::endl << "mesh->getVertices().size(): " << mesh->getVertices().size();


#ifndef USE_GBM_FILES
    gloost::Matrix preTransform;
    preTransform.setIdentity();
    preTransform.setRotateX(gloost::deg2rad(-90.0f));

    mesh->transform(preTransform);
#endif

#endif


#ifdef OPEN_GL_WINDOW
#ifdef DRAW_POINTS_VBO

  mesh->getPoints().resize(mesh->getVertices().size(), 0);
  for (unsigned i=0; i!=mesh->getVertices().size(); ++i)
  {
    mesh->getPoints()[i] = i;
  }


  mesh->interleave();
  g_pointsVbo = new gloost::Vbo(mesh);

#endif

  g_texter = new gloost::TextureText(g_gloostFolder + "/data/fonts/gloost_Fixedsys_16_gui.png");

  // axis
  gloost::Mesh* axisMesh = new gloost::Mesh();
  axisMesh->getVertices().push_back(gloost::Point3(0.7, 0.0, 0.0));
  axisMesh->getColors().push_back(gloost::vec4(1.0, 0.0, 0.0, 1.0));

  axisMesh->getVertices().push_back(gloost::Point3(2.0, 0.0, 0.0));
  axisMesh->getColors().push_back(gloost::vec4(1.0, 0.0, 0.0, 1.0));

  axisMesh->getVertices().push_back(gloost::Point3(0.0, 0.7, 0.0));
  axisMesh->getColors().push_back(gloost::vec4(0.0, 1.0, 0.0, 1.0));

  axisMesh->getVertices().push_back(gloost::Point3(0.0, 2.0, 0.0));
  axisMesh->getColors().push_back(gloost::vec4(0.0, 1.0, 0.0, 1.0));

  axisMesh->getVertices().push_back(gloost::Point3(0.0, 0.0, 0.7));
  axisMesh->getColors().push_back(gloost::vec4(0.0, 0.0, 1.0, 1.0));

  axisMesh->getVertices().push_back(gloost::Point3(0.0, 0.0, 2.0));
  axisMesh->getColors().push_back(gloost::vec4(0.0, 0.0, 1.0, 1.0));

  axisMesh->getLines().push_back(gloost::Line(0, 1));
  axisMesh->getLines().push_back(gloost::Line(2, 3));
  axisMesh->getLines().push_back(gloost::Line(4, 5));

  g_drawAxisVbo = new gloost::Vbo(axisMesh);


#endif

}


//////////////////////////////////////////////////////////////////////////////////////////

void loadOffsetMatrices (const std::vector<std::string>& fileNames)
{

  for (unsigned int f=0; f!=fileNames.size(); ++f)
  {
    // read transform file
    std::string transformFilePath = gloost::pathToBasePath(fileNames[f]) +
                                    gloost::pathToBasename(fileNames[f]) + ".dat";

    gloost::Matrix transformMat;
    transformMat.setIdentity();

    std::cerr << std::endl;
    std::cerr << std::endl << "Processing # " << f+1 << " of " << fileNames.size() << " files:";
    std::cerr << std::endl << "  Reading transform file: " << transformFilePath;

    std::ifstream transformFile;
    transformFile.open(transformFilePath.c_str());

    if (transformFile)
    {
      std::string floatString;

      for (unsigned int i=0; i!=4; ++i)
      {
        transformFile >> floatString;
        transformMat[0+i] = atof(floatString.c_str());
        transformFile >> floatString;
        transformMat[4+i] = atof(floatString.c_str());
        transformFile >> floatString;
        transformMat[8+i] = atof(floatString.c_str());
        transformFile >> floatString;
        transformMat[12+i] = atof(floatString.c_str());
      }
    }
    else
    {
      std::cerr << std::endl << "  ERROR: could NOT open transform file " << transformFilePath;
    }
    _offsetMatrices.push_back(transformMat);
    transformFile.close();

  }


}


//////////////////////////////////////////////////////////////////////////////////////////


gloost::Mesh*
loadAndMergePointClouds(const std::vector<std::string>& fileNames)
{

  gloost::Mesh* mesh = new gloost::Mesh();
  mesh->takeReference();


#ifdef KNOBI
  // knobi file out
  std::cerr << std::endl << "Writing Knobi Out: ";
  std::cerr << std::endl;
  gloost::BinaryFile outFile;
  outFile.openToWrite("/home/otaco/Desktop/schlachte.xyz");
#endif



  for (unsigned int f=0; f!=fileNames.size(); ++f)
  {
    gloost::Mesh* partMesh = 0;

#ifdef USE_GBM_FILES
    partMesh = loadPointsGbm(gloost::pathToBasePath(fileNames[f]) + gloost::pathToBasename(fileNames[f]) + ".gbm");
#else
    partMesh = loadPointsTxt(gloost::pathToBasePath(fileNames[f]) + gloost::pathToBasename(fileNames[f]) + ".txt", f);
#endif

//#define KNOBI
#ifdef KNOBI
  std::cerr << std::endl << "Writing: " << partMesh->getVertices().size() << " points...";
  std::vector<gloost::Point3>& positions = partMesh->getVertices();
  std::vector<gloost::vec4>& colors      = partMesh->getColors();


  for (unsigned i=0; i!=partMesh->getVertices().size(); ++i)
  {
    if (i % 250000 == 0)
    {
      std::cerr << "*";
    }

    outFile.writeString(gloost::toString(positions[i][0]));
    outFile.writeChar8(' ');
    outFile.writeString(gloost::toString(positions[i][1]));
    outFile.writeChar8(' ');
    outFile.writeString(gloost::toString(positions[i][2]));
    outFile.writeChar8(' ');

    int intens = (int)gloost::clamp(colors[i].r*255.0f, 0.0f, 255.0f);

    outFile.writeString(gloost::toString(intens));
    outFile.writeChar8(' ');
    outFile.writeString(gloost::toString(intens));
    outFile.writeChar8(' ');
    outFile.writeString(gloost::toString(intens));
    outFile.writeChar8('\n');
  }

#else

  mesh->add(partMesh);
  std::cerr << std::endl << "current mesh load : " << mesh->getVertices().size();
  std::cerr << std::endl;

#endif

    partMesh->dropReference();
  }



#ifdef KNOBI
  outFile.close();
#endif

  return mesh;


}

//////////////////////////////////////////////////////////////////////////////////////////


gloost::Mesh*
loadPointsGbm(const std::string& fileName)
{

  std::cerr << std::endl << "  Reading points from file: " << fileName;

  gloost::GbmLoader gbmLoader(fileName);

  if (UNDER_SAMPLING > 1)
  {
    std::vector<gloost::Point3> _undersampledVertices;
    std::vector<gloost::vec4>   _undersampledColors;

    for (unsigned int i=0; i!=gbmLoader.getMesh()->getVertices().size(); ++i)
    {
      if (i % UNDER_SAMPLING == 0)
      {
        _undersampledVertices.push_back(gbmLoader.getMesh()->getVertices()[i] );
        _undersampledColors.push_back(gbmLoader.getMesh()->getColors()[i] );
      }
    }

    gbmLoader.getMesh()->clear();
    gbmLoader.getMesh()->getVertices() = _undersampledVertices;
    gbmLoader.getMesh()->getColors()   = _undersampledColors;
  }


  gbmLoader.getMesh()->takeReference();

  return gbmLoader.getMesh();
}

//////////////////////////////////////////////////////////////////////////////////////////


gloost::Mesh*
loadPointsTxt(const std::string& fileName, unsigned int offsetMatrixIndex)
{
  gloost::Mesh* mesh = new gloost::Mesh();
  mesh->takeReference();

  std::cerr << std::endl << "  Reading points from file: " << fileName;

  gloost::BinaryFile inFile;
  if (!inFile.openAndLoad(fileName))
  {
    std::cerr << std::endl << "ERROR: Could NOT open " << fileName;
    return mesh;
  }


  std::string numPointsString = "0";//inFile.readWord();
  unsigned int numPoints = atoi(numPointsString.c_str());

  std::cerr << std::endl << "  Parsing file: " << fileName;
  std::cerr << std::endl << "     Parsing: Points...";

  for (unsigned int i=0; !inFile.eof(); ++i)
  {
    gloost::Point3 position;

    // cartesian coordinates
    position[0] = atof(inFile.readWord().c_str());
    position[1] = atof(inFile.readWord().c_str());
    position[2] = atof(inFile.readWord().c_str());


    // polar coordinates
//    float alpha = atof(inFile.readWord().c_str());
//    float beta  = atof(inFile.readWord().c_str());
//    float gamma = atof(inFile.readWord().c_str());
    inFile.readWord();
    inFile.readWord();
    inFile.readWord();


    float intensity = atof(inFile.readWord().c_str());

    unsigned int vertexCounter = 0;
    if (i % UNDER_SAMPLING == 0)
    {
      position = _offsetMatrices[offsetMatrixIndex] * position;

//        alpha = gloost::abs(alpha)/720.0f;
//        beta  = gloost::abs(beta)/720.0f;
//        gamma = gloost::abs(gamma)/720.0f;

      intensity = (intensity+15.0f)/30.0f;
      mesh->getPoints().push_back(vertexCounter++);
      mesh->getVertices().push_back(position);
      mesh->getColors().push_back(gloost::vec4(intensity,intensity,intensity,1.0));
    }


  }

  std::cerr << std::endl << "     done.";
  std::cerr << std::endl;

  inFile.unload();


//#define WRITE_SINGLE_FILES

#ifdef WRITE_SINGLE_FILES
  mesh->getPoints().clear();

  std::string basePath = gloost::pathToBasePath(fileName);
  std::string baseName = gloost::pathToBasename(fileName);

  gloost::GbmWriter writer;
  writer.write(basePath + "/" + baseName + std::string(".gbm"), mesh);

  mesh->clear();
#endif


  return mesh;

}


//////////////////////////////////////////////////////////////////////////////////////////

float readStringValueToFloat (std::ifstream& infile)
{
  std::string word;
  infile >> word;
  return atof(word.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////


void
initWithContext()
{
  g_camera = new gloost::FirstPersonCamera();

  g_camera->lookAt(gloost::Point3(0.0f, 0.0f, 0.0f),
                   gloost::Point3(0.0f, 0.0f, -1.0f),
                   gloost::Vector3(0.0f, 1.0f, 0.0f));

//  glPointSize(2.0f);
}

//////////////////////////////////////////////////////////////////////////////////////////


void frameStep()
{
//  if (g_frameCounter % 2 == 0)
//    glfwSetMousePos(g_screenWidth*0.5,g_screenHeight*0.5);


  glfwPollEvents();


  int x,y;

  glfwGetMousePos(&x,&y);

  g_mouse.setSpeedToZero();
  g_mouse.setLoc(x,g_screenHeight-y,0);

  if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_1 ))
  {
    g_camera->rotateH(g_mouse.getSpeed()[0]*-0.003);
    g_camera->rotateV(g_mouse.getSpeed()[1]*0.003);
  }

  if (glfwGetKey('W'))
  {
    g_camera->move(gloost::Vector3(0.0,0.0,-16.0*g_timePerFrame));
  }
  else if (glfwGetKey('S'))
  {
    g_camera->move(gloost::Vector3(0.0,0.0,16.0*g_timePerFrame));
  }
  if (glfwGetKey(' '))
  {
    g_camera->move(gloost::Vector3(0.0,16.0*g_timePerFrame,0.0));
  }
  if (glfwGetKey(GLFW_KEY_LSHIFT))
  {
    g_camera->move(gloost::Vector3(0.0,-16.0*g_timePerFrame,0.0));
  }

  if (glfwGetKey('A'))
  {
    g_camera->move(gloost::Vector3(-16.0*g_timePerFrame,0.0,0.0));
  }
  else if (glfwGetKey('D'))
  {
    g_camera->move(gloost::Vector3(16.0*g_timePerFrame,0.0,0.0));
  }

  g_camera->setUpVector(gloost::Vector3(0.0, 1.0, 0.0));

//  glfwSetMousePos(g_screenWidth * 0.5, g_screenHeight *0.5);


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

//
//  g_camera->lookAt(gloost::Vector3(-sin(g_cameraRotateY), g_cameraRotateX, cos(g_cameraRotateY)).normalized() * g_cameraDistance,
//                                     gloost::Point3(0.0f, 0.0f, 0.0f),
//                                     gloost::Vector3(0.0f, 1.0f, 0.0f));
//


  g_camera->set();


  gloost::Matrix offset;
  offset.setIdentity();
  offset.setTranslate(g_modelOffset);


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
    // points
    if (g_drawAxisVbo)
    {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glPushMatrix();
      {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        g_SvoVisualizingShader->set();
          g_pointsVbo->bind();
          g_pointsVbo->draw();
          g_pointsVbo->disable();
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
          g_texter->renderTextLine("fps:    " + gloost::toString(1.0/g_timePerFrame));
          g_texter->renderTextLine("points: " + gloost::toString( g_pointsVbo->getMesh()->getPoints().size() ));
          g_texter->renderFreeLine();
          glColor4f(1.0f, 1.0f, 1.0f, 0.5);
          g_texter->renderTextLine("(h) Show text info:      " + gloost::toString( g_showTextInfo )) ;
          g_texter->renderTextLine("(2) Show points:   " + gloost::toString( g_showPoints )) ;
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
  {
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
      case '2':
        g_showPoints = !g_showPoints;
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
//  glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 2);
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

#endif

  /// cleanup all resources we created for the demo
  cleanup();


  return EXIT_SUCCESS;
}


