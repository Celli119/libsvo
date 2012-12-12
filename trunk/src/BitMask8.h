
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



#ifndef H_GLOOST_BITMASK_8
#define H_GLOOST_BITMASK_8



#define GLOOST_BITMASK8_ALL_SET   0xFF
#define GLOOST_BITMASK8_ALL_UNSET 0x00



// gloost system includes



// cpp includes
#include <string>



namespace svo
{


  //  BitMask to hold 8 flags (0..7)

class BitMask8
{
	public:

    // class constructor
    BitMask8();

    // class constructor
    BitMask8(unsigned char value);

    // class constructor
    BitMask8(const BitMask8& bitMask);

    // class destructor
	  ~BitMask8();


	  // returns true if position of the Bitask is enabled
	  bool getFlag(unsigned int flagPosition) const;

	  // set flags 0..7 of this BitMask
	  void setFlag(unsigned int flagPosition, bool value = true);


	  //  set flags 0..7 of this BitMask on more than one flag positions
	  void setFlags(const BitMask8& positions, bool value = true);

	  //  set all flags 0..7 of this BitMask on all positions to value
	  void setFlags(bool value = 1);


	  // invert the BitMask
	  void invert();

	  // returns a inverted version of this BitMask
	  BitMask8 inverted() const;


	  // returns the value of the mask as unsigned int
	  unsigned char getValue() const;


	  // returns true if at least one flag in mask is identical to this one
	  bool operator&(const BitMask8& mask) const;


	  // assigns a unsigned int value
	  unsigned int operator=(unsigned int value);


	private:

	  unsigned char _mask;


};



// ostream operator
extern std::ostream& operator<< (std::ostream&, const BitMask8& bitMask);


} // namespace svo


#endif // H_GLOOST_BITMASK_8



