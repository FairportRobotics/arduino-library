# arduino-library
This a repsoitory of code to control LEDs on a robot. The lights will be controlled with an Arduino, which does so based off information it receives serially from the roboRio

==========

The Arduino reads 4 bytes of info at a time...

ex. '255000000255' or as individual bytes '255 000 000 255'

***Spaces should not be sent in between bytes, they are only used to make reading this file easier***

...which tell it what color the LED strip should change to, and how to change, specifically.

==========

The first 75% of the information would be color, so in our example from before the Arduino reads '255 000 000' as the RGB value, in this case: red.

The last piece of information lets the Arudino what to do with the color it received. The first 150 values, (0-149) are for individually addressing an LED...

ex. '000 255 000 000'

...the last three values are '0', so the LED at index '0' would turn green. The information on what to do is simply read as everything after the color values, so the following examples would be interpreted the same.

ex. '000 255 000 001' is the same as '000 255 000 1'

==========

However, if the last byte of information isn't a value 0-149, it either will quit without doing anything, or run a function associated with that particular value.(Functions that disregard the color information and only care about the last byte are marked with an *). The full list of values with assigned functions are as follows:
	
	 255 - fillAll: this function sets all LEDSs to the specified to color
	*254*- fillRainbow: this function sets all LEDs to a rainbow (regardless of any information that precedes it)
	 253 - switchGradient: this function will gradient the current color of all LEDs to the specified color
	 252 - fillShift: this function changes colors by having the specified color 'slide' into place
	*251*- shift: this function will have whatever LEDs currently lit shift down towards the end of the strip with no wraparound
	*250*- shiftWrap: this function does the exact same as function '251', but with a wraparound
