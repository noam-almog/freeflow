#include "Arduino.h"
#include "FastLED.h"

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif
#define DATA_PIN   2
//#define CLK_PIN  4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120
#define NUM_LEDS    690 // todo: revert the number later
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void juggle() ;
void nextPattern();
void rainbow();
void addGlitter( fract8 chanceOfGlitter);
void sinelon();
void pulse();
void bpm();
void rainbowWithGlitter();
void lightning2();
void confetti();
void lightning();
int fill_bit(int start,bool fade,bool dir);
void circle(bool dir,bool fade);
void inner_circle(bool dir,bool fade);
void shell();
void shell2();
void shell3();
void shell4();
int fade_bit(int val,bool dir,bool in,uint8_t hue);

CRGB leds[NUM_LEDS];
byte prevState = 1; // uninitiazlied


void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { lightning,bpm,lightning2};


uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t ro = 0; 
uint8_t outer= 77;
uint8_t val=0;
uint8_t val2=255;
uint8_t GA=0;
bool dir2=false;
bool dir=true;
int spot=0;
int spot2=outer;


void loop() {
  // Call the current pattern function once, updating the 'leds' array
  executeCurrentPatten();

 
  FastLED.show();  
  // insert a delay to keep the framerate modest

  // FastLED.delay(1000/FRAMES_PER_SECOND); 

  // // do some periodic updates
  EVERY_N_MILLISECONDS( 10 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 20 ) { nextPattern(); } // change patterns periodically
  EVERY_N_SECONDS( 5 ) { GA++; } // change patterns periodically
}

void executeCurrentPatten() {
  gPatterns[gCurrentPatternNumber]();
}

void nextPattern() {
  // round robin on all pattens
  gCurrentPatternNumber = randomNonRepeatingState();
}

byte randomNonRepeatingState() {
  byte randNumber;
  do {
    randNumber = random(0, ARRAY_SIZE( gPatterns));
  } while (randNumber == prevState);
  prevState = randNumber;

  return randNumber;
}



void rainbow() {
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 5);
  delay(10);
}

void rainbowWithGlitter() {
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) {
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon() {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, outer-1 );
  int pos2 = beatsin16( 13, outer, NUM_LEDS-1 );

  leds[pos2] += CHSV( gHue, 255, beatsin8( 60, 64, 255));
  leds[pos] += CHSV( gHue+16, 255, beatsin8( 50, 0, 70));
}

void bpm() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 40;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  
  if (GA%2==0) {
    for (int i = 0; i < NUM_LEDS; i++)
      leds[i] = ColorFromPalette(palette, gHue+(i*2), 255);
  } else {
    for (int i = NUM_LEDS-1; i > 0; i--)  
      leds[i] = ColorFromPalette(palette, gHue+(i*2), 255);
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for (int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


void lightning() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 70);
  int pos = random16(0,NUM_LEDS);
  if (ro < 8) {
    //  for(int i=pos;i<pos+(NUM_LEDS-random(pos,NUM_LEDS-(pos)));i++)
  
    for (int i=pos; i<pos+25; i++) {
      if(i<NUM_LEDS)
        leds[i] += CHSV( gHue + random8(64), 0, 160 + random8(90));
      }
  }
  ro=ro+64;

//   else 
//   {
//     ro=ro+8;
//   }

// if(dir2)
// {
//   if (val2<255)
//   val2=fade_bit(val2,1,0,gHue+100);
//   else
//   dir2=false;
// }
// else
// {
//     if (val2>0)
//   val2=fade_bit(val2,0,0,gHue);
//   else
//   dir2=true;
// }

}


void lightning2() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 50);
  int pos = random16(outer,NUM_LEDS);
  if (ro < 32) {
    for(int i=pos;i<pos+(NUM_LEDS-random(pos,NUM_LEDS-(pos)));i++)
      leds[i] += CHSV( gHue + random8(64), 0, 160 + random8(90));
    ro=ro+2;
  } else {
    ro=ro+2;
  }

  if (ro < 8) {
    int po=random8(90);
    for (int i=0; i<5; i++) {
      leds[i] += CHSV( 0, 0, 160 + po);
      leds[i+20]+= CHSV( 0, 0, 160 + po);
      leds[i+40]+= CHSV( 0, 0, 160 + po);
      leds[i+60]+= CHSV(0, 0, 160 + po);
    }
    ro=ro+2;
  } else {
    ro=ro+2;
  }
}


void pulse() {
  for (int i=0;i<NUM_LEDS/4;i++) {
    leds[i] = CHSV( gHue ,255, 255);
    leds[i+NUM_LEDS/4] = CHSV( gHue ,255, 255);
    leds[i+2*NUM_LEDS/4] = CHSV( gHue ,255, 255);
    leds[i+3*NUM_LEDS/4] = CHSV( gHue ,255, 255);

    FastLED.show();  
    fadeToBlackBy(leds,NUM_LEDS,10);
    delay(30);
  }
}

int fill_bit(int start,bool fade,bool dir) {
  leds[start]=CHSV(gHue, 255,255);
  if (fade)
    fadeToBlackBy(leds,NUM_LEDS,10);
  if (dir)
    return start+1;
  else
    return start-1;
}

void circle(bool dir,bool fade) {
   spot=  fill_bit(spot,fade,dir);
    if (dir) {
      spot=spot%outer;
    } else {
      if(spot==-1)
        spot=outer-1;
    }
}

void inner_circle(bool dir,bool fade) {
  spot2=  fill_bit(spot2,fade,dir);
  if (dir) {
    if(spot2==NUM_LEDS)
      spot2=outer;
  } else {
  if (spot2 == outer-1)
    spot2=NUM_LEDS-1;
  }
}

void shell() {
//   inner_circle(0,0);
  circle(0,0);
  delay(30);

  if (dir) {
    if (val < 255)
      val=fade_bit(val,1,1,gHue);
    else
      dir=false;
  } else {
    if (val > 0)
      val=fade_bit(val,0,1,gHue);
    else
    dir=true;
  }
}

void shell2() {
  inner_circle(0,0);
  circle(0,0);
  delay(20);
}

void shell3() {
  //  inner_circle(0,1);
  // circle(0,0);
  delay(20);

  if (dir) {
    if (val<255)
      val=fade_bit(val,1,1,gHue+100);
    else
    dir=false;
  } else {
    if (val > 0)
      val=fade_bit(val,0,1,gHue+150);
    else
      dir=true;
  }

  if (dir2) {
    if (val2<255)
      val2=fade_bit(val2,1,0,gHue);
    else
      dir2=false;
    } else {
      if (val2 > 0)
        val2=fade_bit(val2,0,0,gHue);
      else
        dir2=true;
    }
}

void shell4() {
  inner_circle(0,1);
  // circle(0,0);
  delay(20);

// if(dir)
// {
//   if (val<255)
//   val=fade_bit(val,1,1,gHue);
//   else
//   dir=false;
// }
// else
// {
//     if (val>0)
//   val=fade_bit(val,0,1,gHue);
//   else
//   dir=true;
// }

  if (dir2) {
    if (val2 < 255)
      val2=fade_bit(val2,1,0,gHue+100);
    else
      dir2=false;
  } else {
    if (val2 > 0)
      val2=fade_bit(val2,0,0,gHue);
    else
      dir2=true;
  }
}


int fade_bit(int val,bool dir,bool in,uint8_t hue) {
  if (in) {
    for (int i=outer; i<NUM_LEDS; i++) {
      if (dir)      
        leds[i]=CHSV(hue,255,val+1);
      else
        leds[i]=CHSV(hue,255,val-1);
    }
  } else {
    for (int i=0; i < outer; i++) {
      if (dir)      
        leds[i]=CHSV(hue,255,val+1);
      else
        leds[i]=CHSV(hue,255,val-1);
    }
  }
  if (dir)
    return val+1;
  else 
    return val-1;
}
