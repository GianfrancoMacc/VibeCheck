/*Vibe Check two way communication via BT and analysis of biometrics
   CREATED BY: Gianfranco Maccagnan
   SUBMISSION BY: Gianfranco Maccagnan
   CREDITS: GSR code by Dunya Kirkali,
   Interaction Design Studies 2 - Vibe Check
*/


//Resources:
//Adafruit NeoPixel Library  https://github.com/adafruit/Adafruit_NeoPixel
//Pulse sensor library    https://pulsesensor.com/pages/installing-our-playground-for-pulsesensor-arduino
//Easy Transfer library  https://github.com/madsci1016/Arduino-EasyTransfer
//DHT library  https://github.com/adafruit/DHT-sensor-library
//GSR code http://ftmedia.eu/diy-gsr-sensor/



////////////////////////////////////////  LIBRARIES  //////////////////////////////////////////////////////////
//#include <Adafruit_NeoPixel.h> //NeoPixel control library
#include <FastLED.h>

//Pulse sensor
#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math.
#include <PulseSensorPlayground.h>    //Pulse sensor

//Tranfer libraries
#include <Wire.h>
#include <EasyTransfer.h> 

//DHT Temp/Humidity sensor
#include "DHT.h"

///////////////////////////////////////  VARIABLES & OBJECTS  //////////////////////////////////////////////////////////


//Vibration Motor
int heartMotor = 3;


//Pulse Sensor variables
const int PulseWire = A0;      //Pulse sensor data pin
const int LED13 = 13;          //The on-board Arduino LED, close to PIN 13.
int Threshold = 640;           //Determining the threshold for the pulse sensor to ignore noise 
                               

PulseSensorPlayground pulseSensor;  //create instance for the pulse sensor


//initialize DHT
#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 2        // DHT data pin
DHT dht(DHTPIN, DHTTYPE);


//GSR
// GSR sensor variables
int sensorPin = A1; //data pin for the GSR
int sensorValue;    //readings from GSR

// Time variables for GSR
unsigned long time;
int secForGSR;
int curMillisForGSR;
int preMillisForGSR;

//Fast LED variables
#define LED_PIN 5
#define NUM_LEDS 31
#define BRIGHTNESS  255
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100
CRGB leds[NUM_LEDS];
TBlendType    currentBlending;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//data structure for transfer
struct ALPHA_DATA_STRUCTURE //alpha garment structure
{
  float alphaBPM ;
};


struct BETA_DATA_STRUCTURE //beta garment structure
{
  float betaBPM;
};

//variables to update BPMs 
int alphaBPMupdate;
int betaBPMupdate;

//counter for sending data on a timer
int counter = 0;

//reference names for the data structures
ALPHA_DATA_STRUCTURE alphaData;
BETA_DATA_STRUCTURE betaData;


EasyTransfer ETalpha, ETbeta; //two EasyTransfer objects, one to send & one to receive

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600); //Baudrate of the Bluetooth modules
  //output for the vibration motors
  pinMode(heartMotor,OUTPUT);

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
    Serial.println("We created a pulseSensor Object !");  //One time pop-up for pulse set up     
  }

  
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  //initializing data structures for EasyTransfer
  ETalpha.begin(details(alphaData), &Serial);
  ETbeta.begin(details(betaData), &Serial);
  
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  time = millis();//millis time for gsr


  //if data from alpha garment received then:
  if(ETalpha.receiveData())
  {
    //receive data from alpha garment & store it in the updateAlpha variable to update BPM
    alphaBPMupdate = alphaData.alphaBPM;
  }



  //add to the counter  
  counter ++;
  
  if(counter == 40)//Send data once every 30 seconds
  {
   //send betaBPM to alpha garment
   betaData.betaBPM = pulseSensor.getBeatsPerMinute();
    
   //send the data object
   ETbeta.sendData();
   counter = 0; //reset counter for the timer to send data
  }


//////////////////////////////////////////////////////////////////////////////


  //GSR Calculation
   curMillisForGSR = time / (secForGSR * 1000);
  if(curMillisForGSR != preMillisForGSR) {
  // Read GSR sensor and send over Serial port
    sensorValue = analogRead(sensorPin);

    preMillisForGSR = curMillisForGSR;
  }


  ////////////////////////////////////DHT variables in Celsius//////////////////////////////
  
  float humidity = dht.readHumidity();
  float temp = dht.readTemperature();

  ////////////////////////////////////////FAST LED//////////////////////////////////////////////

  int colorValue = map(sensorValue, 0 , 152, 255,0);// remaping GSR readings to the range of RGB LEDs
  int colorValueTemp = map(temp, 18 , 30, 200, 255);// remaping GSR readings to the range of RGB LEDs

  //display to all LEDs
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB( colorValueTemp, 0, colorValue);
  }
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);


///////////////////////////////////////
  
  



  delay(250);  //delay so the data transfer doesn't freak out 
  heartBeat();

}



//Function to create the heart beat "double-bump" feeling
void heartBeat(){
  int myHeart = alphaBPMupdate / 60; //divide the updated BPM by 60

  digitalWrite(heartMotor,HIGH);
  delay(200);
  digitalWrite(heartMotor,LOW);
  delay(200);
  digitalWrite(heartMotor,HIGH);
  delay(200);
  digitalWrite(heartMotor,LOW);

  delay(myHeart*1000); //multiply for one second to get how many millis of a delay there are in between each heart beat
}

