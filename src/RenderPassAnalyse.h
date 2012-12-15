


#ifndef H_SVO_RENDERPASSANALYSE
#define H_SVO_RENDERPASSANALYSE



// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/Matrix.h>


// cpp includes
#include <string>
#include <vector>
#include <set>


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
	      _first  = 0u;
	      _second = 0.0f;
	    }

      unsigned _first;
      float    _second;
	  };


    // class constructor
    RenderPassAnalyse( TreeletMemoryManagerCl* memoryManager,
                       unsigned                bufferWidth,
                       unsigned                bufferHeight);

    // class destructor
	  virtual ~RenderPassAnalyse();


	  // performs the analyse pass and read back the feedback buffer
	  void performAnalysePass(gloost::gloostId           deviceGid,
                            gloost::PerspectiveCamera* camera,
                            const gloost::Matrix&      modelMatrix,
                            float                      tScaleRatio);


    // returns a std::set of TreeletIds belonging to visible leaves
    std::set<gloost::gloostId>& getVisibleTreelets();

	protected:

   TreeletMemoryManagerCl* _memoryManager;
   unsigned                _bufferWidth;
   unsigned                _bufferHeight;

   std::vector<FeedBackDataElement> _hostSideFeedbackBuffer;
   gloost::gloostId                 _feedbackBufferGid;

   std::set<gloost::gloostId>       _visibleTreelets;


	private:

   // ...

};


} // namespace svo


#endif // H_SVO_RENDERPASSANALYSE


