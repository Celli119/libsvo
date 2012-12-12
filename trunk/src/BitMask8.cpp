
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



// gloost system includes
#include <BitMask8.h>



// cpp includes
#include <string>
#include <iostream>



namespace svo
{

/////////////////////////////////////////////////////


  // class constructor

BitMask8::BitMask8():
  _mask(GLOOST_BITMASK8_ALL_UNSET)
{

}


/////////////////////////////////////////////////////


  // class constructor

BitMask8::BitMask8(unsigned char value):
  _mask(value)
{

}


/////////////////////////////////////////////////////


  // class constructor

BitMask8::BitMask8(const BitMask8& BitMask8):
  _mask(BitMask8._mask)
{

}


/////////////////////////////////////////////////////


  // class destructor

BitMask8::~BitMask8()
{

}


/////////////////////////////////////////////////////

  // returns true if position of the BitMask8 is enabled

bool
BitMask8::getFlag(unsigned int flagPosition) const
{
	flagPosition = 1 << flagPosition;
	return (bool)(_mask & flagPosition);
}


/////////////////////////////////////////////////////


  // enable flags 0..31 of this TravMask


void
BitMask8::setFlag(unsigned int flagPosition, bool value)
{

  flagPosition = 1 << flagPosition;

  if (value)
  {
    _mask = _mask | flagPosition;
  }
  else
  {
    flagPosition = ~flagPosition;
    _mask = _mask & flagPosition;
  }
}


/////////////////////////////////////////////////////


  //  set flags 0..31 of this BitMask8 on more than one flag positions

void
BitMask8::setFlags(const BitMask8& positions, bool value)
{
  if (value)
  {
    _mask = _mask | positions._mask;
  }
  else
  {
    _mask = _mask & positions.inverted()._mask;
  }
}


/////////////////////////////////////////////////////


  // set all flags 0..31 of this BitMask8 on all positions to value

void
BitMask8::setFlags(bool value)
{
  if (value)
  {
    _mask = GLOOST_BITMASK8_ALL_SET;
  }
  else
  {
    _mask = GLOOST_BITMASK8_ALL_UNSET;
  }
}


/////////////////////////////////////////////////////


  // invert the BitMask8

void
BitMask8::invert()
{
	_mask = ~_mask;
}


/////////////////////////////////////////////////////


  // returns a inverted version of this BitMask8

BitMask8
BitMask8::inverted() const
{
	BitMask8 bm( (*this) );
	bm.invert();
	return bm;
}


/////////////////////////////////////////////////////


  // returns the value of the mask as unsigned int

unsigned char
BitMask8::getValue() const
{
  return _mask;
}


/////////////////////////////////////////////////////


bool
BitMask8::operator&(const BitMask8& mask) const
{
  return (_mask & mask._mask); // & mask mask mask          ... maks, aahh mask
}


//////////////////////////////////////////////////////


// assigns a unsigned int value

unsigned int
BitMask8::operator=(unsigned int value)
{
  _mask = value;
  return _mask;
}


//////////////////////////////////////////////////////


  //

/* extern */
std::ostream&
operator<< (std::ostream& os, const BitMask8& BitMask8)
{
  os << "BitMask8( " << std::fixed << BitMask8.getValue() << " )";
  return os;
}


/////////////////////////////////////////////////////

} // namespace svo


