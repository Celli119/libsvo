


#ifndef H_SVO_RENDERPASSANALYSE
#define H_SVO_RENDERPASSANALYSE



// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/Matrix.h>


// cpp includes
#include <string>
#include <vector>


namespace gloost
{
  class PerspectiveCamera;

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
    RenderPassAnalyse( TreeletMemoryManagerCl* memoryManager,
                       unsigned bufferWidth,
                       unsigned bufferHeight);

    // class destructor
	  virtual ~RenderPassAnalyse();

	  // get


	  // performs the analyse pass
	  void performAnalysePass(gloost::PerspectiveCamera* camera,
                            const gloost::Matrix&      modelMatrix,
                            float                      tScaleRatio);




	protected:

   TreeletMemoryManagerCl* _memoryManager;
   unsigned                _bufferWidth;
   unsigned                _bufferHeight;

   std::vector<FeedBackDataElement> _hostSideFeedbackBuffer;
   gloost::gloostId                 _feedbackBufferGid;

	private:

   // ...

};


} // namespace svo


#endif // H_SVO_RENDERPASSANALYSE


