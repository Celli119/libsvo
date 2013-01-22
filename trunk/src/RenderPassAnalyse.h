


#ifndef H_SVO_RENDERPASSANALYSE
#define H_SVO_RENDERPASSANALYSE



// gloost system includes
#include <gloost/gloostConfig.h>
#include <gloost/Matrix.h>


// cpp includes
#include <string>
#include <vector>
#include <set>
#include <map>


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
	      _errorIfLeafe        = 0.0f;
	      unused               = 0.0f;
	    }

      int      _nodePosOrTreeletGid;
      int      _isLeafeWithSubtreelet;
      float    _errorIfLeafe;
      float    unused;
	  };

	  struct TreeletGidAndError
	  {
      TreeletGidAndError(int treeletGid, float error)
      {
        _treeletGid = treeletGid;
        _error      = error;
      }

      int      _treeletGid;
      float    _error;
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
    std::multiset<TreeletGidAndError>& getVisibleNewTreeletsGids();
    // returns a std::set of TreeletIds belonging inner nodes or final leaves
    std::multiset<TreeletGidAndError>& getVisibleOldTreeletsGids();


	protected:


   TreeletMemoryManagerCl* _memoryManager;
   unsigned                _bufferWidth;
   unsigned                _bufferHeight;

   std::vector<FeedBackDataElement> _hostSideFeedbackBuffer;
   gloost::gloostId                 _feedbackBufferGid;

   std::multiset<TreeletGidAndError> _visibleNewTreeletsGids;
   std::multiset<TreeletGidAndError> _visibleOldTreeletsGids;



   // adds all parent Treelet Gids to the container, replaces error values with bigger ones
   void addNodeAndItsParentTreeletsToVisibleTreelets(gloost::gloostId treeletId,
                                                     float error,
                                                     std::map<gloost::gloostId, float>& treeletGidContainer);


	private:

   //

};

inline bool operator<(const RenderPassAnalyse::TreeletGidAndError &a, const RenderPassAnalyse::TreeletGidAndError &b)
{
  return a._error > b._error;
}

inline bool operator==(const RenderPassAnalyse::TreeletGidAndError &a, const RenderPassAnalyse::TreeletGidAndError &b)
{
  return a._treeletGid == b._treeletGid;
}


} // namespace svo


#endif // H_SVO_RENDERPASSANALYSE


