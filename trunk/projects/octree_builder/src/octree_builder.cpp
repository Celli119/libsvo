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


/// c++ includes
#include <iostream>
#include <cmath>
#include <stdlib.h>
std::string g_inputPath    = "";
std::string g_outputPath   = "";

#include <gloost/Matrix.h>
gloost::Matrix g_sizeAndCenterMatrix;


#include <gloost/PlyLoader.h>
#include <gloost/InterleavedAttributes.h>
#include <gloost/Mesh.h>
gloost::Mesh* g_mesh = 0;

// SVO
#include <Svo.h>
svo::Svo* g_svo = 0;
#include <SvoNode.h>

#include <SvoBuilderFaces.h>

#include <SvoVisualizer.h>
svo::SvoVisualizer* g_svoVisualizerNodes  = 0;
svo::SvoVisualizer* g_svoVisualizerLeaves = 0;

//#include <chrono>
#include <attribute_generators/Ag_colorAndNormalsTriangles.h>
#include <attribute_generators/Ag_colorAndNormalsFromTextures.h>



// info
bool g_showTextInfo                = true;
bool g_showModel                   = false;
bool g_showOctreeNodes             = false;
bool g_showOctreeLeaves            = true;
bool g_showSerializedSvoTexture    = false;
bool g_showSerializedAttribTexture = false;

////////////////////////////////////////////////////////////////////////////////

void init();
void buildSvoVisualization(gloost::InterleavedAttributes* attributes);
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


unsigned g_maxSvoDepth        = 11;
unsigned g_nodesVisDepth      = 99;

unsigned g_numBuildingThreads = 12;


////////////////////////////////////////////////////////////////////////////////


  /// load and initialize resources for our demo

void init()
{

  std::cerr << std::endl << "Loading geometry: " << g_inputPath;
  gloost::PlyLoader ply(g_inputPath);
  g_mesh = ply.getMesh();
  std::cerr << std::endl << " Mesh memmory usage: " << g_mesh->getMemoryUsageCpu()/1024.0/1024.0 << " MB";


  /// transform
  gloost::Matrix rotateMat;
  rotateMat.setIdentity();

  gloost::Matrix translateMat;
  translateMat.setIdentity();
  translateMat.setTranslate(g_mesh->getOffsetToCenter());

  gloost::Matrix scaleMat;
  scaleMat.setIdentity();
  scaleMat.setScale(g_mesh->getScaleFactorToSize(1.0));

  g_sizeAndCenterMatrix.setIdentity();
  g_sizeAndCenterMatrix =  scaleMat * (rotateMat * translateMat);

  g_mesh->transform(g_sizeAndCenterMatrix, true);
  g_mesh->normalizeNormals();


//  g_mesh->printMeshInfo();


  // SVO builder
  g_svo = new svo::Svo(g_maxSvoDepth);
  svo::SvoBuilderFaces fromFaceBuilder(g_numBuildingThreads);
  fromFaceBuilder.build(g_svo, g_mesh);

  // attribute generator
  svo::Ag_colorAndNormalsTriangles generator;
  generator.generate(g_svo, g_mesh, new gloost::ObjMatFile());


  g_svo->serializeSvo();

  g_svo->writeSerializedSvoToFile(g_outputPath
                                   + gloost::pathToBasename(g_inputPath)
                                   + "_" + gloost::toString(g_maxSvoDepth)
                                   + ".svo" );

  generator.writeCompressedAttributeBufferToFile(g_outputPath
                                                 + gloost::pathToBasename(g_inputPath)
                                                 + "_" + gloost::toString(g_maxSvoDepth) + "c"
                                                 + ".ia" );

  std::cerr << std::endl;
  std::cerr << std::endl;
  std::cerr << std::endl;
}


////////////////////////////////////////////////////////////////////////////////

#include <gloost/CmdParser.h>

int main(int argc, char *argv[])
{
  gloost::CmdParser cmdp("");

  cmdp.addOpt("t", 1,  "threads  ", "number of threads (default: 12)");
  cmdp.addOpt("d", 1,  "svo depth", "max depth of the resulting svo (default: " + gloost::toString(g_maxSvoDepth) + ")");
  cmdp.addOpt("i", 1,  "input", "input file path, loads *.ply files with per vertex color and normals");
  cmdp.addOpt("o", 1,  "output", "output directory path (default: input file path)");

  cmdp.init(argc,argv);


  if (!cmdp.isOptSet("i"))
  {
    std::cerr << std::endl << "Hey sucker! No input file was specified.";
    std::cerr << std::endl << "Use the -i parameter to do it.";
    std::cerr << std::endl;
    std::cerr << std::endl;
    cmdp.showHelp();
    exit(EXIT_SUCCESS);
  }


  if (argc == 1)
  {
    cmdp.showHelp();
    exit(EXIT_SUCCESS);
  }





  if (cmdp.isOptSet("t"))
  {
    g_numBuildingThreads = cmdp.getOptsInt("t")[0];
  }
  if (cmdp.isOptSet("d"))
  {
    g_maxSvoDepth = cmdp.getOptsInt("d")[0];
  }
  if (cmdp.isOptSet("i"))
  {
    g_inputPath = cmdp.getOptsString("i")[0];
    g_outputPath = gloost::pathToBasePath(g_inputPath);
  }
  if (cmdp.isOptSet("o"))
  {
    g_outputPath = cmdp.getOptsString("o")[0];
  }


  /// load and intialize stuff for our demo
  init();


  return EXIT_SUCCESS;
}


