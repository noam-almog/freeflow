#include "Arduino.h"
#include "FastLED.h"


#define DATA_PIN    7  
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define num_of_leds  300
#define NUM_OF_STATES   9

CRGB F_leds[num_of_leds];
//const int ledsPerStrip = 700;
//DMAMEM int displayMemory[ledsPerStrip*6];
//int drawingMemory[ledsPerStrip * 6];
//const int config = WS2811_GRB | WS2811_800kHz;
//OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);


void rainbow();
void rainbowWithGlitter();
void addGlitter( fract8 chanceOfGlitter); 
void confetti() ;
void confetti2(); 
void sinelon();
void bpm();
void juggle();
void jumping_pole();
void jumping_pole2();
void mellowSagol();


typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, confetti2, jumping_pole2, sinelon, bpm, juggle, jumping_pole };


byte prevState = 1; // uninitiazlied

// SimplePatternList gPatterns = {rainbow,rainbowWithGlitter,confetti,bpm};
// SimplePatternList gPatterns = {mellowSagol};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t glit = 0; // rotating "base color" used by many of the patterns
uint8_t ro = 0;
uint8_t pole = 0;
uint8_t p_hue=130;
bool light = false;


void setup() {
  randomSeed(analogRead(0));
//  leds.begin();
//  leds.show();

  //FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(F_leds, num_of_leds).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
}


void loop() {
// Call the current pattern function once, updating the 'leds' array
  if (!light) {
    gPatterns[gCurrentPatternNumber]();
  } else {
    confetti2();
  }
  
  //convert fast_led calculations into octows driving
//  for (int i = 0; i < num_of_leds; i++) {
//    CRGB fastLedRGB = F_leds[i];
//    int octowsColor = leds.color(fastLedRGB.r, fastLedRGB.g, fastLedRGB.b);
//    leds.setPixel( i, octowsColor);
//  }
  
  // send the 'leds' array out to the actual LED strip
//  leds.show();

  // do some periodic updates
  EVERY_N_MILLISECONDS( 10 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_MILLISECONDS( 10 ) { p_hue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 30 ) { nextRandomPattern(); } // change patterns periodically
  EVERY_N_SECONDS( 30 ) { light=0; } // change patterns periodically
  EVERY_N_SECONDS( 900 ) { light=1; } // change patterns periodically
  EVERY_N_MILLISECONDS( 300 ) { glit= glit+10; } // change patterns periodically
}

void nextRandomPattern() {
  gCurrentPatternNumber = randomNonRepeatingState();
}

byte randomNonRepeatingState() {
  byte randNumber; 
  do
  {
    randNumber = random(1,NUM_OF_STATES + 1); // generate random number between 1 & 5 (minimum is inclusive, maximum is exclusive)
  } while (randNumber == prevState);
  prevState = randNumber;

  return randNumber;
}



void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow(F_leds, num_of_leds, gHue, 3);
  fadeToBlackBy( F_leds, num_of_leds, 200);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(glit);
}


void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    F_leds[ random16(num_of_leds) ] += CRGB::White;
  }
}

void confetti() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( F_leds, num_of_leds, 35);
  int pos = random16(num_of_leds);
  F_leds[pos] += CHSV( gHue + random8(64), 200, 255);
  delay(50);
}

void confetti2() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( F_leds, num_of_leds, 50);
  int pos = random16(num_of_leds);
  if(ro<8)
  {
    for(int i=pos;i<pos+(num_of_leds-random(pos,num_of_leds-(pos)));i++)
  F_leds[i] += CHSV( gHue + random8(64), 0, 160 + random8(90));
  ro=ro+8;
  }
  else
  {
    ro=ro+8;
  }
}

void sinelon() {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( F_leds, num_of_leds, 20);
  int pos = beatsin16( 52, 0, num_of_leds-1 );
  F_leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 60;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < num_of_leds; i++) 
    { //9948
    F_leds[i] = ColorFromPalette(palette, gHue+(i), beat-i*10);
    }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( F_leds, num_of_leds, 10);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    F_leds[beatsin16( i+7, 0, num_of_leds-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
fadeToBlackBy( F_leds, num_of_leds, 10);
  for( int i = 8; i < 0; i++) {
    F_leds[beatsin16( 0, i+7, num_of_leds-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void jumping_pole(){
    CRGBPalette16 palette = CloudColors_p ;
    CRGBPalette16 palette2= RainbowStripeColors_p;
  fill_solid(F_leds,num_of_leds,CHSV(0,0,0));
  for (int i =pole*120; i<(pole+1)*120;i++)
  {
    if(pole%2==0)
    F_leds[i]=ColorFromPalette(palette,gHue,gHue);
    else
    F_leds[i]=ColorFromPalette(palette2,gHue,gHue);
  }
   pole++;
   if(pole==4)
  pole=0;
ro=ro+5;
if(gHue<150)
delay(150-gHue);

}

void jumping_pole2(){

  fill_solid(F_leds,num_of_leds,CHSV(0,0,0));
  for (int i =pole*120; i<(pole+1)*120;i++)
  {
    if(pole%2==0)
    F_leds[i]=CHSV(ro,200,255);
    else
    F_leds[i]=CHSV(255-ro,200,255);
  }
   pole++;
   ro=ro+1;

   if(pole==4)
  pole=0;


delay(80);
}

// void mellowSagol()
// {
 
//   for(int i=0;i<num_of_leds;i++)
//   {
//     F_leds[i]=CHSV(beatsin8( 30, 130, 200), beatsin8( 30, 64, 80),255);
//   }
//   if(p_hue>=200)
//   p_hue=150;
//   delay(20);
// }
