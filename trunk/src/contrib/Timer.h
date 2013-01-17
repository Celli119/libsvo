
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



#ifndef H_GLOOST_TIMER
#define H_GLOOST_TIMER



/// gloost system includes
#include <gloost/gloostConfig.h>
//#include <gloost/gloostMath.h>


/// cpp includes
#include <string>
#include <ctime>



namespace gloostTest
{


  //  Timer

class Timer
{
	public:

    // class constructor
    Timer();
    Timer(time_t seconds, long nanoseconds);
    Timer(const timespec& tsp);

    // class destructor
	  virtual ~Timer();


    // returns the messured duration
    timespec getTimeSpec() const;

    // returns the messured duration in milliseconds
    long getDurationInMilliseconds() const;

    // returns the messured duration in microseconds
    long getDurationInMicroseconds() const;

    // returns the seconds of the duration
    long getSeconds() const;

    // returns the nanoseconds of the duration
    long getNanoseconds() const;


    // start time messurement
	  void start();

    // start time messurement
	  void stop();

    // resets the timer
    void reset();


	  // normalizes the timespec value
	  void normalize();

    /// assignment
    const Timer& operator= (const Timer&);



	protected:

   // ...


	private:

    timespec _time;
    timespec _lastStart;

};



// subtraction of timespec
inline timespec operator-(const timespec& time2, const timespec& time1)
{
	timespec temp;
	if ((time2.tv_nsec-time1.tv_nsec) < 0)
  {
		temp.tv_sec  = time2.tv_sec-time1.tv_sec-1;
		temp.tv_nsec = 1000000000+time2.tv_nsec-time1.tv_nsec;
	}
	else
	{
		temp.tv_sec  = time2.tv_sec  - time1.tv_sec;
		temp.tv_nsec = time2.tv_nsec - time1.tv_nsec;
	}
	return temp;
}

// addition of timespec
inline timespec operator+(const timespec& time1, const timespec& time2)
{
	timespec temp;

	temp.tv_sec  = time1.tv_sec  + time2.tv_sec;
	temp.tv_nsec = time1.tv_nsec + time2.tv_nsec;

  if (temp.tv_nsec > 1000000000)
  {
    long sec      = temp.tv_nsec/1000000000;
    temp.tv_nsec -= sec*1000000000;
    temp.tv_sec  += sec;
  }
  return temp;
}



// subtraction of Timer
extern Timer operator-(const Timer& timer2, const Timer& timer1)
{
  return Timer(timer2.getTimeSpec() - timer1.getTimeSpec());
}

// subtraction of Timer
extern Timer operator+(const Timer& timer1, const Timer& timer2)
{
  return Timer(timer1.getTimeSpec() + timer2.getTimeSpec());
}



} // namespace gloostTest


#endif // H_GLOOST_TIMER

