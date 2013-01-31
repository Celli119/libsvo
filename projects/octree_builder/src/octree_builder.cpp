/**

  Last change 30.04.24 by Felix Weißig

  Email: thesleeper@gmx.net
  skype: otacocato
  URL:   http://www.otaco.de

**/



////////////////////////////////////////////////////////////////////////////////


/// c++ includes
#include <iostream>
#include <cmath>
#include <stdlib.h>

// default values
std::string g_inputPath    = "";
std::string g_outputPath   = "";
unsigned g_treeletSizeInByte  = 512u*1024u;
unsigned g_maxSvoDepth        = 8u;
unsigned g_nodesVisDepth      = 99u;
unsigned g_numBuildingThreads = 12u;
float    g_rotateXInDeg       = 0.0f;

#include <gloost/Matrix.h>
gloost::Matrix g_sizeAndCenterMatrix;


#include <gloost/PlyLoader.h>
#include <gloost/InterleavedAttributes.h>
#include <gloost/Mesh.h>
gloost::Mesh* g_mesh = 0;

// SVO
//#include <SvoBranch.h>
//svo::SvoBranch* g_svo = 0;

#include <TreeletBuildManager.h>
svo::TreeletBuildManager g_treeletBuildManager;


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

#include <gloost/DirectoryList.h>


////////////////////////////////////////////////////////////////////////////////



  /// load and initialize resources for our demo

void init()
{



  // merge files of a directory
  if (gloost::pathToBasename(g_inputPath).length() == 0)
  {
    std::cerr << std::endl;
    std::cerr << std::endl << "Reading directory: " << g_inputPath;


    g_mesh = new gloost::Mesh();
    g_mesh->takeReference();

    gloost::DirectoryList directory(g_inputPath);
    directory.open(".ply");

    for (unsigned i=0; i!=directory.get_entries().size(); ++i)
    {
      std::cerr << std::endl << "  -> " << gloost::pathToBasename(directory.get_entries()[i]);
    }

    std::cerr << std::endl;
    std::cerr << std::endl << "Loading and mergin files: " << g_inputPath;
    for (unsigned i=0; i!=directory.get_entries().size(); ++i)
    {
      std::cerr << std::endl << "  -> " << gloost::pathToBasename(directory.get_entries()[i]) << " ... ";

      gloost::PlyLoader ply(directory.get_entries()[i]);

      std::cerr << std::endl << "loaded ... ";

      g_mesh->add(ply.getMesh());

      std::cerr << std::endl << "merged";
    }
    g_mesh->takeReference();
  }
  // just load one file
  else
  {
    std::cerr << std::endl << "Loading geometry: " << g_inputPath;
    gloost::PlyLoader ply(g_inputPath);
    g_mesh = ply.getMesh();
    g_mesh->takeReference();
  }

  g_mesh->recalcBoundingBox();

  std::cerr << std::endl << " Mesh memmory usage: " << g_mesh->getMemoryUsageCpu()/1024.0/1024.0 << " MB";



  /// transform
  gloost::Matrix rotateMat = gloost::Matrix::createIdentity();

  if (gloost::abs(g_rotateXInDeg) > 0.00001)
  {
    rotateMat = gloost::Matrix::createRotMatrix(gloost::Vector3(1.0, 0.0, 0.0), gloost::deg2rad(g_rotateXInDeg));
  }

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


  // SVO builder
  g_treeletBuildManager.buildFromFaces(g_treeletSizeInByte,
                                       g_maxSvoDepth,
                                       g_numBuildingThreads,
                                       g_mesh);

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
  cmdp.addOpt("s", 1,  "<uint treelet size in KB>", "size of a treelet in Kilobyte(default: " + gloost::toString(g_treeletSizeInByte) + ")");
  cmdp.addOpt("d", 1,  "<uint min svo depth>", "minimal depth of all leaves (default: " + gloost::toString(g_maxSvoDepth) + ")");
  cmdp.addOpt("i", 1,  "input", "input file path, loads *.ply files with per vertex color and normals");
  cmdp.addOpt("o", 1,  "output", "output directory path (default: input file path)");
  cmdp.addOpt("X", 1,  "rotationX", "rotate the input mesh around the X axis");

  cmdp.init(argc,argv);


  if (!cmdp.isOptSet("i"))
  {
    std::cerr << std::endl << "No input file was specified.";
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


  if (cmdp.isOptSet("s"))
  {
    g_treeletSizeInByte = cmdp.getOptsInt("s")[0]*1024u;
  }
  else
  {
    std::cerr << std::endl << " USE THE -s argument to set Treelet size in kilobyte: ";
    std::cerr << std::endl;
    exit(0);
  }

  if (cmdp.isOptSet("d"))
  {
    g_maxSvoDepth = cmdp.getOptsInt("d")[0];
  }
  else
  {
    std::cerr << std::endl << " USE THE -d argument to set the global minimal depth of the svo: ";
    std::cerr << std::endl;
    exit(0);
  }

  if (cmdp.isOptSet("i"))
  {
    g_inputPath = cmdp.getOptsString("i")[0];
    g_outputPath = gloost::pathToBasePath(g_inputPath);
  }
  else
  {
    std::cerr << std::endl << " USE THE -i argument to specify an input file (*.ply): ";
    std::cerr << std::endl;
    exit(0);
  }


  if (cmdp.isOptSet("o"))
  {
    g_outputPath = cmdp.getOptsString("o")[0];
  }


  if (cmdp.isOptSet("X"))
  {
    g_rotateXInDeg = cmdp.getOptsFloat("X")[0];
  }


  /// load and intialize stuff for our demo
  init();


  return EXIT_SUCCESS;
}


