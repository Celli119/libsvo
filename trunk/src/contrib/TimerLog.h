
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


// cpp includes
#include <string>
#include <map>
#include <vector>


namespace gloostTest
{


  //  TimerLog

class TimerLog
{
	public:


    typedef std::map<std::string, std::vector<double> > sampleMap;


    // class destructor
	  virtual ~TimerLog();

    // returns the instance
	  static TimerLog* get();



    // add a timer
    void addTimer(const std::string& name);



    // add a sample
    void putSample(const std::string& name, double value);


    // returns the average value by averaging all current samples
    double getCurrentAverage(const std::string& name) const;

    // returns the average value gained after % numSamples steps
    double getLastAverage(const std::string& name) const;


    // sets the number of samples to record
    void setNumSamples(unsigned num);



    // reset all timers
    void resetAllTimers();

    // returns a map with all timers
    sampleMap& getTimes();



    // inits the TimerLog
	  void init();


	protected:

    unsigned _numSamples;

   sampleMap                       _valuesForName;
   std::map<std::string, unsigned> _insertPositions;
   std::map<std::string, double>   _averageForName;


    // creates average value for a timer name
    double createAverage(const std::string& name) const;


	private:

	  static TimerLog* _instance;

    // class constructor
    TimerLog();

};


} // namespace gloostTest


#endif // GLOOST_TIMERLOG_H


