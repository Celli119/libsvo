/**

  Last change 30.04.24 by Felix Weißig

  Email: thesleeper@gmx.net
  skype: otacocato
  URL:   http://www.otaco.de

**/

/**

  Converts ply meshes into gbm meshes

**/


////////////////////////////////////////////////////////////////////////////////


/// c++ includes
#include <iostream>
#include <cmath>
#include <stdlib.h>

// gloost includes
#include <CmdParser.h>
#include <gloostHelper.h>
#include <DirectoryList.h>
#include <Mesh.h>
#include <PlyLoader.h>
#include <GbmWriter.h>



////////////////////////////////////////////////////////////////////////////////


void init();
void convert(const std::string& inPath, const std::string& outPath);
void cleanup();

std::string g_inFilePath  = "";
std::string g_outFilePath = "";



#define FILE_MODE_NONE      0
#define FILE_MODE_DIRECTORY 1
#define FILE_MODE_FILE      2

////////////////////////////////////////////////////////////////////////////////


  /// load and initialize resources for our demo

void init()
{
  unsigned int inMode  = 0;
  unsigned int outMode = 0;


  // in
  std::string inBasePath  = gloost::pathToBasePath(g_inFilePath);
  std::string inBasename  = gloost::pathToBasename(g_inFilePath);
  std::string inExtension = gloost::pathToExtension(g_inFilePath);

  if (inBasePath.size())
  {
    inMode = FILE_MODE_DIRECTORY;
  }
  if (inExtension.size())
  {
    inMode = FILE_MODE_FILE;
  }


  // out
  std::string outBasePath  = gloost::pathToBasePath(g_outFilePath);
  std::string outBasename  = gloost::pathToBasename(g_outFilePath);
  std::string outExtension = gloost::pathToExtension(g_outFilePath);

  if (outBasePath.size())
  {
    outMode = FILE_MODE_DIRECTORY;
  }
  if (outExtension.size())
  {
    outMode = FILE_MODE_FILE;
  }


  if (inMode == FILE_MODE_FILE)
  {
    std::string realInPath = g_inFilePath;
    std::string realOutPath = "";

    if (outMode == FILE_MODE_NONE)
    {
      realOutPath = inBasePath + inBasename + ".gbm";
    }
    else if (outMode == FILE_MODE_DIRECTORY)
    {
      realOutPath = outBasePath + inBasename + ".gbm";
    }
    else // (outMode == FILE_MODE_FILE)
    {
      realOutPath = outBasePath + outBasename + "." + outExtension;
    }

    convert(realInPath, realOutPath);
  }


  // convert a whole directory
  else if (inMode == FILE_MODE_DIRECTORY)
  {
    gloost::DirectoryList dir(inBasePath);
    dir.open("ply");

    std::vector<std::string>& infiles = dir.get_entries();

    for (unsigned int i=0; i!=infiles.size(); ++i)
    {

      if (outMode == FILE_MODE_NONE)
      {
        convert(infiles[i], inBasePath + gloost::pathToBasename(infiles[i]) + ".gbm");
      }
      else if (outMode == FILE_MODE_DIRECTORY)
      {
        convert(infiles[i], outBasePath + gloost::pathToBasename(infiles[i]) + ".gbm");
      }


    }

  }





}


////////////////////////////////////////////////////////////////////////////////


void convert(const std::string& inPath, const std::string& outPath)
{
  std::cerr << std::endl;
  std::cerr << std::endl << "Converting: " << inPath <<
               std::endl << "        to: " << outPath;

  gloost::PlyLoader ply(inPath);

  if (!ply.getMesh()->isEmpty())
  {
    ply.getMesh()->interleave();

    gloost::GbmWriter gbm;
    gbm.write(outPath, ply.getMesh());
  }
  else
  {
    std::cerr << std::endl << "    ERROR: Mesh is empty";
    std::cerr << std::endl;
  }
}



////////////////////////////////////////////////////////////////////////////////


  /// free all resources we created

void cleanup()
{


}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[])
{

  if (argc == 1 || argc > 3)
  {
    std::cerr << std::endl;
    std::cerr << std::endl << "Converts a ply file (Stanford Polygon Format) into an";
    std::cerr << std::endl << "gdm file (Gloost Binary Mesh)";
    std::cerr << std::endl;
    std::cerr << std::endl << "  Examples:";
    std::cerr << std::endl;
    std::cerr << std::endl << "    gbmConv <inputFile>";
    std::cerr << std::endl << "    gbmConv <inputFile> <outputFile>";
    std::cerr << std::endl << "    gbmConv <inputDirectory>";
    std::cerr << std::endl << "    gbmConv <inputDirectory> <outputDirectory>";
    std::cerr << std::endl;
    std::cerr << std::endl;
  }
  else if (argc == 2)
  {
    g_inFilePath = argv[1];
  }
  else if (argc == 3)
  {
    g_inFilePath  = argv[1];
    g_outFilePath = argv[2];
  }


  /// load and intialize stuff for our demo
  init();

  /// cleanup all resources we created for the demo
  cleanup();


  std::cerr << std::endl;

  return EXIT_SUCCESS;
}


