// IF THE LEDs ARE ACTING REALLY STRANGE FOR SEEMINGLY NEW REASON, TRY A NEW BATTERY
/*
   1/31/23
   Program to drive up to 3 serial chains of RGB LEDs.
   The LEDs are mounted on the Fairport team 578 2023 robot.

   The LEDs are eoocvt WS2811 operating at 12 VDC. The serial signal
   to the LED string(s) is from the Nano 5 VDC internal supply.

   The circuit board has an Arduino Nano which is powered through the Vin pin
   from the 12 VDC supply. The voltage at the Vin pin is dropped to about 9V
   using a series 100 ohm power resistor and a 9.1 V zener diode.  The supply
   is stabilized using a 430 uf capacitor.

   Three PWM pins from the Nano (D6, D5, D3) are connected to 3-pin
   female PWM ouptut connectors using 100 ohm resistors.  The resistors are
   not technically needed but were included to protect the Nano from
   possible wiring abuse.
   
        =============
        LED TEST MODE
        =============
  
   To enter a test mode and test a NeoPixel string, send a serial string to
   the Nano on the USB port, following these rules:

   The first character must be a 't' or a 'T'.  This enters the test mode.
   The second character must be a '0' or '1' or '2' indicating which string will be tested.
   If you want to use the pre-defined NeoPixel count in the program, send the string.
   If you want to modify the pre-defined NeoPixe count,
      the third character must be a space 
      followed by the number of NeoPixels to be tested. 

   The test mode will automatically be exited when the NeoPixel string test is complete.
*/

#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

// What Arduino Nano Pins are the NeoPixels connected to forn each string?

int outputPin[] = {6, 5, 3}; //Data pins on Nano

// How many NeoPixels are attached to the Arduino in each string?
int numPixels[] = {150, 150, 150};

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel lightString_0 = Adafruit_NeoPixel(numPixels[0], outputPin[0], NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel lightString_1 = Adafruit_NeoPixel(numPixels[1], outputPin[1], NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel lightString_2 = Adafruit_NeoPixel(numPixels[2], outputPin[2], NEO_GRB + NEO_KHZ800);

int delayval = 500; // delay milli seconds
int i;

char redValue[4] = {};
char greenValue[4] = {};
char blueValue[4] = {};

//Command from serial port...
char command[12];       //Space for extra characters (only 4-6 are needed)
#define SERIALPOS 9

void fillAll(char* redValue, char* greenValue, char* blueValue);
void fillRainbow();
void setSpecificLED(char* command, char* redValue, char* greenValue, char* blueValue);
void switchGradient(char* command, char* redValue, char* greenValue, char* blueValue);
void getInputColors(char* command, char* redValue, char* greenValue, char* blueValue);
void fillShift(char* command, char* redValue, char* greenValue, char* blueValue);
void shift();
void shiftWrap();
void flash(char* command, char* redValue, char* greenValue, char* blueValue);
//==============================================================
void setup()
//==============================================================
{
  lightString_0.begin(); // This initializes the NeoPixel library.
  lightString_1.begin(); // This initializes the NeoPixel library.
  lightString_2.begin(); // This initializes the NeoPixel library.
  Serial.begin(9600);

  Serial.println("");
  Serial.println("=============================================");

  Serial.println("Program Started");
  Serial.println("Enter 'T'String Number(0, 1, or 2) [Space] Number of NeoPixels");


}

//==============================================================
void loop()         //=============== LOOP =====================
//==============================================================
{
  if (Serial.available())  //A serial string is coming in from the USB Port
  {
    parseSerialCommand();
  }
  //testLEDs(0);
}

//=================  END OF LOOP ===============================



//==============================================================
void parseSerialCommand()
//==============================================================
{
  int testMode = 0;
  int testString = 0;
  int testPixNum = 0;

  delay (50);   //Delay for hardware to accept the complete string
  int availableBytes = Serial.available();

  for (int i = 0; i < availableBytes; i++)
  {
    command[i] = Serial.read();
    command[i + 1] = '\0'; // Append a null
  }
  Serial.println("-----------------------------");
  Serial.print("Command = ");
  Serial.println(command);

  
  if ( strlen(command) < SERIALPOS + 1 )
  {
    return -1;
  }


  if ( atoi(&command[SERIALPOS]) == 255 )
  {
    getInputColors(command, redValue, greenValue, blueValue);
    fillAll(redValue, greenValue, blueValue);
  }
  else if ( atoi(&command[SERIALPOS]) == 254 )
  {
    fillRainbow();      
  }
  else if ( atoi(&command[SERIALPOS]) == 253 )
  {
    switchGradient(command, redValue, greenValue, blueValue);
  }
  else if ( atoi(&command[SERIALPOS]) == 252 )
  {
    memcpy(redValue, &command[0], 3);
    memcpy(greenValue, &command[3], 3);
    memcpy(blueValue, &command[6], 3);   
    fillShift(command, redValue, greenValue, blueValue);
  }
  else if ( atoi(&command[SERIALPOS]) == 251)
  {
    shift();
  }
  else if ( atoi(&command[SERIALPOS]) == 250 )
  {
    shiftWrap();  
  }
  else if ( atoi(&command[SERIALPOS]) == 249 )
  {
    flash(command, redValue, greenValue, blueValue);
  }
  else
  {
    getInputColors(command, redValue, greenValue, blueValue);
    setSpecificLED(command, redValue, greenValue, blueValue);
  }

  //Get the string number to test
  if (testMode)
  {
    Serial.println("Test Mode Entered");
    switch (command[1])         // Second character in command must be 0, 1, or 2
    {
      case '0':
      case '1':
      case '2':
        testString = atoi(&command[1]);  //Test a string of NeoPixels
        Serial.print ("testString = ");
        Serial.println(testString);
        break;
      default://Default Case
        testMode = 0;
        break;
    }
  }


  //Check the space after the string number to test
  if (testMode)
  {
    if (command[2] == ' ')
    {
      Serial.println("Space Found");

      //Find the number of NeoPixels to be tested
      //    int charLen = strlen(command);

      Serial.println("Find NeoPixel Count");
      testPixNum = atoi(&command[3]);
      Serial.print("Number of NeoPixels to Test = ");
      Serial.println(testPixNum);

      if (testPixNum == 0)
      {
        testMode = 0;
      }
      else
      {
        numPixels[testString] = testPixNum;
      }
    }
    else
    {
      Serial.print("Space NOT Found. Testing the default number of ");
      Serial.print(numPixels[testString]);
      Serial.println(" NeoPixels.");
    }
  }

  //Test the Pixel String
  if (testMode)
  {
    testLEDs(testString);

    Serial.println();
    Serial.println("Enter 'T'String Number(0, 1, or 2) [Space] Number of NeoPixels");

  }
}  // ========= End of Serial Command Parsing ===============



//==============================================================
void testLEDs(int stringNum)
//==============================================================
{

  //Test the indicated light string
  //Sequentially test the RED LED's, then the GREEN LED's and last the BLUE LED's

  // lightString_x.Color takes GRB values, from t0 20,0,0 up to 255,255,255
  int brightness = 50;

  for (int y = 0; y < 3; y++)
  {
    for (int x = 0; x < numPixels[stringNum]; x++)
    {
      switch (atoi(&command[1]))
      {
        case 0:
          {
            lightString_0.setPixelColor(x, lightString_0.Color((y == 0)*brightness, (y == 2)*brightness, (y == 1)*brightness)); // Moderately bright G-R-B color.
            lightString_0.show(); // This sends the updated pixel color to the hardware.
            digitalWrite(LED_BUILTIN, x == 0); // turn the LED on if start of the sequence
            delay(delayval); // Delay for a period of time (in milliseconds).
            lightString_0.clear();
            lightString_0.show(); // This sends the updated pixel color to the hardware.
            break;
          }
        case 1:
          {
            //      Serial.print("x = ");
            //    Serial.print(x);
            //      Serial.print("  y = ");
            //    Serial.println(y);
            lightString_1.setPixelColor(x, lightString_1.Color((y == 0)*brightness, (y == 2)*brightness, (y == 1)*brightness)); // Moderately bright G-R-B color.
            lightString_1.show(); // This sends the updated pixel color to the hardware.
            digitalWrite(LED_BUILTIN, x == 0); // turn the LED on if start of the sequence
            delay(delayval); // Delay for a period of time (in milliseconds).
            lightString_1.clear();
            lightString_1.show(); // This sends the updated pixel color to the hardware.
            break;
          }
        case 2:
          {
            lightString_2.setPixelColor(x, lightString_2.Color((y == 0)*brightness, (y == 2)*brightness, (y == 1)*brightness)); // Moderately bright G-R-B color.
            lightString_2.show(); // This sends the updated pixel color to the hardware.
            digitalWrite(LED_BUILTIN, x == 0); // turn the LED on if start of the sequence
            delay(delayval); // Delay for a period of time (in milliseconds).
            lightString_2.clear();
            lightString_2.show(); // This sends the updated pixel color to the hardware.
            break;
          }
      }
    }
    delay(delayval * 2); // Delay for a period of time (in milliseconds).
  }
}

//==============================================================
void clearAndPause(int pauseMS )
//==============================================================
{
  lightString_0.clear();
  lightString_0.show();
  delay(pauseMS);
}


//==============================================================
void displayLine(uint8_t lineNum, uint8_t green, uint8_t red, uint8_t blue)
//==============================================================
{
  // Display a line of one color of pixels from the center to the edge
  // lineNum = 0 for straight down, then counting CCW
  // green = the intensity of the GREEN led range is 0 to 255
  // red = the intensity of the RED led range is 0 to 255
  // blue = the intensity of the BLUE led range is 0 to 255

  // for (int x = 0; x < LINE_LED; x++)
  {

    // lightString_0.Color takes GRB values, from 0,0,0 up to 255,255,255
    //  lightString_0.setPixelColor(line[lineNum][x], pixels.Color(green, red, blue));
  }
}

// ===================================================
void randomSeed()
//==============================================================
{
  // This routine will seed the random number generator
  randomSeed(micros() + millis() + analogRead(0));


}
// ===================================================
int randomNum(int maxNum)
//==============================================================
{
  // This routine will return an integer between 0 and maxNum, inclusive


  int x;

  x = random(0, maxNum + 1); // Generate a random number (from 0 to n-1)...
  /*
    Serial.print("maxNum = ");
    Serial.print(maxNum);
    Serial.print("  LED Selected = ");
    Serial.println(x);
  */
  return x;    //return LED index (1 to maxNum; index 0 is all off)

}
//=========================-Functions-=================================

void fillAll(char* redValue, char* greenValue, char* blueValue)
{
  lightString_0.fill(lightString_0.Color(atoi(&greenValue[0]), atoi(&redValue[0]), atoi(&blueValue[0]), 255), 0, numPixels[0]);
  lightString_0.show();
}
void fillRainbow()
{
  lightString_0.rainbow(0, 10);
  lightString_0.show();
}
void setSpecificLED(char* command, char* redValue, char* greenValue, char* blueValue)
{
  //If only the first LED turns on, a null is being read as a zero
  lightString_0.setPixelColor(atoi(&command[SERIALPOS]), atoi(&greenValue[0]), atoi(&redValue[0]), atoi(&blueValue[0]), 255);
  lightString_0.show();
}
void switchGradient(char* command, char* redValue, char* greenValue, char* blueValue)
{
  char backupCommand[12] = {};
  for ( i = 0; i <= 12; i++ ) {
    backupCommand[i] = command[i];
  }
  //Declare values of what color should end up as
  char redFinal[4] = {};
  char greenFinal[4] = {};
  char blueFinal[4] = {};
  memcpy(redFinal, &backupCommand[0], 3);
  memcpy(greenFinal, &backupCommand[3], 3);
  memcpy(blueFinal, &backupCommand[6], 3);
  //Define value to increment by determined by whether the final value is greater than or less than the current value
  //The first number is a multiplier for how fast you want colors to fade
  //All colors being switched to **MUST** be a multiple of this first number
  int redIncrement = ( atoi(redFinal) - atoi(redValue) ) / abs(atoi(redFinal) - atoi(redValue)) * 5;
  int greenIncrement = ( atoi(greenFinal) - atoi(greenValue) ) / abs(atoi(greenFinal) - atoi(greenValue)) * 5;
  int blueIncrement = ( atoi(blueFinal) - atoi(blueValue) ) / abs(atoi(blueFinal) - atoi(blueValue)) * 5;
  //Check just in case a value is equivalent to would it should end up as
  atoi(redFinal) == atoi(redValue) ? redIncrement = 0: redIncrement = redIncrement;
  atoi(greenFinal) == atoi(greenValue) ? greenIncrement = 0: greenIncrement = greenIncrement;
  atoi(blueFinal) == atoi(blueValue) ? blueIncrement = 0: blueIncrement = blueIncrement;
  i = 0;
  int redColor = atoi(redValue) + ( i * redIncrement);
  int greenColor = atoi(greenValue) + ( i * greenIncrement);
  int blueColor = atoi(blueValue) + ( i * blueIncrement);
  while ( redColor != atoi(redFinal) || greenColor != atoi(greenFinal) || blueColor != atoi(blueFinal) ) {
    i++;      
    if ( redColor != atoi(redFinal) ) {
      redColor = atoi(redValue) + ( i * redIncrement);        
      lightString_0.fill(lightString_0.Color( greenColor, redColor, blueColor), 0, numPixels[0]);
      lightString_0.show();
    }   
    if ( greenColor != atoi(greenFinal) ) {
      greenColor = atoi(greenValue) + ( i * greenIncrement);
      lightString_0.fill(lightString_0.Color( greenColor, redColor, blueColor), 0, numPixels[0]);
      lightString_0.show();
    }
    if ( blueColor != atoi(blueFinal) ) {
      blueColor = atoi(blueValue) + ( i * blueIncrement);
      lightString_0.fill(lightString_0.Color( greenColor, redColor, blueColor), 0, numPixels[0]);
      lightString_0.show();
    }
  }
  //Set values as what color the LEDs have ended up as
  memcpy(redValue, &command[0], 3);
  memcpy(greenValue, &command[3], 3);
  memcpy(blueValue, &command[6], 3);
}
void getInputColors(char* command, char* redValue, char* greenValue, char* blueValue)
{
  memcpy(redValue, &command[0], 3);
  memcpy(greenValue, &command[3], 3);
  memcpy(blueValue, &command[6], 3);
}

void fillShift( char* command, char* redValue, char* greenValue, char* blueValue)
{
  for ( i = 1; i <= numPixels[0]; i++ ) {
    lightString_0.fill(lightString_0.Color(atoi(&greenValue[0]), atoi(&redValue[0]), atoi(&blueValue[0]), 255), 0, i );
    lightString_0.show();
  }
}
void shift()
{
  for ( i = 0; i <= numPixels[0]; i++ ) {
      for ( i = numPixels[0]; i > 0; i-- ) {
        lightString_0.setPixelColor(i, lightString_0.getPixelColor(i - 1));   
      }
      lightString_0.setPixelColor(0, lightString_0.getPixelColor( (numPixels[0]) ));
      lightString_0.show();
      if ( Serial.available() ) {
        break;
      }
      delay(35);
  }  
}
void shiftWrap()
{
  for ( i = 0; i <= numPixels[0]; i++ ) {
      for ( i = numPixels[0]; i > 0; i-- ) {
        lightString_0.setPixelColor(i, lightString_0.getPixelColor(i - 1));   
      }
      lightString_0.setPixelColor(0, lightString_0.getPixelColor( (numPixels[0]) - 1 ));
      lightString_0.show();
      if ( Serial.available() ) {
        break;
      }
      delay(35);
  }
}
void flash(char* command, char* redValue, char* greenValue, char* blueValue)
{
  char redValueNew[4] = {};
  char greenValueNew[4] = {};
  char blueValueNew[4] = {};
  memcpy(redValueNew, &command[0], 3);
  memcpy(greenValueNew, &command[3], 3);
  memcpy(blueValueNew, &command[6], 3);

  while ( ! Serial.available() ) {
    lightString_0.fill(lightString_0.Color(atoi(greenValueNew), atoi(redValueNew), atoi(blueValueNew)), 0, numPixels[0]);
    lightString_0.show();
    if ( Serial.available() ) {
      break;
    }
    delay(650);
    if ( Serial.available() ) {
      break;
    }
    lightString_0.fill(lightString_0.Color(atoi(greenValue), atoi(redValue), atoi(blueValue)), 0, numPixels[0]);
    lightString_0.show();
    delay(650);
  }
}