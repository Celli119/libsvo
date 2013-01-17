
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



#ifndef H_GLOOST_TIMERLOG
#define H_GLOOST_TIMERLOG



// gloost system includes
#include <gloost/gloostConfig.h>
//#include <gloost/gloostMath.h>

// gloostTest inclues
#include <contrib/Timer.h>


// cpp includes
#include <string>
#include <map>


namespace gloostTest
{


  //  TimerLog

class TimerLog
{
	public:

    // class constructor
    TimerLog();

    // class destructor
	  virtual ~TimerLog();


    // inits the TimerLog
	  void init();


	protected:

   std::map<std::string, int>   _timersForName;
   std::map<std::string, Timer> _sampleCountersForName;


	private:

   // ...

};


} // namespace gloostTest


#endif // GLOOST_TIMERLOG_H


