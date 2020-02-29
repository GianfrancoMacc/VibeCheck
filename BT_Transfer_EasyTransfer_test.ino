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

//Pulse sensor variables
int seq = 1;
int wave = 1;
SFE_HMD_DRV2605L HMD; //Create haptic motor driver object 


//Pulse Sensor variables
const int PulseWire = 0;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
const int LED13 = 13;          // The on-board Arduino LED, close to PIN 13.
int Threshold = 550;           // Determine which Signal to "count as a beat" and which to ignore.
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

// Time variables
unsigned long time;
int secForGSR;
int curMillisForGSR;
int preMillisForGSR;


//NeoPixel
//#define PIN 6
//#define NUMPIXELS 3
//Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

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

int counter = 0;
SEND_DATA_STRUCTURE data;
ACKNOWLEDGE acknowledge;

EasyTransfer ETin, ETout; //We need two EasyTransfer object, one for the data we send, and one for the data we receive.

///////////////////////////////////////////////////////////////////////////////////////////////////
// Gradient palette "bhw1_01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw1_01_gp ) {
    0, 227,101,  3,
  117, 194, 18, 19,
  255,  92,  8,192};









// Gradient palette "bhw1_w00t_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_w00t.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw1_w00t_gp ) {
    0,   3, 13, 43,
  104,  78,141,240,
  188, 255,  0,  0,
  255,  28,  1,  1};


////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600); //Baudrate of the Bluetooth modules
 
  ETout.begin(details(data), &Serial);
  ETin.begin(details(acknowledge), &Serial);

  //haptic motors /////////////////////////////////////////////////////////////////////////////////
  HMD.begin();
  Serial.begin(9600);
  HMD.Mode(0); // Internal trigger input mode -- Must use the GO() function to trigger playback.
  HMD.MotorSelect(0x36); // ERM motor, 4x Braking, Medium loop gain, 1.365x back EMF gain
  HMD.Library(2); //1-5 & 7 for ERM motors, 6 for LRA motors 

  //GSR
  secForGSR = 1; // How often do we get a GSR reading
  curMillisForGSR = 0;
  preMillisForGSR = -1;
  

  //DHT
  dht.begin();

  //Neopixel
  //pixels.begin(); // This initializes the NeoPixel library.

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
    
      delay(200);
      
  }
  
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CRGBPalette16 currentPalette; //palette selection

void loop() {
  time = millis();
  if(ETin.receiveData())
  {
    if(acknowledge.received == true) //confirmation of received package
    {
      HMD.Waveform(seq, wave);
      HMD.go();

      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
//      pixels.setPixelColor(i, pixels.Color(0,150,0)); // Moderately bright green color.
//
//      pixels.show(); // This sends the updated pixel color to the hardware.
      
    }
  }
  counter ++;

  

//  for (uint8_t i=0; i<NUM_LEDS; i++) {
//    uint8_t paletteIndex = map(i,0,NUM_LEDS-1,0,240);  //index is now based on pixel number
//    leds[i] = ColorFromPalette( myPal, paletteIndex, BRIGHTNESS, LINEARBLEND);
//  }
//  
//  FastLED.show();
//  FastLED.delay(1000);
////////////////////////////////////////FAST LED//////////////////////////////////////////////




//ChangePalettePeriodically();
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    FillLEDsFromPaletteColors( startIndex);
    
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);


//////////////////////////////////////////////////////////////////////////////





  //GSR Calculation
   curMillisForGSR = time / (secForGSR * 1000);
  if(curMillisForGSR != preMillisForGSR) {
  // Read GSR sensor and send over Serial port
    sensorValue = analogRead(sensorPin);
    Serial.println(sensorValue);
    preMillisForGSR = curMillisForGSR;
  }

  if (sensorValue >=65){
    currentPalette = bhw1_w00t_gp;
  }
  if (sensorValue >=80){
    currentPalette = bhw1_01_gp;
  }

  //DHT variables in Celsius
  float humidity = dht.readHumidity();
  float temp = dht.readTemperature();

  /////////////////////////////// asign data to the structure /////////////////////////////////////////
  if(counter == 8)//Send data once every two seconds
  {

    int myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 
    data.bpm = pulseSensor.getBeatsPerMinute(); //add bpm to "bpm" variable in the data structure
   
  
   ETout.sendData();
   counter = 0; //reset counter for the timer to send data
  }



  
  delay(250); //delay so it doesn't freak out 
  acknowledge.received = false; //reset the acknowledgement of data received to set up new package
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}
