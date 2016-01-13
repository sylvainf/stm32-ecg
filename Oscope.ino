
/*
STM32 Medical monitoring system project (ECG, Puse Oxymeter...)

ILI9341 display code inspired by Ray Burnette Maple-o-scope project and the STM32 community.
*/

#include "Adafruit_ILI9341_STM.h"
#include "Adafruit_GFX_AS.h"
#include <SPI.h>

#include "./firFilter.h"
#include "./Average.h"

Average<int> samplesTableRed(80);
Average<int> samplesTableIR(80);
Average<int> samplesTableECG(80);

int minPulseRed=0;
int maxPulseRed;

int minPulseIR=0;
int maxPulseIR;

int minECG=0;
int maxECG;

float ratio;
float spO2;

// ILI9341 TFT GLCD display connections for hardware SPI
// Signal           Maple Mini         Leonardo      LCD Display    UNO pins
//#define _sclk         6         //         15       J2 pin 7          13
//#define _miso         5 NC      //         14          pin 9          12
//#define _mosi         4         //         16          pin 6          11
#define TFT_CS         13         
#define TFT_DC         12        
#define TFT_RST        14        

#define PORTRAIT 0
#define LANDSCAPE 1

#define LEDRed 10
#define LEDIR 11
#define inputPinspO2 3
float sensorspO2red= 0;
float sensorspO2ir= 0;


// create lcd object
Adafruit_ILI9341_STM TFT = Adafruit_ILI9341_STM(TFT_CS, TFT_DC, TFT_RST); // Using hardware SPI


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
uint16_t signalYspO2red;
uint16_t signalYspO2ir;
uint16_t last;
uint16_t lastspO2red;
uint16_t lastspO2ir;
uint32 lastTime;
uint16_t freq;
uint32 delays;
uint32 newTime;


firFilter FilterRed;
firFilter FilterIR;

void setup() 
{
  pinMode(LEDRed, OUTPUT);
  pinMode(LEDIR,OUTPUT);
  pinMode( 2, INPUT) ; digitalWrite( 2, HIGH);  // turn on pullup resistor
  pinMode(analogInPin, INPUT_ANALOG);                    // Declare the sensorPin as INPUT_ANALOG:
  pinMode(inputPinspO2, INPUT_ANALOG);                    // Declare the sensorPin as INPUT_ANALOG:
  TFT.begin();   
  TFT.setRotation(3);
  myWidth   = TFT.width() ;
  myHeight  = TFT.height();
  maxPulseRed = ANALOG_MAX_VALUE;
  maxPulseIR = ANALOG_MAX_VALUE;
  maxECG=ANALOG_MAX_VALUE;
  clearTFT();
  TFT.setTextSize(2);                           // Small 26 char / line 
  TFT.setTextColor(ILI9341_YELLOW, ILI9341_BLACK) ;
  TFT.print(" ECG starting...") ;
  delay(500) ;
  TFT.fillScreen(ILI9341_BLACK); 
  last=150;
  lastTime=millis();
  delays=0;
  digitalWrite(LEDIR,LOW);
  digitalWrite(LEDRed, HIGH);
  
}


void loop()
{
  int ECGSignal;
  int lastY;
 // TFT.setTextSize(2);
 // TFT.setCursor(200, 10);
 // TFT.print(freq);
 ratio = (log(maxPulseRed-minPulseRed)/log(maxPulseIR-minPulseIR));
  
  TFT.setTextSize(2);
      TFT.setCursor(10, 10);
      TFT.print("         ");
      TFT.setCursor(10, 10);
      TFT.print(ratio);
  for(uint16_t j = 0; j <= myWidth; j++ )
  {
    TFT.drawFastVLine( j+1,  30, myHeight, ILI9341_BLACK);
    
   //Pulse RED
   sensorspO2red = FilterRed.run(redPulseSample());
   //sensorspO2red = redPulseSample();
   samplesTableRed.push(sensorspO2red);
   signalYspO2red = map(sensorspO2red,  minPulseRed-20,  maxPulseRed+20,  myHeight,  150   );
  // TFT.drawLine(j, lastspO2red, j+1, signalYspO2red, ILI9341_RED) ;

   delay(5);
   //ECG 
   ECGSignal=ECGSample();
   samplesTableECG.push(ECGSignal);  
   signalY = map(ECGSignal,  0,  ANALOG_MAX_VALUE,  myHeight-100,  30); 
   TFT.drawLine(j, lastY, j+1, signalY, ILI9341_YELLOW) ;

   
   
   //Pulse IR 
   sensorspO2ir = FilterIR.run(IRPulseSample());
   samplesTableIR.push(sensorspO2ir);  
   signalYspO2ir = map(sensorspO2ir,  minPulseIR-20,  maxPulseIR+20,  myHeight,  150   );
   TFT.drawLine(j, lastspO2ir, j+1, signalYspO2ir, ILI9341_GREEN) ;

   
    if( (last< samplesTableECG.mean()-5) && (ECGSignal
    > samplesTableECG.mean()+5)){
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


    lastY=signalY;
    last=ECGSignal;
    lastspO2red=signalYspO2red;
    lastspO2ir=signalYspO2ir;
    

  }
  
    minPulseRed=samplesTableRed.minimum();
    maxPulseRed=samplesTableRed.maximum();
    minPulseIR=samplesTableIR.minimum();
    maxPulseIR=samplesTableIR.maximum();
    minECG=samplesTableECG.minimum();
    maxECG=samplesTableECG.maximum();
}



void clearTFT()
{
  TFT.fillScreen(BEAM_OFF_COLOUR);                // Blank the display
}


int ECGSample(){     //300micros slot
      int sampleValue=0;
      int nbmes=20;
      for(uint16_t k = 0; k < nbmes; k++ ){
        sampleValue = sampleValue + analogRead(analogInPin);  
        delayMicroseconds(100);
      }
      return sampleValue/nbmes;
}


int redPulseSample(){     //300micros slot
      int sampleValue=0;
      int nbmes=20;
      digitalWrite(LEDIR,LOW);
      digitalWrite(LEDRed, HIGH);
      delayMicroseconds(5);
      for(uint16_t k = 0; k < nbmes; k++ ){
        sampleValue = sampleValue + analogRead(inputPinspO2);  
        delayMicroseconds(100);
      }
      digitalWrite(LEDIR,LOW);
      digitalWrite(LEDRed, LOW);
      return sampleValue/nbmes;
}

int IRPulseSample(){     //300micros slot
      int sampleValue=0;
      int nbmes=20;
      digitalWrite(LEDIR,HIGH);
      digitalWrite(LEDRed, LOW);
      delayMicroseconds(5);
      for(uint16_t k = 0; k < nbmes; k++ ){
        sampleValue = sampleValue + analogRead(inputPinspO2);  
        delayMicroseconds(100);
      }
      digitalWrite(LEDIR,LOW);
      digitalWrite(LEDRed, LOW);
      return sampleValue/nbmes;
}



