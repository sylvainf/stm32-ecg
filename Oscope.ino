/*
STM32 Medical monitoring system project (ECG, Puse Oxymeter...)

ILI9341 display code inspired by Ray Burnette Maple-o-scope project and the STM32 community.
*/

#include "Adafruit_ILI9341.h"
#include "Adafruit_GFX.h"
#include <SPI.h>


// ILI9341 TFT GLCD display connections for hardware SPI
// Signal           Maple Mini         Leonardo      LCD Display    UNO pins
//#define _sclk         6         //         15       J2 pin 7          13
//#define _miso         5 NC      //         14          pin 9          12
//#define _mosi         4         //         16          pin 6          11
#define TFT_CS         13         //         10          pin 3          10
#define TFT_DC         12         //          9          pin 5           9
#define TFT_RST        14         //          8          pin 4           8

#define PORTRAIT 0
#define LANDSCAPE 1

// create lcd object
Adafruit_ILI9341 TFT = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); // Using hardware SPI


//void(* resetFunc) (void) = 0; //declare reset function @ address 0

// Analog
const int analogInPin = 9;   // Analog input pin: any of 3, 4, 5, 6, 7, 8, 9, 10, 11



// screen stuff
int myWidth ;
int myHeight ;

// Display colours
#define BEAM1_COLOUR ILI9341_GREEN
#define BEAM2_COLOUR ILI9341_RED
#define GRATICULE_COLOUR 0x07FF
#define BEAM_OFF_COLOUR ILI9341_BLACK
#define CURSOR_COLOUR ILI9341_GREEN

// Analog input
#define ANALOG_MAX_VALUE 4096
float samplingTime = 0;

// Variables for the beam position
uint16_t signalX ;
uint16_t signalY ;
uint16_t signalY1;
uint16_t last;
uint16_t tmpSignal;
uint32 lastTime;
uint16_t freq;
uint32 delays;


void setup() 
{
  
  pinMode( 2, INPUT) ; digitalWrite( 2, HIGH);  // turn on pullup resistor
  pinMode(analogInPin, INPUT_ANALOG);                    // Declare the sensorPin as INPUT_ANALOG:
  TFT.begin();   
  TFT.setRotation(3);
  myWidth   = TFT.width() ;
  myHeight  = TFT.height();
  clearTFT();
  last=150;
  lastTime=millis();
  delays=0;
}


void loop()
{
 // TFT.setTextSize(2);
 // TFT.setCursor(200, 10);
 // TFT.print(freq);
  for(uint16_t j = 0; j <= myWidth; j++ )
  {
    // average mesure against noise
    tmpSignal=0;
    for(uint16_t k = 0; k < 16; k++ ){
      delayMicroseconds(500);
      tmpSignal= tmpSignal+analogRead(analogInPin);
    }
    signalY = map(tmpSignal/16,  1,  4090,  myHeight-1,  1   ) +30;
   TFT.drawFastVLine( j+1,  30, myHeight, ILI9341_BLACK);
   TFT.drawLine(j, last, j+1, signalY, ILI9341_YELLOW) ;
    if(last -signalY > 20){
      newTime=millis();
      delays=newTime-lastTime;
      freq=60000/delays;
      lastTime=millis();
      TFT.setTextSize(2);
      TFT.setCursor(200, 10);
      TFT.print("     ");
      TFT.setCursor(200, 10);
      if(freq <250)
        TFT.print(freq);
    }

    
    last=signalY;

  }
}


void clearTFT()
{
  TFT.fillScreen(BEAM_OFF_COLOUR);                // Blank the display
}
