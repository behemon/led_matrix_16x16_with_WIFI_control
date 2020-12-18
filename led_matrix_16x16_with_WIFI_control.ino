// globals
String inString = "";    // string to hold input
#define DATA_PIN D0
#define WIDTH 8
#define HEIGHT 8
#define NUM_LEDS WIDTH * HEIGHT
#define BRIGHTNESS    10
#define FRAMES_PER_SECOND  120
#define kMatrixWidth WIDTH    // for VisualEffect.h as reveriting is annoing
#define kMatrixHeight HEIGHT  // for VisualEffect.h as reveriting is annoing

// Cycle time (milliseconds between pattern changes)
#define cycleTime 15000
// Hue time (milliseconds between hue increments)
#define hueTime 30


// libs
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"         //https://github.com/tzapu/WiFiManager
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include "FastLED.h"
#include "Plasma.cpp"
#include "Spiral.cpp"


// Set web server port number to 80
WiFiServer server(80);

CRGB leds[NUM_LEDS];

int events = 0;
String readString;

//#include <EEPROM.h>
//#include "messages.h"
#include "palettes.h"
//#include "font.h"
//#include "XYmap.h"  // has overlapping parameters for the LEDS before enabling fix the leds defenition
#include "utils.h"
#include "FireworksXY.h"
#include "VisualEffects.h"
#include "VisualEffectsTest.h"

//functionList effectList[] = { sideRain,
//                              colorFill,
//                              barfight,
//                              candycaneSlantbars,
//                              checkerboard,
////                              snow, // crash
////                              xmasThreeDee,
////                              giftbox, // needs rework for lower pixel count
////                              fireworks, // crash
//                              plasma,
//                              slantBars,
//                              threeSine,
//                              confetti,
//                              rider,
//                              glitters,
//                              spinPlasma
//                            };
//

// fun1 // 3 waves that work too fast on 8x8
functionList effectList[] = { 
//                              fun1, // 3 waves that work too fast on 8x8
//                              starfield, // nice stars on the pallet
//                              rain // same as side rain
//                              rainbowStripes
//                              radiate
//                              sinelon
//                              juggle
//                              Fire2012WithPalette
//                              plasma2
//                              bpm
//                              radiate2
//                              slantBars2
//                              slantBars3
//                              spinPlasma2 // nice config of spin plasma
//                              upvote //upvote sign green arrow pointing up
//                              three_sin
//                              three_sinVert
//                              crazy2
//                              colorRotation
//                              vertThreeSine
//                              threeSine2
//                              amazing //static rainbow 45deg
//                              blendme
//                              inoise8_fire
//                              two_sin // not working
//                              twoSin // not working
//                              ripple
//                              serendipitous
                              amazingNoise // +++ so good !!!
//                              colorSnake
//                              tungsten // first led is white
//                              overcastSky // first led is white
                              
                            };

                        
byte numEffects = (sizeof(effectList)/sizeof(effectList[0]));

void setup()
{
  Serial.begin(115200);
  Serial.println();
  
  WiFiManager wifiManager;

  wifiManager.autoConnect();

  server.begin();
  Serial.println("Server started");

//    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  if (currentEffect > (numEffects - 1)) currentEffect = 0;
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients
  currentMillis = millis(); // save the current timer value

//  if (client) {
//    while (client.connected()) {
//      if (client.available()) {
//        char c = client.read();
//
//        //read char by char HTTP request
//        if (readString.length() < 100) {
//
//          //store characters to string 
//          readString += c; 
//          //Serial.print(c);
//        } 
//
//        //if HTTP request has ended
//        if (c == '\n') {
//
//          ///////////////
//          Serial.println(readString); //print to serial monitor for debuging 
//
//          client.println("HTTP/1.1 200 OK");
//          client.println("Content-Type: text/html");
//          client.println("Connection: close");
//          client.println("Refresh: 5");
//
//          client.println("<HTML>");
//          client.println("<HEAD>");
//          client.println("<TITLE>Arduino GET test page</TITLE>");
//          client.println("</HEAD>");
//          client.println("<BODY>");
//
//          client.println("<H1>Zoomkat's simple Arduino button</H1>");
//          
//          // For simple testing, pin 5, 6, 7, and 8 are used in buttons
//          // DIY buttons
//          client.println("<a href=/?on2 >ON</a>"); 
//          client.println("<a href=/?off3 >OFF</a>"); 
//          client.println("&nbsp;<a href=/?off357 >ALL OFF</a><br><br>"); 
//
//          // mousedown buttons
//          client.println("<input type=button value=ON onmousedown=location.href='/?on4'>"); 
//          client.println("<input type=button value=OFF onmousedown=location.href='/?off5'>");        
//          client.println("&nbsp;<input type=button value='ALL OFF' onmousedown=location.href='/?off3579'><br><br>");        
//                   
//          // mousedown radio buttons
//          client.println("<input type=radio onmousedown=location.href='/?on6'>ON</>"); 
//          client.println("<input type=radio onmousedown=location.href='/?off7'>OFF</>"); 
//          client.println("&nbsp;<input type=radio onmousedown=location.href='/?off3579'>ALL OFF</><br><br>");    
//   
//          
//          // custom buttons
//          client.print("<input type=submit value=ON style=width:100px;height:45px onClick=location.href='/?on8'>");
//          client.print("<input type=submit value=OFF style=width:100px;height:45px onClick=location.href='/?off9'>");
//          client.print("&nbsp;<input type=submit value='ALL OFF' style=width:100px;height:45px onClick=location.href='/?off3579'>");
//
//          client.println("</BODY>");
//          client.println("</HTML>");
//
//          delay(1);
//          //stopping client
//          client.stop();
//
//           //controls the Arduino if you press the buttons
//           if (readString.indexOf('2') >0){
//              events = 0;
//              Serial.println("0");
//           }
//           if (readString.indexOf("3") >0){
//              events = 1;
//              Serial.println("1");
//           }
//           if (readString.indexOf("4") >0){
//              events = 2;
//              Serial.println("2");
//           }
//           
////            switch (events){
////              case 0: 
////                demo();
////                break;
////              case 1: 
////                doPlasma();
////                break;
////              case 2: 
////                doSpiral();
////                break;
////            }
//          //clearing string for next read
//          readString="";
//        }
//      }
//    }
////    client.stop();
//    Serial.println("Client disconnected.");
//    Serial.println("");
//  }

//  demo(); // runs the VisualEffect list of patternts
//  doPlasma();
//  doSpiral();

switch (events){
  case 0: 
    demo();
    break;
  case 1: 
    doPlasma();
    break;
  case 2: 
    doSpiral();
    break;
  case 3: 
    doSpiral();
    break;
}

  LEDS.show();
//  FastLED.show();
//  FastLED.delay(1000/120);
}

void doPlasma() {
    Plasma plasma(leds, WIDTH, HEIGHT);
    plasma.start();
}

void doSpiral() {
    Spiral spiral(leds, WIDTH, HEIGHT);
    spiral.start();
}

void demo(){
  // switch to a new effect every cycleTime milliseconds
  if (currentMillis - cycleMillis > cycleTime && autoCycle == true) {
    cycleMillis = currentMillis;
    if (++currentEffect >= numEffects) currentEffect = 0; // loop to start of effect list
    effectInit = false; // trigger effect initialization when new effect is selected
  }

  // increment the global hue value every hueTime milliseconds
  if (currentMillis - hueMillis > hueTime) {
    hueMillis = currentMillis;
    hueCycle(1); // increment the global hue value
  }

  // run the currently selected effect every effectDelay milliseconds
  if (currentMillis - effectMillis > effectDelay) {
    effectMillis = currentMillis;
    effectList[currentEffect](); // run the selected effect function
    random16_add_entropy(1); // make the random values a bit more random-ish
  }

  // run a fade effect too if the confetti effect is running
  if (effectList[currentEffect] == confetti) fadeAll(1);


}
