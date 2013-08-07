
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



#ifndef H_SVO_SVOLOADER
#define H_SVO_SVOLOADER



// svo system includes
#include <CpuSvoNode.h>


// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/gloostMath.h>


// cpp includes
#include <string>
#include <memory>



namespace gloost
{
  class BinaryBundle;
  class InterleavedAttributes;
}



namespace svo
{


  //  Loads a Svo and Attributes and provides the data as array and as TBOs

class SvoLoader
{
	public:

    // class constructor
    SvoLoader(const std::string& basePath);

    // class destructor
	  ~SvoLoader();


	  // returns the serialized svo structure as a vector
	  std::vector<CpuSvoNode>& getSerializedSvo();
	  const std::vector<CpuSvoNode>& getSerializedSvo() const;


    // svo data
    unsigned getChildPointersTextureId() const;
    unsigned getBitMasksTextureId() const;
    unsigned getAttributePointersTextureId() const;

    // attribute data
    unsigned getAttributeTextureId(unsigned id = 0) const;



	protected:

   std::vector<CpuSvoNode> _svoCpuData;

   gloost::BinaryBundle*   _svoChildPointersBundle;
   gloost::BinaryBundle*   _svoBitMasksBundle;
   gloost::BinaryBundle*   _attributePointersBundle;

   unsigned _svoChildPointersTexId;
   unsigned _bitMaskTexId;
   unsigned _svoAttributePointersTexId;

   std::vector<std::shared_ptr<gloost::InterleavedAttributes> > _attributeData;
   std::vector<unsigned>                                        _attributeDataTextureIds;

};


} // namespace svo


#endif // H_SVO_SVOLOADER


