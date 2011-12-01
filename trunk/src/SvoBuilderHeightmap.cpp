
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



/// gloost system includes
#include <SvoBuilderHeightmap.h>
#include <Svo.h>


/// gloost system includes
#include <TextureManager.h>


/// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/**
  \class SvoBuilderHeightmap

  \brief generates the svo from heightmap and a colormap

  \author Felix Weiszig
  \date   December 2009
  \remarks
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class constructor

  \remarks ...
*/

SvoBuilderHeightmap::SvoBuilderHeightmap()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Class destructor
  \remarks ...
*/

SvoBuilderHeightmap::~SvoBuilderHeightmap()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief Inits the SvoBuilderHeightmap
  \param ...
  \remarks ...
*/

Svo*
SvoBuilderHeightmap::build( unsigned int heightMapId,
                            unsigned int colorMapId,
                            unsigned int maxDepth)
{

  gloost::mathType heightScale = 0.1;



	gloost::Texture* heightMap = gloost::TextureManager::getInstance()->getTextureWithoutRefcount(heightMapId);
	gloost::Texture* colorMap  = gloost::TextureManager::getInstance()->getTextureWithoutRefcount(colorMapId);


  gloost::mathType maxDimension = gloost::max(heightMap->getWidth(), heightMap->getHeight());
  gloost::mathType texelSize    = 1.0/maxDimension;
  unsigned int     depth        = 1;


  while (pow(2, -(int)depth) > texelSize)
  {
    ++depth;
  }


  if (depth > 9)
  {
    depth = 9;
  }


  gloost::mathType voxelsize = pow(2, -(int)depth);


  Svo* svo = new Svo(depth);


  std::cerr << std::endl;
  std::cerr << std::endl << "maxDimension: " << maxDimension;
  std::cerr << std::endl << "texelSize:    " << texelSize;
  std::cerr << std::endl << "voxelsize:    " << voxelsize;
  std::cerr << std::endl << "depth:        " << depth;



  int imageWidth  = heightMap->getWidth();
  int imageHeight = heightMap->getHeight();


//  int numSamplesH = 1.0/heightMap->getWidth();
//  int numSamplesV = 1.0/heightMap->getHeight();



  for (int v=1; v!=imageHeight-1; ++v)
  {
    for (int u=1; u!=imageWidth-1; ++u)
    {

      gloost::vec4 heightColor   = heightMap->getPixelColor(u, v);
      gloost::mathType height    = heightColor.r*0.3 + heightColor.g*0.59 + heightColor.b*0.11;
      height -= 0.5;
      height *= heightScale;

      gloost::mathType voxelPosX = (1.0/imageWidth) * u - 0.5;
      gloost::mathType voxelPosZ = (1.0/imageHeight) * v - 0.5;

      SvoNode* newNode = svo->insert(gloost::Point3(voxelPosX, height, voxelPosZ));

      // push new leaf attributes to the attribute buffer
      if (newNode)
      {
        newNode->setAttribPosition(svo->getCurrentAttribPosition());


        // calculate gradient
        gloost::vec4 leftHeightColor     = heightMap->getPixelColor(u-1, v);
        gloost::mathType leftHeightValue = leftHeightColor.r*0.3 + leftHeightColor.g*0.59 + leftHeightColor.b*0.11;
        leftHeightValue -= 0.5;
        leftHeightValue *= heightScale;

        gloost::vec4 rightHeightColor    = heightMap->getPixelColor(u+1, v);
        gloost::mathType rightHeightValue = rightHeightColor.r*0.3 + rightHeightColor.g*0.59 + rightHeightColor.b*0.11;
        rightHeightValue -= 0.5;
        rightHeightValue *= heightScale;

        gloost::mathType diffH = leftHeightValue-rightHeightValue;
        gloost::Vector3  vecH(diffH, 0.0, 0.0);


        gloost::vec4 upHeightColor       = heightMap->getPixelColor(u, v+1);
        gloost::mathType upHeightValue   = upHeightColor.r*0.3 + upHeightColor.g*0.59 + upHeightColor.b*0.11;
        upHeightValue -= 0.5;
        upHeightValue *= heightScale;

        gloost::vec4 downHeightColor     = heightMap->getPixelColor(u, v-1);
        gloost::mathType downHeightValue = downHeightColor.r*0.3 + downHeightColor.g*0.59 + downHeightColor.b*0.11;
        downHeightValue -= 0.5;
        downHeightValue *= heightScale;

        gloost::mathType diffV = upHeightValue-downHeightValue;
        gloost::Vector3  vecV(0.0, 0.0, diffV);

//        gloost::Vector3 normal = cross(vecV, vecH);
//        normal.normalize();
        gloost::Vector3 normal(diffH, 0.333, diffV);
        normal.normalize();



        svo->pushAttributeComponent(normal[0]);
        svo->pushAttributeComponent(normal[1]);
        svo->pushAttributeComponent(normal[2]);

        gloost::vec4 color = colorMap->getPixelColor(u, v);
        svo->pushAttributeComponent(color.r);
        svo->pushAttributeComponent(color.g);
        svo->pushAttributeComponent(color.b);


        // fill holes
        gloost::mathType lowesNeightbourHeight = gloost::min( gloost::min(leftHeightValue,rightHeightValue),
                                                              gloost::min(upHeightValue,downHeightValue) );

        if (lowesNeightbourHeight < height)
        {
          int numFillVoxels = ((height - lowesNeightbourHeight)/voxelsize) + 1;

          for (unsigned int i=0; i!=numFillVoxels+1; ++i)
          {

            SvoNode* fillNode = svo->insert(gloost::Point3(voxelPosX, lowesNeightbourHeight+i*voxelsize, voxelPosZ));

            if (fillNode)
            {
              fillNode->setAttribPosition(newNode->getAttribPosition());
            }
          }
        }


      }

    }
  }



#ifndef GLOOST_SYSTEM_DISABLE_OUTPUT_MESSAGES
  std::cerr << std::endl << "               Number of leaves:          " << svo->getNumLeaves();
  std::cerr << std::endl << "               Number of nodes:           " << svo->getNumNodes();
  std::cerr << std::endl << "               Number of OOB Points:      " << svo->getNumOutOfBoundPoints();
  std::cerr << std::endl << "               Number of double Points:   " << svo->getNumDoublePoints();
  std::cerr << std::endl << "               Octree memory real CPU:    " << svo->getNumNodes()*sizeof(svo::SvoNode)/1024.0/1024.0 << " MB";
  std::cerr << std::endl;
  std::cerr << std::endl << "             Creating attributes for inner nodes: ";
#endif

  svo->generateInnerNodesAttributes(svo->getRootNode());

#ifndef GLOOST_SYSTEM_DISABLE_OUTPUT_MESSAGES
  std::cerr << std::endl << "               Octree memory serialized:  " << svo->getNumNodes()*svo::SvoNode::getSerializedNodeSize()/1024.0/1024.0 << " MB";
  std::cerr << std::endl << "               Attribs memory serialized: " << svo->getCurrentAttribPosition()*sizeof(float)/1024.0/1024.0 << " MB";
//  std::cerr << std::endl << "               Number of one-child-nodes: " << _numOneChildNodes << " ( " << (100.0f*_numOneChildNodes)/(float)getNumNodes() << " % )";
  std::cerr << std::endl;

#endif



  return svo;

}




////////////////////////////////////////////////////////////////////////////////





} // namespace svo


