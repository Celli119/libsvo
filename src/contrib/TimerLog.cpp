
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
#include <contrib/TimerLog.h>



/// cpp includes
#include <string>
#include <iostream>



namespace gloostTest
{

/**
  \class   TimerLog

  \brief   A class template because I'm to lazy to write a class from scretch

  \author  Felix Weiszig
  \date    May 2011
  \remarks Lazy mans way to create a new class ;-)
*/

////////////////////////////////////////////////////////////////////////////////


/*static*/ TimerLog* TimerLog::_instance = 0;


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class constructor
  \remarks ...
*/

TimerLog::TimerLog():
    _numSamples(30),
    _valuesForName(),
    _insertPositions(),
    _averageForName()
{
	// insert your code here
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns the instance
  \remarks ...
*/

/*static*/
TimerLog*
TimerLog::get()
{
	if (_instance == 0 )
  {
    _instance = new TimerLog();
  }

  return _instance;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   Class destructor
  \remarks ...
*/

TimerLog::~TimerLog()
{
	_instance = 0;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   sets the number of samples to record
  \param   ...
  \remarks ...
*/

void
TimerLog::setNumSamples(unsigned num)
{
  _numSamples = num;
  resetAllTimers();
}

////////////////////////////////////////////////////////////////////////////////


/**
  \brief   adds a timer
  \param   ...
  \remarks ...
*/

void
TimerLog::addTimer(const std::string& name)
{
	_valuesForName[name]   = std::vector<double>(_numSamples, 0.0);
	_insertPositions[name] = 0u;
	_averageForName[name] = 0u;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   add a time messurement
  \param   ...
  \remarks ...
*/

void
TimerLog::putSample(const std::string& name, double value)
{
  std::map<std::string, unsigned>::iterator pos = _insertPositions.find(name);
  if (pos == _insertPositions.end())
  {
    return;
  }
  _valuesForName[name][_insertPositions[name]] = value;


  unsigned newInsertPos = _insertPositions[name] + 1;
  if (newInsertPos == _numSamples)
  {
    _averageForName[name]  = createAverage(name);
    _insertPositions[name] = 0;
  }
  else
  {
    _insertPositions[name] = newInsertPos;
  }
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns the average value by averaging all current samples
  \param   ...
  \remarks ...
*/

double
TimerLog::getCurrentAverage(const std::string& name) const
{
  std::map<std::string, unsigned>::const_iterator posCounter = _insertPositions.find(name);
  if (posCounter == _insertPositions.end())
  {
    return 0u;
  }

  return createAverage(name);
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns the average value gained after % numSamples steps
  \param   ...
  \remarks ...
*/

double
TimerLog::getLastAverage(const std::string& name) const
{
  std::map<std::string, double>::const_iterator pos = _averageForName.find(name);
  if (pos == _averageForName.end())
  {
    return 0.0;
  }

  return pos->second;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   reset all timers
  \param   ...
  \remarks ...
*/

void
TimerLog::resetAllTimers()
{
  std::map<std::string, std::vector<double> >::iterator it    = _valuesForName.begin();
  std::map<std::string, std::vector<double> >::iterator endIt = _valuesForName.end();

  while (it != endIt)
  {
    it->second = std::vector<double>(_numSamples, 0.0);
  }
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   returns a map with all timers
  \param   ...
  \remarks ...
*/

std::map<std::string, std::vector<double> >&
TimerLog::getTimes()
{
  return _valuesForName;
}


////////////////////////////////////////////////////////////////////////////////


/**
  \brief   creates average value for a timer name
  \param   ...
  \remarks ...
*/

double
TimerLog::createAverage(const std::string& name) const
{
  const std::vector<double>& vec = _valuesForName.find(name)->second;
  double sum = 0;

  for (unsigned i=0; i!=_numSamples; ++i)
  {
    sum += vec[i];
  }
  return sum/_numSamples;
}


////////////////////////////////////////////////////////////////////////////////





} // namespace gloostTest


