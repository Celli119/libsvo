


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
	      _nodePosOrTreeletGid   = 0;
	      _isLeafeWithSubtreelet = 0;
	      _qualityIfLeafe        = 0.0f;
	      quality2               = 0.0f;
	    }

      int      _nodePosOrTreeletGid;
      int      _isLeafeWithSubtreelet;
      float    _qualityIfLeafe;
      float    quality2;
	  };

	  struct TreeletGidAndQuality
	  {
      TreeletGidAndQuality(int treeletGid, float quality)
      {
        _treeletGid = treeletGid;
        _quality    = quality;
      }

      int      _treeletGid;
      float    _quality;
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
                            float                      tScaleRatio,
                            const gloost::Vector3&     frameBufferFrustumOnePixelWidth,
                            const gloost::Vector3&     frameBufferFrustumOnePixelHeight,
                            unsigned                   frameBufferToFeedbackBufferRatio);


    // returns a std::set of TreeletIds belonging to visible leaves
    std::set<TreeletGidAndQuality>& getVisibleTreeletsGids();

	protected:

   TreeletMemoryManagerCl* _memoryManager;
   unsigned                _bufferWidth;
   unsigned                _bufferHeight;

   std::vector<FeedBackDataElement> _hostSideFeedbackBuffer;
   gloost::gloostId                _feedbackBufferGid;

   std::set<TreeletGidAndQuality>  _visibleTreeletsGids;


	private:

   // ...

};

inline bool operator<(const RenderPassAnalyse::TreeletGidAndQuality &a, const RenderPassAnalyse::TreeletGidAndQuality &b)
{
    return a._quality > b._quality;
}


} // namespace svo


#endif // H_SVO_RENDERPASSANALYSE


