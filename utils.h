
uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;

//  if( kMatrixSerpentineLayout == false) {
//    i = (y * kMatrixWidth) + x;
//  }

//  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (WIDTH - 1) - x;
      i = (y * WIDTH) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * WIDTH) + x;
    }
//  }

  return i;
}

// Assorted useful functions and variables

// Global variables
boolean effectInit = false; // indicates if a pattern has been recently switched
uint16_t effectDelay = 0; // time between automatic effect changes
unsigned long effectMillis = 0; // store the time of last effect function run
unsigned long cycleMillis = 0; // store the time of last effect change
unsigned long currentMillis; // store current loop's millis value
unsigned long hueMillis; // store time of last hue change
unsigned long eepromMillis; // store time of last setting change
byte currentEffect = 0; // index to the currently running effect
boolean autoCycle = true; // flag for automatic effect changes
boolean eepromOutdated = false; // flag for when EEPROM may need to be updated
byte currentBrightness = BRIGHTNESS; // 0-255 will be scaled to 0-MAXBRIGHTNESS
boolean initialized = false; // switch to true when startup tasks are finished
uint8_t fadeActive = 0;
byte runMode = 0;
byte repCount = 0;
boolean fadingActive = true;

CRGB fadeBaseColor = CRGB::Black;

CRGBPalette16 currentPalette(RainbowColors_p); // global palette storage

typedef void (*functionList)(); // definition for list of effect function pointers
extern byte numEffects;

// Increment the global hue value for functions that use it
byte cycleHue = 0;
byte cycleHueCount = 0;
void hueCycle(byte incr) {
    cycleHueCount = 0;
    cycleHue+=incr;
}

// Set every LED in the array to a specified color
void fillAll(CRGB fillColor) {
  for (byte i = 0; i < NUM_LEDS; i++) {
    leds[i] = fillColor;
  }
}

// Fade every LED in the array by a specified amount
void fadeAll(byte fadeIncr) {
  for (byte i = 0; i < NUM_LEDS; i++) {
    leds[i] = leds[i].fadeToBlackBy(fadeIncr);
  }
}

void fadeTo(CRGB basecolor, byte fadeIncr) {
  for (byte i = 0; i < NUM_LEDS; i++) {
      leds[i] = leds[i].fadeToBlackBy(fadeIncr);
      leds[i] |= basecolor;
  }
}



// Shift all pixels by one, right or left (0 or 1)
//void scrollArray(byte scrollDir) {
//  
//    byte scrollX = 0;
//    for (byte x = 1; x < kMatrixWidth; x++) {
//      if (scrollDir == 0) {
//        scrollX = kMatrixWidth - x;
//      } else if (scrollDir == 1) {
//        scrollX = x - 1;
//      }
//      
//      for (byte y = 0; y < kMatrixHeight; y++) {
//        leds[XY(scrollX,y)] = leds[XY(scrollX + scrollDir*2 - 1,y)];
//      }
//    }
//  
//}


// Pick a random palette from a list
//void selectRandomPalette() {
//
//  switch(random8(8)) {
//    case 0:
//    currentPalette = CloudColors_p;
//    break;
//    
//    case 1:
//    currentPalette = LavaColors_p;
//    break;
//    
//    case 2:
//    currentPalette = OceanColors_p;
//    break;
//    
//    case 4:
//    currentPalette = ForestColors_p;
//    break;
//    
//    case 5:
//    currentPalette = RainbowColors_p;
//    break;
//    
//    case 6:
//    currentPalette = PartyColors_p;
//    break;
//    
//    case 7:
//    currentPalette = HeatColors_p;
//    break;
//  }
//
//}

#define NORMAL 0
#define RAINBOW 1
#define PALETTEWORDS 2
#define HOLLY 3
#define CANDYCANE 4
#define HOLLY2 5

const CRGB candycane[2] = {CRGB::Red, CRGB::Gray};
const CRGB holly[2] = {CRGB::Red, CRGB::Green};

CRGB colorCycle(byte colorGroup) {
  static byte colorIndex = 0;

  switch(colorGroup) {
    case CANDYCANE:
      {
        colorIndex++;
        if (colorIndex > 1) colorIndex = 0;
        return candycane[colorIndex];
      }
      break;

    case HOLLY:
    case HOLLY2:
      {
        colorIndex++;
        if (colorIndex > 1) colorIndex = 0;
        return holly[colorIndex];
      }
      break;

    default:
      return CRGB::Black;
      break;
  }
}

void cyclePattern() {
      cycleMillis = currentMillis;
      if (++currentEffect >= numEffects) currentEffect = 0; // loop to start of effect list
      effectInit = false; // trigger effect initialization when new effect is selected
      fadeActive = 0;
      repCount = 0;
}



// Interrupt normal operation to indicate that auto cycle mode has changed
void confirmBlink() {

  if (autoCycle) { // one blue blink, auto mode active
    fillAll(CRGB::DarkBlue);
    FastLED.show();
    FastLED.delay(200);
    fillAll(CRGB::Black);
    FastLED.delay(200);
  } else { // two red blinks, manual mode active
    fillAll(CRGB::DarkRed);
    FastLED.show();
    FastLED.delay(200);
    fillAll(CRGB::Black);
    FastLED.delay(200);
    fillAll(CRGB::DarkRed);
    FastLED.show();
    FastLED.delay(200);
    fillAll(CRGB::Black);
    FastLED.delay(200);
  }

}

//// Determine flash address of text string
//unsigned int currentStringAddress = 0;
//void selectFlashString(byte string) {
//  currentStringAddress = pgm_read_word(&stringArray[string]);
//}
//
//// Fetch font character bitmap from flash
//byte charBuffer[5] = {0};
//void loadCharBuffer(byte character) {
//  byte mappedCharacter = character;
//  if (mappedCharacter >= 32 && mappedCharacter <= 95) {
//    mappedCharacter -= 32; // subtract font array offset
//  } else if (mappedCharacter >= 97 && mappedCharacter <= 122) {
//    mappedCharacter -= 64; // subtract font array offset and convert lowercase to uppercase
//  } else {
//    mappedCharacter = 96; // unknown character block
//  }
//  
//  for (byte i = 0; i < 5; i++) {
//    charBuffer[i] = pgm_read_byte(Font[mappedCharacter]+i);
//  }
//  
//}
//
//// Fetch a character value from a text string in flash
//char loadStringChar(byte string, byte character) {
//  return (char) pgm_read_byte(currentStringAddress + character);
//}
//
//// write EEPROM value if it's different from stored value
//void updateEEPROM(byte location, byte value) {
//  if (EEPROM.read(location) != value) EEPROM.write(location, value);
//}
//
//void saveEEPROMvals() {
//      updateEEPROM(0, 99);
//      updateEEPROM(1, currentEffect);
//      updateEEPROM(2, autoCycle);
//      updateEEPROM(3, currentBrightness);
//      updateEEPROM(4, runMode);  
//}
//
//// Write settings to EEPROM if necessary
//void checkEEPROM() {
//  if (eepromOutdated) {
//    if (currentMillis - eepromMillis > EEPROMDELAY) {
//      saveEEPROMvals();
//      eepromOutdated = false;
//    }
//  }
//}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generic/shared routine variables ----------------------------------------------------------------------
uint8_t allfreq = 32;                                         // You can change the frequency, thus overall width of bars.
uint8_t bgclr = 0;                                            // Generic background colour
uint8_t bgbri = 0;                                            // Generic background brightness
bool    glitter = 0;                                          // Glitter flag
uint8_t palchg;                                               // 0=no change, 1=similar, 2=random
uint8_t startindex = 0;
uint8_t thisbeat;                                             // Standard beat
uint8_t thisbright = 0;                                       // Standard brightness
uint8_t thiscutoff = 192;                                     // You can change the cutoff value to display this wave. Lower value = longer wave.
uint8_t thisdiff = 1;                                         // Standard palette jump
bool    thisdir = 0;                                          // Standard direction
uint8_t thisfade = 224;                                       // Standard fade rate
uint8_t thishue = 0;                                          // Standard hue
uint8_t thisindex = 0;                                        // Standard palette index
uint8_t thisinc = 1;                                          // Standard incrementer
int     thisphase = 0;                                        // Standard phase change
uint8_t thisrot = 1;                                          // You can change how quickly the hue rotates for this wave. Currently 0.
uint8_t thissat = 255;                                        // Standard saturation
int8_t  thisspeed = 4;                                        // Standard speed change
uint8_t wavebright = 255;                                     // You can change the brightness of the waves/bars rolling across the screen.

uint8_t thathue;                                              // You can change the starting hue for other wave.
uint8_t thisdelay = 0;
uint8_t thatrot;                                              // You can change how quickly the hue rotates for the other wave. Currently 0.
uint8_t allsat;                                               // I like 'em fully saturated with colour.

uint8_t thatdir;
uint8_t alldir;                                               // You can change direction.
 
int8_t thatspeed;                                             // You can change the speed.
int thatphase;                                                // Phase change value gets calculated.
uint8_t thatcutoff;                                           // You can change the cutoff value to display that wave. Lower value = longer wave.
uint8_t fadeval;                                              // Use to fade the led's of course.
long summ=0;
uint8_t colour;                                               // Ripple colour is randomized.
int center = 0;                                               // Center of the current ripple.
int step = -1;                                                // -1 is the initializing step.
uint8_t myfade = 255;                                         // Starting brightness.
#define maxsteps 16                                          // Case statement wouldn't allow a variable.
//uint8_t fadeval = 128;
uint16_t Xorig = 0x012;
uint16_t Yorig = 0x015;
uint16_t X;
uint16_t Y;
uint16_t Xn;
uint16_t Yn;
uint8_t index2;
int wave1=0;                                                  // Current phase is calculated.
int wave2=0;
int wave3=0;

uint8_t lvl1 = 80;                                            // Any result below this value is displayed as 0.
uint8_t lvl2 = 80;
uint8_t lvl3 = 80;

uint8_t mul1 = 7;                                            // Frequency, thus the distance between waves
uint8_t mul2 = 6;
uint8_t mul3 = 5;
uint8_t bgcol = 0;                                            // Background colour rotates.

const bool kMatrixSerpentineLayout = true;

CRGBPalette16 thisPalette;
CRGBPalette16 thatPalette;
TBlendType    currentBlending;       


uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

extern const TProgmemRGBPalette16 NoahsColors_p FL_PROGMEM = {
CRGB::Turquoise,
CRGB::Purple,
CRGB::Teal,
CRGB::Red,
CRGB::Cyan,
CRGB::Orange,
CRGB::Purple,
CRGB::Green,
CRGB::Indigo,
CRGB::Gold,
CRGB::SeaGreen,
CRGB::MidnightBlue,
CRGB::SteelBlue,
CRGB::GreenYellow,
CRGB::SkyBlue,
CRGB::DarkBlue};

//CRGBPalette16 currentPalette(RainbowColors_p); // global palette storage 

#define HOLD_PALETTES_X_TIMES_AS_LONG 1
#define MAX_DIMENSION ((kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight)
uint8_t noise[MAX_DIMENSION][MAX_DIMENSION];
uint8_t colorLoop = 1;
// The 16 bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;

uint16_t speed = 10; // speed is set dynamically once we've started up
uint16_t scale = 40; // scale is set dynamically once we've started up

// Fill the x/y array of 8-bit noise values using the inoise8 function.
void fillnoise8() {
  // If we're runing at a low "speed", some 8-bit artifacts become visible
  // from frame-to-frame.  In order to reduce this, we can do some fast data-smoothing.
  // The amount of data smoothing we're doing depends on "speed".
  uint8_t dataSmoothing = 0;
  if( speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }
  
  for(int i = 0; i < MAX_DIMENSION; i++) {
    int ioffset = scale * i;
    for(int j = 0; j < MAX_DIMENSION; j++) {
      int joffset = scale * j;
      
      uint8_t data = inoise8(x + ioffset,y + joffset,z);

      // The range of the inoise8 function is roughly 16-238.
      // These two operations expand those values out to roughly 0..255
      // You can comment them out if you want the raw noise data.
      data = qsub8(data,16);
      data = qadd8(data,scale8(data,39));

      if( dataSmoothing ) {
        uint8_t olddata = noise[i][j];
        uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
        data = newdata;
      }
      
      noise[i][j] = data;
    }
  }
  
  z += speed;
  
  // apply slow drift to X and Y, just for visual variation.
  x += speed / 8;
  y -= speed / 16;
}

void mapNoiseToLEDsUsingPalette() {
  static uint8_t ihue=0;
  
  for(int i = 0; i < kMatrixWidth; i++) {
    for(int j = 0; j < kMatrixHeight; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's index into the color palette.

      uint8_t index = noise[j][i];
      uint8_t bri =   noise[i][j];

      // if this palette is a 'loop', add a slowly-changing base value
      if( colorLoop) { 
        index += ihue;
      }

      // brighten up, as the color palette itself often contains the 
      // light/dark dynamic range desired
      if( bri > 127 ) {
        bri = 255;
      } else {
        bri = dim8_raw( bri * 2);
      }

      CRGB color = ColorFromPalette( currentPalette, index, bri);
      leds[XY(i,j)] = color;
    }
  }
  
  ihue+=1;
}

void SetupRandomPalette() {
  currentPalette = CRGBPalette16( 
                      CHSV( random8(), 255, 32), 
                      CHSV( random8(), 255, 255), 
                      CHSV( random8(), 128, 255), 
                      CHSV( random8(), 255, 255)); 
}

void SetupBlackAndWhiteStripedPalette() {
  // 'black out' all 16 palette entries...
  fill_solid( currentPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  currentPalette[0] = CRGB::White;
  currentPalette[4] = CRGB::White;
  currentPalette[8] = CRGB::White;
  currentPalette[12] = CRGB::White;

}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette() {
  CRGB purple = CHSV( HUE_PURPLE, 255, 255);
  CRGB green  = CHSV( HUE_GREEN, 255, 255);
  CRGB black  = CRGB::Black;
  
  currentPalette = CRGBPalette16( 
    green,  green,  black,  black,
    purple, purple, black,  black,
    green,  green,  black,  black,
    purple, purple, black,  black );
}
void ChangePaletteAndSettingsPeriodically() {
  uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if( secondHand ==  0)  { currentPalette = RainbowColors_p;         speed = 20; scale = 30; colorLoop = 1; }
    if( secondHand ==  5)  { SetupPurpleAndGreenPalette();             speed = 10; scale = 50; colorLoop = 1; }
    if( secondHand == 10)  { SetupBlackAndWhiteStripedPalette();       speed = 20; scale = 30; colorLoop = 1; }
    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  8; scale =120; colorLoop = 0; }
    if( secondHand == 20)  { currentPalette = CloudColors_p;           speed =  4; scale = 30; colorLoop = 0; }
    if( secondHand == 25)  { currentPalette = LavaColors_p;            speed =  8; scale = 50; colorLoop = 0; }
    if( secondHand == 30)  { currentPalette = OceanColors_p;           speed = 20; scale = 90; colorLoop = 0; }
    if( secondHand == 35)  { currentPalette = PartyColors_p;           speed = 20; scale = 30; colorLoop = 1; }
    if( secondHand == 40)  { SetupRandomPalette();                     speed = 20; scale = 20; colorLoop = 1; }
    if( secondHand == 45)  { SetupRandomPalette();                     speed = 50; scale = 50; colorLoop = 1; }
    if( secondHand == 50)  { SetupRandomPalette();                     speed = 90; scale = 90; colorLoop = 1; }
    if( secondHand == 55)  { currentPalette = RainbowStripeColors_p;   speed = 30; scale = 20; colorLoop = 1; }
  }
}

void resetvars() {                       // Reset the variable back to the beginning.

  thishue = 0;                          // You can change the starting hue value for the first wave.
  thathue = 140;                        // You can change the starting hue for other wave.
  thisrot = 1;                          // You can change how quickly the hue rotates for this wave. Currently 0.
  thatrot = 1;                          // You can change how quickly the hue rotates for the other wave. Currently 0.
  allsat = 255;                         // I like 'em fully saturated with colour.
  thisdir = 0;                          // Change the direction of the first wave.
  thatdir = 0;                          // Change the direction of the other wave.
  alldir = 0;                           // You can change direction.
  thisspeed = 4;                        // You can change the speed, and use negative values.
  thatspeed = 4;                        // You can change the speed, and use negative values.
  allfreq = 32;                         // You can change the frequency, thus overall width of bars.
  thisphase = 0;                        // Phase change value gets calculated.
  thatphase = 0;                        // Phase change value gets calculated.
  thiscutoff = 192;                     // You can change the cutoff value to display this wave. Lower value = longer wave.
  thatcutoff = 192;                     // You can change the cutoff value to display that wave. Lower value = longer wave.
  thisdelay = 10;                       // You can change the delay. Also you can change the allspeed variable above.
  fadeval = 192;                        // How quickly we fade.
  
} // resetvars()

// Increment the global hue value for functions that use it
//byte cycleHue = 0;
//byte cycleHueCount = 0;
//void hueCycle(byte incr) {
//    cycleHueCount = 0;
//    cycleHue+=incr;
//}

void DrawOneFrame( byte startHue8, int8_t yHueDelta8, int8_t xHueDelta8) {
  byte lineStartHue = startHue8;
  for( byte y = 0; y < kMatrixHeight; y++) {
    lineStartHue += yHueDelta8;
    byte pixelHue = lineStartHue;      
    for( byte x = 0; x < kMatrixWidth; x++) {
      pixelHue += xHueDelta8;
      leds[ XY(x, y)]  = CHSV( pixelHue, 255, 255);
    }
  }
}

// Shift all pixels by one, right or left (0 or 1)
void scrollArray(byte scrollDir) {
  
    byte scrollX = 0;
    for (byte x = 1; x < kMatrixWidth; x++) {
      if (scrollDir == 0) {
        scrollX = kMatrixWidth - x;
      } else if (scrollDir == 1) {
        scrollX = x - 1;
      }
      
      for (byte y = 0; y < kMatrixHeight; y++) {
        leds[XY(scrollX,y)] = leds[XY(scrollX + scrollDir*2 - 1,y)];
      }
    }
}

void scrollArray2(byte scrollDir) {
  
    byte scrollY = 0;
    for (byte y = 0; y < kMatrixHeight; y++) {
      if (scrollDir == 0) {
        scrollY = kMatrixHeight - y;
      } else if (scrollDir == 1) {
        scrollY = y;
      }
      
      for (byte x = 0; x < kMatrixWidth; x++) {
        leds[XY(x, scrollY)] = leds[XY(x, scrollY + scrollDir * 2 - 1)];
      }
    }
}

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }
    
// Set every LED in the array to a specified color
//void fillAll(CRGB fillColor) {
//  for (byte i = 0; i < NUM_LEDS; i++) {
//    leds[i] = fillColor;
//  }
//}

// Fade every LED in the array by a specified amount
//void fadeAll(byte fadeIncr) {
//  for (byte i = 0; i < NUM_LEDS; i++) {
//    leds[i] = leds[i].fadeToBlackBy(fadeIncr);
//  }
//}
   
// Pick a random palette from a list
void selectRandomPalette() {

  switch(random8(19)) {
    case 0:
    currentPalette = CloudColors_p;
    break;
    
    case 1:
    currentPalette = LavaColors_p;
    break;
    
    case 2:
    currentPalette = OceanColors_p;
    break;
    
    case 4:
    currentPalette = ForestColors_p;
    break;
    
    case 5:
    currentPalette = RainbowColors_p;
    break;
    
    case 6:
    currentPalette = PartyColors_p;
    break;
    
    case 7:
    currentPalette = HeatColors_p;
    break;
    
    case 8:
    currentPalette = ib_jul01_gp;
    break;
    
    case 9:
    currentPalette = es_vintage_57_gp;
    break;
    
    case 10:
    currentPalette = es_vintage_01_gp;
    break;
    
    case 11:
    currentPalette = es_rivendell_15_gp;
    break;
    
    case 12:
    currentPalette = rgi_15_gp;
    break;
    
    case 13:
    currentPalette = es_emerald_dragon_08_gp;
    break;
    
    case 14:
    currentPalette = Magenta_Evening_gp;
    break;
    
    case 15:
    currentPalette = Sunset_Real_gp;
    break;
    
    case 16:
    currentPalette = BlacK_Blue_Magenta_White_gp;
    break;
    
    case 17:
    currentPalette = BlacK_Magenta_Red_gp;
    break;
    
    case 18:
    currentPalette = BlacK_Red_Magenta_Yellow_gp;
    break;
   
//    case 8:
//    currentPalette = CRGBPalette16(CRGB::LightGrey, CRGB::MidnightBlue, CRGB::DeepPink, CRGB::LimeGreen, CRGB::Aqua, CRGB::Maroon);
//    break;
  }
}

uint8_t maxChanges = 24;
CRGBPalette16 targetPalette(PartyColors_p);

void ChangeMe() {  
  
  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    CRGB p = CHSV( HUE_PURPLE, 255, 255);
    CRGB g = CHSV( HUE_GREEN, 255, 255);
    CRGB u = CHSV( HUE_BLUE, 255, 255);
    CRGB b = CRGB::Black;
    CRGB w = CRGB::White;

    switch(secondHand) {
      case  0: targetPalette = RainbowColors_p; break;
      case  5: targetPalette = CRGBPalette16( u,u,b,b, p,p,b,b, u,u,b,b, p,p,b,b); break;
      case 10: targetPalette = OceanColors_p; break;
      case 15: targetPalette = CloudColors_p; break;
      case 20: targetPalette = LavaColors_p; break;
      case 25: targetPalette = ForestColors_p; break;
      case 30: targetPalette = PartyColors_p; break;
      case 35: targetPalette = CRGBPalette16( b,b,b,w, b,b,b,w, b,b,b,w, b,b,b,w); break;
      case 40: targetPalette = CRGBPalette16( u,u,u,w, u,u,u,w, u,u,u,w, u,u,u,w); break;
      case 45: targetPalette = CRGBPalette16( u,p,u,w, p,u,u,w, u,g,u,w, u,p,u,w); break;
      case 50: targetPalette = CloudColors_p; break;
      case 55: targetPalette = CRGBPalette16( u,u,u,w, u,u,p,p, u,p,p,p, u,p,p,w); break;
      case 60: break;
    }
  }
}

void ChangeMe2() {
  
  uint8_t secondHand = (millis() / 1000) % 60;                // Increase this if you want a longer demo.
  static uint8_t lastSecond = 99;                             // Static variable, means it's only defined once. This is our 'debounce' variable.
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    switch (secondHand) {
      case  0: thisrot = 1; thatrot = 1; thisPalette=PartyColors_p; thatPalette=PartyColors_p; break;
      case  5: thisrot = 0; thatdir = 1; thatspeed = -4; thisPalette=ForestColors_p; thatPalette=OceanColors_p; break;
      case 10: thatrot = 0; thisPalette=PartyColors_p; thatPalette=RainbowColors_p; break;
      case 15: allfreq = 16; thisdir = 1; thathue = 128; break;
      case 20: thiscutoff = 96; thatcutoff = 240; break;
      case 25: thiscutoff = 96; thatdir = 0; thatcutoff = 96; thisrot = 1; break;
      case 30: thisspeed= -4; thisdir = 0; thatspeed= -4; break;
      case 35: thiscutoff = 128; thatcutoff = 128; break;
      case 40: thisspeed = 3; break;
      case 45: thisspeed = 3; thatspeed = -3; break;
      case 50: thisspeed = 2; thatcutoff = 96; thiscutoff = 224; thatspeed = 3; break;
      case 55: resetvars(); break;
      case 60: break;
    }
  }
  
} // ChangeMe2()


//// Determine flash address of text string
//unsigned int currentStringAddress = 0;
//void selectFlashString(byte string) {
//  currentStringAddress = pgm_read_word(&stringArray[string]);
//}
//
//// Fetch font character bitmap from flash
//byte charBuffer[5] = {0};
//void loadCharBuffer(byte character) {
//  byte mappedCharacter = character;
//  if (mappedCharacter >= 32 && mappedCharacter <= 95) {
//    mappedCharacter -= 32; // subtract font array offset
//  } else if (mappedCharacter >= 97 && mappedCharacter <= 122) {
//    mappedCharacter -= 64; // subtract font array offset and convert lowercase to uppercase
//  } else {
//    mappedCharacter = 96; // unknown character block
//  }
//  
//  for (byte i = 0; i < 5; i++) {
//    charBuffer[i] = pgm_read_byte(Font[mappedCharacter]+i);
//  }
//  
//}
//
//// Fetch a character value from a text string in flash
//char loadStringChar(byte string, byte character) {
//  return (char) pgm_read_byte(currentStringAddress + character);
//}
//
//// write EEPROM value if it's different from stored value
//void updateEEPROM(byte location, byte value) {
//  if (EEPROM.read(location) != value) EEPROM.write(location, value);
//}
//
//// Write settings to EEPROM if necessary
//void checkEEPROM() {
//  if (eepromOutdated) {
//    if (currentMillis - eepromMillis > EEPROMDELAY) {
//      updateEEPROM(0, 99);
//      updateEEPROM(1, currentEffect);
//      updateEEPROM(2, autoCycle);
//      updateEEPROM(3, currentBrightness);
//      eepromOutdated = false;
//    }
//  }
//}
