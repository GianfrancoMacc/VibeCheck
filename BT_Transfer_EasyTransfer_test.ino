//Resources:
//Adafruit NeoPixel Library  https://github.com/adafruit/Adafruit_NeoPixel
//Sparkfun Haptic control library  https://github.com/sparkfun/SparkFun_Haptic_Motor_Driver_Arduino_Library
//Pulse sensor library  
//Easy Transfer library  https://github.com/madsci1016/Arduino-EasyTransfer
//DHT library  https://github.com/adafruit/DHT-sensor-library
//GSR code http://ftmedia.eu/diy-gsr-sensor/



////////////////////////////////////////  LIBRARIES  //////////////////////////////////////////////////////////
//#include <Adafruit_NeoPixel.h> //NeoPixel control library
#include <FastLED.h>

#include <Sparkfun_DRV2605L.h>//haptic driver

//Pulse sensor
#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math.
#include <PulseSensorPlayground.h>    //Pulse sensor

//Tranfer libraries
#include <Wire.h>
#include <EasyTransfer.h> 

//DHT Temp/Humidity sensor
#include "DHT.h"

///////////////////////////////////////  VARIABLES & OBJECTS  //////////////////////////////////////////////////////////

//Haptic variables
int seq = 1;
int wave = 1;
SFE_HMD_DRV2605L HMD; //Create haptic motor driver object 


//Pulse Sensor variables
const int PulseWire = A0;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
const int LED13 = 13;          // The on-board Arduino LED, close to PIN 13.
int Threshold = 640;           // Determine which Signal to "count as a beat" and which to ignore.
                               // Use the "Gettting Started Project" to fine-tune Threshold Value beyond default setting.
                               // Otherwise leave the default "550" value. 


PulseSensorPlayground pulseSensor;  // Creates an instance of the PulseSensorPlayground object called "pulseSensor"

//initialize DHT
#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 2     // Digital pin connected to the DHT sensor
DHT dht(DHTPIN, DHTTYPE);


//GSR
// GSR sensor variables
int sensorPin = A1; // select the input pin for the GSR
int sensorValue; // variable to store the value coming from the sensor

// Time variables for GSR
unsigned long time;
int secForGSR;
int curMillisForGSR;
int preMillisForGSR;



//Fast LED variables
#define LED_PIN 5
#define NUM_LEDS 3
#define BRIGHTNESS  64
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100
CRGB leds[NUM_LEDS];
TBlendType    currentBlending;


//data structure for transfer
struct SEND_DATA_STRUCTURE
{
  float bpm ;
};

struct ACKNOWLEDGE
{
  boolean received = false;
};

int partnerBPM;
int myBPM;
int counter = 0;
SEND_DATA_STRUCTURE data;
ACKNOWLEDGE acknowledge;

EasyTransfer ETin, ETout; //We need two EasyTransfer object, one for the data we send, and one for the data we receive.

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600); //Baudrate of the Bluetooth modules
 
  ETout.begin(details(data), &Serial);
  ETin.begin(details(acknowledge), &Serial);

  //haptic motors /////////////////////////////////////////////////////////////////////////////////
  HMD.begin();
  HMD.Mode(0); // Internal trigger input mode -- Must use the GO() function to trigger playback.
  HMD.MotorSelect(0x36); // ERM motor, 4x Braking, Medium loop gain, 1.365x back EMF gain
  HMD.Library(2); //1-5 & 7 for ERM motors, 6 for LRA motors 

  //GSR
  secForGSR = 1; // How often do we get a GSR reading
  curMillisForGSR = 0;
  preMillisForGSR = -1;
  

  //DHT
  dht.begin();

  //FastLED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  currentBlending = LINEARBLEND;

  //pulse sensor////////////////////////////////////////////////////////////////
  pulseSensor.analogInput(PulseWire);   
  pulseSensor.blinkOnPulse(LED13);       //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);   

  if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset. 

    //confirmation light for heartbeat
    leds[1].setRGB( 255, 0, 0);
    FastLED.show();
    delay(200);
    leds[1] = CRGB :: Black;
    FastLED.show();
    delay(200);
    leds[1].setRGB( 255, 0, 0);
    FastLED.show();
    delay(200);
    leds[1] = CRGB :: Black;
    FastLED.show();


    //heartBeat();
      
  }
  
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CRGBPalette16 currentPalette; //palette selection

void loop() {
  time = millis();//millis time for gsr
  
  if(ETin.receiveData())
  {
    if(acknowledge.received == true) //confirmation of received package
    {
    
    //confirmation light for heartbeat  FOR DEBUGGING  -----
    // REMOVE AFTER CODE IS DONE //////////////////////////////////////////////////////////
    leds[1].setRGB( 0, 255, 0);
    FastLED.show();
    delay(200);
    leds[1] = CRGB :: Black;
    FastLED.show();
    delay(200);
    leds[1].setRGB( 0, 255, 0);
    FastLED.show();
    delay(200);
    leds[1] = CRGB :: Black;
    FastLED.show();
    
    }
  }
  counter ++;



//////////////////////////////////////////////////////////////////////////////


  //GSR Calculation
   curMillisForGSR = time / (secForGSR * 1000);
  if(curMillisForGSR != preMillisForGSR) {
  // Read GSR sensor and send over Serial port
    sensorValue = analogRead(sensorPin);
    //Serial.println(sensorValue);
    preMillisForGSR = curMillisForGSR;
  }


  ////////////////////////////////////DHT variables in Celsius//////////////////////////////
  
  float humidity = dht.readHumidity();
  float temp = dht.readTemperature();



  /////////////////////////////// asign data to the structure /////////////////////////////////////////
  //Serial.println(pulseSensor.getBeatsPerMinute());
  if(counter == 40)//Send data once every 10 seconds
  {

    myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                                  // "myBPM" hold this BPM value now. 
    data.bpm = pulseSensor.getBeatsPerMinute(); //add bpm to "bpm" variable in the data structure
    
   //Serial.println(pulseSensor.getBeatsPerMinute());
  
   ETout.sendData();
   counter = 0; //reset counter for the timer to send data
  }



/////////////////////////////////////Inner Calculations /////////////////////////////
  ////////////////////////////////////////FAST LED//////////////////////////////////////////////

  int colorValue = map(sensorValue, 0 , 152, 255,0);

  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB( 255, colorValue, 221);
  }
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);


///////////////////////////////////////
  
  



  delay(250); //delay so it doesn't freak out 
  acknowledge.received = false; //reset the acknowledgement of data received to set up new package
}




void heartBeat(){
  int myHeart = myBPM / 60;
for (int waveHeart = 0; waveHeart <=1; waveHeart++){
  HMD.Waveform(seq, waveHeart);
  HMD.go();
  delay(200);
  }
  delay(myHeart);
}

