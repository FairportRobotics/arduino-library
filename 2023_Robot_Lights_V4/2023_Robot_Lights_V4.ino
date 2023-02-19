// IF THE LEDs ARE ACTING REALLY STRANGE FOR SEEMINGLY NO REASON, TRY A NEW BATTERY
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
void test();
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
}

//==============================================================
void loop()         //=============== LOOP =====================
//==============================================================
{
  if (Serial.available())  //A serial string is coming in from the USB Port
  {
    parseSerialCommand();
  }
}
//==============================================================



//==============================================================
void parseSerialCommand()
//==============================================================
{
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

  switch ( atoi(&command[SERIALPOS]) ) {
    case 150:
      test();
      break;
    case 249:
      flash(command, redValue, greenValue, blueValue);
      break;
    case 250:
      shiftWrap();
      break;   
    case 251:
      shift();
      break;
    case 252:
      memcpy(redValue, &command[0], 3);
      memcpy(greenValue, &command[3], 3);
      memcpy(blueValue, &command[6], 3);   
      fillShift(command, redValue, greenValue, blueValue);
      break;
    case 253:
      switchGradient(command, redValue, greenValue, blueValue);
      break;    
    case 254:
      fillRainbow();
      break;
    case 255:
      getInputColors(command, redValue, greenValue, blueValue);
      fillAll(redValue, greenValue, blueValue);
      break;
    default:     
      break;
  }
  if ( atoi(&command[SERIALPOS]) < 150 ) {
      getInputColors(command, redValue, greenValue, blueValue);
      setSpecificLED(command, redValue, greenValue, blueValue);    
  }
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

void test()
{
  /*
  //Test fillAll
  char command[] = "255000000255";
  getInputColors(command, redValue, greenValue, blueValue);
  fillAll(redValue, greenValue, blueValue);
  delay(1000);
  //Test setSpecificLED
  char command[] = "2552552550";
  getInputColors(command, redValue, greenValue, blueValue);
  setSpecificLED(command, redValue, greenValue, blueValue); 
  delay(650);
  command[12] = "0002550001";
  getInputColors(command, redValue, greenValue, blueValue);
  setSpecificLED(command, redValue, greenValue, blueValue); 
  delay(650);
  command[12] = "0000002552";
  getInputColors(command, redValue, greenValue, blueValue);
  setSpecificLED(command, redValue, greenValue, blueValue); 
  delay(1000);
  //Test rainbow and shift functions
  fillRainbow();
  delay(650);
  shift();
  delay(650);
  fillRainbow();
  shiftWrap();
  */
}