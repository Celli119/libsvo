


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
	      _nodeId        = 0;
	      _error         = 0.0f;
	      _subTreeletGid = 0;
	      _tMin          = 0;
	    }
      int      _nodeId;
      float    _error;
      int      _subTreeletGid;
      float    _tMin;
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
                            unsigned                   frameBufferToFeedbackBufferRatio,
                            bool drawOnly = false);


    // returns a std::set of TreeletIds belonging to visible leaves
    std::multiset<TreeletGidAndError>& getVisibleNewTreeletsGids();
    // returns a std::set of TreeletIds belonging inner nodes or final leaves
    std::multiset<TreeletGidAndError>& getVisibleOldTreeletsGids();


    void setEnableRumble(bool onOrOff);

    bool getEnableRumble();


    // returns the test framebuffer gid
    gloost::gloostId getTestFrameBufferGid() const;


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

   bool _rumble;

   gloost::gloostId _testFramebufferTextureId;
   gloost::gloostId _clRenderbufferGid;

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


