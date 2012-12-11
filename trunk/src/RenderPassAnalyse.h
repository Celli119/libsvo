


#ifndef H_SVO_RENDERPASSANALYSE
#define H_SVO_RENDERPASSANALYSE



/// gloost system includes
#include <gloost/gloostConfig.h>
//#include <gloost/gloostMath.h>


/// cpp includes
#include <string>
#include <vector>


namespace gloost
{


}



namespace svo
{

  class TreeletMemoryManagerCl;


  //  Renders the SVO and creates the feedback buffer in the process

class RenderPassAnalyse
{
	public:

	  struct FeedBackDataElement
	  {
	    FeedBackDataElement()
	    {
	      first  = 0u;
	      second = 0u;
	    }

      unsigned first;
      unsigned second;
	  };



    // class constructor
    RenderPassAnalyse(TreeletMemoryManagerCl* memoryManager,
                      unsigned bufferWidth,
                      unsigned bufferHeight,
                      float    tScaleRatio);

    // class destructor
	  virtual ~RenderPassAnalyse();

	  // get





	protected:

   TreeletMemoryManagerCl* _memoryManager;
   unsigned                _bufferWidth;
   unsigned                _bufferHeight;

   float                   _tScaleRatio;

   std::vector<FeedBackDataElement> _hostSideFeedbackBuffer;
   gloost::gloostId                 _feedbackBufferTextureGid;

	private:

   // ...

};


} // namespace svo


#endif // H_SVO_RENDERPASSANALYSE


