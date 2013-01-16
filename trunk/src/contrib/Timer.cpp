
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



/// gloost system includes
#include <contrib/Timer.h>



/// cpp includes
#include <string>
#include <iostream>
#include <time.h>



namespace gloostTest
{

/**
  \class   Timer

  \brief   A class template because I'm to lazy to write a class from scretch

  \author  Felix Weiszig
  \date    May 2013
  \remarks Lazy mans way to create a new class ;-)
*/

////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

Timer::Timer():
  _time(),
  _lastStart()
{
	reset();
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

Timer::Timer(time_t seconds, long nanoseconds):
  _time(),
  _lastStart()
{
  reset();
  _time.tv_sec  = seconds;
	_time.tv_nsec = nanoseconds;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

Timer::~Timer()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns the messured duration
  \param   ...
  \remarks ...
*/

timespec
Timer::getDuration() const
{
  if (_lastStart.tv_sec == 0 &&_lastStart.tv_nsec == 0)
  {
	  return _time;
  }
  else
  {
    timespec now;
    clock_gettime( CLOCK_REALTIME, &now );
	  return (now - _lastStart);
  }
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns the messured duration in milliseconds
  \param   ...
  \remarks ...
*/

long
Timer::getDurationInMilliseconds() const
{
  timespec tmp = getDuration();
	return (long)tmp.tv_sec*1000 + tmp.tv_nsec/1000000.0;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns the messured duration in microseconds
  \param   ...
  \remarks ...
*/

long
Timer::getDurationInMicroseconds() const
{
  timespec tmp = getDuration();
	return (long)tmp.tv_sec*1000000 + tmp.tv_nsec/1000.0;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   // returns the seconds of the duration
  \param   ...
  \remarks ...
*/

long
Timer::getSeconds() const
{
	return getDuration().tv_sec;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   // returns the seconds of the duration
  \param   ...
  \remarks ...
*/

long
Timer::getNanoseconds() const
{
	return getDuration().tv_nsec;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   start time messurement
  \param   ...
  \remarks ...
*/

void
Timer::start()
{
	clock_gettime( CLOCK_REALTIME, &_lastStart );
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   stop time messurement
  \param   ...
  \remarks ...
*/

void
Timer::stop()
{
  timespec now;
  clock_gettime( CLOCK_REALTIME, &now );
	_time = _time +  (now - _lastStart);

	_lastStart.tv_sec  = 0;
	_lastStart.tv_nsec = 0;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   resets the timer
  \param   ...
  \remarks ...
*/

void
Timer::reset()
{
	_time.tv_sec  = 0;
	_time.tv_nsec = 0;
	_time.tv_sec  = 0;
	_lastStart.tv_nsec = 0;
}
////////////////////////////////////////////////////////////////////////////////


/**
  \brief   normalizes the timespec value
  \param   ...
  \remarks ...
*/

void
Timer::normalize()
{
  if (_time.tv_nsec > 1000000000)
  {
    long unsigned sec = _time.tv_nsec/1000000000;
    _time.tv_nsec -= sec*1000000000;
    _time.tv_sec += sec;
  }
}


////////////////////////////////////////////////////////////////////////////////





} // namespace gloostTest

