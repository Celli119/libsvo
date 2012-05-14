
/*
                       ___                            __
                      /\_ \                          /\ \__
                   __ \//\ \     ___     ___     ____\ \  _\
                 /'_ `\ \ \ \   / __`\  / __`\  /  __\\ \ \/
                /\ \ \ \ \_\ \_/\ \ \ \/\ \ \ \/\__   \\ \ \_
                \ \____ \/\____\ \____/\ \____/\/\____/ \ \__\
                 \/___/\ \/____/\/___/  \/___/  \/___/   \/__/
                   /\____/
                   \_/__/

                   OpenGL framework for fast demo programming

                             http://www.gloost.org

    This file is part of the gloost framework. You can use it in parts or as
       whole under the terms of the GPL (http://www.gnu.org/licenses/#GPL).

            gloost is being created by Felix Weißig and Stephan Beck

     Felix Weißig (thesleeper@gmx.net), Stephan Beck (stephan@pixelstars.de)
*/


///
#include <SvoLoader.h>

/// gloost system includes
#include <gloost/BinaryFile.h>
#include <gloost/BinaryBundle.h>
#include <gloost/Texture.h>
#include <gloost/TextureManager.h>
#include <gloost/InterleavedAttributes.h>



/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class   SvoLoader

  \brief   Reads a Svo file and Attribute files and provides the data as arrays and as TBOs

  \author  Felix Weiszig
  \date    May 2012
  \remarks
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

SvoLoader::SvoLoader(const std::string& basePath):
    _svoCpuData(),
    _svoChildPointersBundle(0),
    _svoBitMasksBundle(0),
    _svoChildPointersTexId(0),
    _bitMaskTexId(0),
    _svoAttributePointersTexId(0),
    _attributePointersBundle(0),
    _attributeDataTextureIds()

{

  // load and prepare svo data
	std::string svoDataPath = basePath + ".svo";

  std::cerr << std::endl;
  std::cerr << std::endl << "Message from SvoLoader::SvoLoader(const std::string& basePath): ";
  std::cerr << std::endl << "             Loading " + svoDataPath;

	gloost::BinaryFile svoDataFile;

	if (!svoDataFile.openAndLoad(svoDataPath))
	{
    std::cerr << std::endl;
    std::cerr << std::endl << "ERROR in SvoLoader::SvoLoader(const std::string& basePath): ";
    std::cerr << std::endl << "             Could NOT load \"" + svoDataPath + "\"";
    std::cerr << std::endl;
	  return;
	}

  // reading depth and number of nodes
	unsigned svoDepth = atoi(svoDataFile.readWord().c_str());
	unsigned numNodes = atoi(svoDataFile.readWord().c_str());

  std::cerr << std::endl << "             depth           " << svoDepth;
  std::cerr << std::endl << "             Number of Nodes " << numNodes;

  _svoCpuData.resize(numNodes);
  memcpy(&_svoCpuData.front(),
         svoDataFile.getCurrent(),
         numNodes*sizeof(CpuSvoNode));


  // create and fill 3 buffers: 1. Child pointers, 2. Bitmasks, 3. Attribute pointers
  _svoChildPointersBundle  = new gloost::BinaryBundle(numNodes*sizeof(unsigned));
  _svoBitMasksBundle       = new gloost::BinaryBundle(numNodes*sizeof(unsigned char) * 2);
  _attributePointersBundle = new gloost::BinaryBundle(numNodes*sizeof(unsigned));

  for (unsigned i=0; i!=_svoCpuData.size(); ++i)
  {
    _svoChildPointersBundle->putUnsigned(_svoCpuData[i].getFirstChildIndex());
    _svoBitMasksBundle->putChar(_svoCpuData[i].getValidMask().getValue());
    _svoBitMasksBundle->putChar(_svoCpuData[i].getLeafMask().getValue());
    _attributePointersBundle->putUnsigned(_svoCpuData[i].getAttribPosition());
  }

  // create 3 textures as TBO
  std::cerr << std::endl;
  std::cerr << std::endl << "             Creating child pointers TBO ";

  gloost::Texture* childPointersTexture = new gloost::Texture(_svoChildPointersBundle,
                                                              GL_R32UI,
                                                              GL_DYNAMIC_DRAW,
                                                              GL_TEXTURE_BUFFER);

  _svoChildPointersTexId = gloost::TextureManager::get()->addTexture(childPointersTexture);

  std::cerr << std::endl << "                 texId: " << _svoChildPointersTexId;




  std::cerr << std::endl << "             Creating bitmasks TBO ";

  gloost::Texture* bitMaskTexture = new gloost::Texture(_svoBitMasksBundle,
                                                        GL_RG8,
                                                        GL_DYNAMIC_DRAW,
                                                        GL_TEXTURE_BUFFER);

  _bitMaskTexId = gloost::TextureManager::get()->addTexture(bitMaskTexture);

  std::cerr << std::endl << "                 texId: " << _bitMaskTexId;



  std::cerr << std::endl << "             Creating attrib pointers TBO ";

  gloost::Texture* attributePointersTexture = new gloost::Texture(_attributePointersBundle,
                                                                  GL_R32UI,
                                                                  GL_DYNAMIC_DRAW,
                                                                  GL_TEXTURE_BUFFER);

  _svoAttributePointersTexId = gloost::TextureManager::get()->addTexture(attributePointersTexture);

  std::cerr << std::endl << "                 texId: " << _svoAttributePointersTexId;





  // load and prepare attribute data
	std::string attributeDataPath = basePath + ".ia";

  std::cerr << std::endl;
  std::cerr << std::endl;
  std::cerr << std::endl << "Message from SvoLoader::SvoLoader(const std::string& basePath): ";
  std::cerr << std::endl << "             Loading attributes " + attributeDataPath;

	gloost::InterleavedAttributes* attributes = new gloost::InterleavedAttributes(attributeDataPath);
	attributes->takeReference();
  _attributeData.push_back(attributes);

  std::cerr << std::endl << "Svo attribute layout: " << attributes->getLayoutString();

  gloost::BinaryBundle* attributeBundle = new gloost::BinaryBundle(attributes->getNumPackages() * 8 * sizeof(float));

  std::cerr << std::endl << "Elements/Package: " << attributes->getNumElementsPerPackage();
  std::cerr << std::endl << "Packages:         " << attributes->getNumPackages();
  std::cerr << std::endl << "Package stride    " << attributes->getPackageStride();
  std::cerr << std::endl << "Bundle size:      " << attributes->getNumPackages() * attributes->getPackageStride();


  for (unsigned i=0; i<attributes->getNumPackages(); ++i)
  {

//    std::cerr << std::endl << "i: " << i+1 << " of " << attributes->getNumPackages();

    // normal
    attributeBundle->putFloat( attributes->getVector()[i*6 + 0] );
    attributeBundle->putFloat( attributes->getVector()[i*6 + 1] );
    attributeBundle->putFloat( attributes->getVector()[i*6 + 2] );
    attributeBundle->putFloat( 1.0f );

    // color
    attributeBundle->putFloat( attributes->getVector()[i*6 + 3] );
    attributeBundle->putFloat( attributes->getVector()[i*6 + 4] );
    attributeBundle->putFloat( attributes->getVector()[i*6 + 5] );
    attributeBundle->putFloat( 1.0f );

    if (attributeBundle->getFreeSpace() == 0)
    {
      std::cerr << std::endl;
      std::cerr << std::endl << "attribute bundle is full !!!: ";
      std::cerr << std::endl;
    }


  }

  std::cerr << std::endl << "             Creating attribute TBO ";

  gloost::Texture* attributesTexture = new gloost::Texture(attributeBundle,
                                                            GL_RGBA32F,
                                                            GL_DYNAMIC_DRAW,
                                                            GL_TEXTURE_BUFFER);

  _attributeDataTextureIds.push_back(gloost::TextureManager::get()->addTexture(attributesTexture));

  std::cerr << std::endl << "                 texId: " << _attributeDataTextureIds[0];



}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

SvoLoader::~SvoLoader()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns the serialized svo structure as a vector
  \param   ...
  \remarks ...
*/

std::vector<CpuSvoNode>&
SvoLoader::getSerializedSvo()
{
  return _svoCpuData;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns the serialized svo structure as a vector
  \param   ...
  \remarks ...
*/

const std::vector<CpuSvoNode>&
SvoLoader::getSerializedSvo() const
{
  return _svoCpuData;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   ...
  \param   ...
  \remarks ...
*/

unsigned
SvoLoader::getChildPointersTextureId() const
{
  return _svoChildPointersTexId;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   ...
  \param   ...
  \remarks ...
*/

unsigned
SvoLoader::getBitMasksTextureId() const
{
  return _bitMaskTexId;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   ...
  \param   ...
  \remarks ...
*/

unsigned
SvoLoader::getAttributePointersTextureId() const
{
  return _svoAttributePointersTexId;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   ...
  \param   ...
  \remarks ...
*/

unsigned
SvoLoader::getAttributeTextureId(unsigned id) const
{
  return _attributeDataTextureIds[id];
}

////////////////////////////////////////////////////////////////////////////////





} // namespace svo


