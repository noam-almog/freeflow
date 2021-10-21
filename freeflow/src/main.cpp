#include <Arduino.h>
#include <FastLED.h>
#include "OctoWS2811.h"


#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif
#define DATA_PIN   2
//#define CLK_PIN  4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120
#define NUM_LEDS    624 // todo: revert the number later
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define ROW_NUM      6
#define LEDS_PER_ROW   104



DMAMEM int displayMemory[NUM_LEDS*6];
int drawingMemory[NUM_LEDS*6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 ledz(NUM_LEDS, displayMemory, drawingMemory, config);

//#define UP
// i hate git!
//#define DOWN

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

void executeCurrentPatten();
int32_t randomNonRepeatingState();
u_int16_t posFor(u_int16_t column, u_int16_t row);
void runOnFrame(int (*f)());
void paint_pixel(int i);
void fadeFrame(u_int8_t f);
void lightOnePixelAndFadeFrame(u_int16_t x, u_int16_t y, u_int8_t h, u_int8_t s, u_int8_t v,u_int8_t f);
void fadePixel(u_int16_t x, u_int16_t y,u_int8_t f);

void render();

void snakePattern();
void horizontalSnake();
void horizontalDrunkSnake();
void horizontalDrunkSnakeReverse();
void snakePatternReverse();

void frame_and_hori_snake();
void fill_frame(u_int8_t h, u_int8_t s, u_int8_t v);
void fadeRow(u_int16_t y, u_int8_t fadeBy);

void wormPattern();
u_int16_t* wormMove(u_int16_t* location, int16_t* direction, u_int8_t* hsv);

typedef void (*SimplePatternList[])();

CRGB leds[NUM_LEDS];
// List of patterns to cycle through.  Each is defined as a separate function below.
SimplePatternList gPatterns = { snakePatternReverse,wormPattern,snakePattern, bpm, horizontalSnake, horizontalDrunkSnake ,frame_and_hori_snake,  horizontalDrunkSnakeReverse };
// SimplePatternList gPatterns = { lightning,bpm,juggle,rainbow,rainbowWithGlitter,confetti,pulse};





uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t prevState = 1; // uninitiazlied

uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t ro = 0;
uint8_t val=0;
uint8_t val2=255;
uint8_t GA=0;




void setup() {
  Serial.begin(115200);

  
  delay(3000); // 3 second delay for recovery
  randomSeed(analogRead(3));
  // tell FastLED about the LED strip configuration
  // FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  ledz.begin();
  ledz.show();
}

void loop() {
  // Call the current pattern function once, updating the 'leds' array
  executeCurrentPatten();

  // FastLED.show();

  // // do some periodic updates
  EVERY_N_MILLISECONDS( 10 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 40 ) { nextPattern(); } // change patterns periodically
  EVERY_N_SECONDS( 5 ) { GA++; } // change patterns periodically
}

void executeCurrentPatten() {
  gPatterns[gCurrentPatternNumber]();
}

void nextPattern() {
  // round robin on all pattens
  // gCurrentPatternNumber = randomNonRepeatingState();
  gCurrentPatternNumber = (gCurrentPatternNumber+1)%ARRAY_SIZE( gPatterns);
}

int32_t randomNonRepeatingState() {
  byte randNumber;
  do {
    randNumber = random(0, ARRAY_SIZE( gPatterns));
  } while (randNumber == prevState);
  prevState = randNumber;

  return randNumber;
}

u_int16_t posFor(u_int16_t column, u_int16_t row) {
  if ((row % 2) == 0)
  {
    // Serial.println((row * LEDS_PER_ROW) + column);
    return (row * LEDS_PER_ROW) + column;
  }
  else
  {
    // Serial.println((((row + 1) * LEDS_PER_ROW) - column - 1));

    return (((row + 1) * LEDS_PER_ROW) - column - 1);
  }
}

/////////patterns from here on !





void rainbow() {
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 5);
  delay(10);
  render();
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
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
  render();
}

void sinelon() {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, 40-1 );
  int pos2 = beatsin16( 13, 40, NUM_LEDS-1 );

  leds[pos2] += CHSV( gHue, 255, beatsin8( 60, 64, 255));
  leds[pos] += CHSV( gHue+16, 255, beatsin8( 50, 0, 70));
  render();
}

void bpm() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  CRGBPalette16 palette = PartyColors_p;
  // uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  
  if (GA%2 == 0) {
    for (int i = 0; i < NUM_LEDS; i++)
      leds[i] = ColorFromPalette(palette, gHue+(i*2), 255);
  } else {
    for (int i = NUM_LEDS-1; i > 0; i--)  
      leds[i] = ColorFromPalette(palette, gHue+(i*2), 255);
  }
        render();
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for (int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
    render();
  }
}


void lightning() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 70);
  int pos = random16(0, NUM_LEDS);
  if (ro < 8) {
    //  for(int i=pos;i<pos+(NUM_LEDS-random(pos,NUM_LEDS-(pos)));i++)
  
    for (int i=pos; i < pos+25; i++) {  
      leds[i % NUM_LEDS] += CHSV( gHue + random8(64), 0, 160 + random8(90));
    }

    ro=ro+64;
  };
  render();
}




void pulse() {
  for (int i=0;i<NUM_LEDS/4;i++) {
    leds[i] = CHSV( gHue ,255, 255);
    leds[i+NUM_LEDS/4] = CHSV( gHue ,255, 255);
    leds[i+2*NUM_LEDS/4] = CHSV( gHue ,255, 255);
    leds[i+3*NUM_LEDS/4] = CHSV( gHue ,255, 255);

    render();
    fadeToBlackBy(leds, NUM_LEDS, 10);
    delay(30);
  }
}


void snakePattern() {
  u_int8_t s = 255;
  u_int8_t v = 255;
  u_int8_t f=5;
  
    
 for (u_int16_t i = 0; i < LEDS_PER_ROW; i++) {
   lightOnePixelAndFadeFrame(i, 0, gHue, s, v,f);
   gHue++;
 }

 for (u_int16_t i = 1; i < ROW_NUM; i++) {
   lightOnePixelAndFadeFrame(LEDS_PER_ROW - 1, i, gHue, s, v,f);
  gHue++;
 }

 for (u_int16_t i = LEDS_PER_ROW - 2; i > 0; i--) {
   lightOnePixelAndFadeFrame(i, ROW_NUM - 1, gHue, s, v,f);
 gHue--;
 }

 for (u_int16_t i = ROW_NUM - 1; i > 0; i--) {
   lightOnePixelAndFadeFrame(0, i, gHue, s, v,f);
   gHue--;
 }
}

void snakePatternReverse() {
  u_int8_t s = 255;
  u_int8_t v = 255;
  u_int8_t f=5;
  
    
 for (u_int16_t i = LEDS_PER_ROW - 1; i > 0 ; i--) {
   lightOnePixelAndFadeFrame(i, 0, gHue, s, v,f);
   gHue++;
 }

 for (u_int16_t i = 0; i < ROW_NUM; i++) {
   lightOnePixelAndFadeFrame(0, i, gHue, s, v,f);
   gHue--;
 }

 for (u_int16_t i = 1; i < LEDS_PER_ROW - 2; i++) {
   lightOnePixelAndFadeFrame(i, ROW_NUM - 1, gHue, s, v,f);
   gHue--;
 }

 for (u_int16_t i = ROW_NUM - 1; i > 0 ; i--) {
   lightOnePixelAndFadeFrame(LEDS_PER_ROW - 1, i, gHue, s, v,f);
   gHue++;
 }


}

void lightOnePixelAndFadeFrame(u_int16_t x, u_int16_t y, u_int8_t h, u_int8_t s, u_int8_t v,uint8_t f) {
    leds[posFor(x, y)] = CHSV(h, s, v); 
    fadeFrame(f);
    delay(30);
    render();
}

void fadeFrame(u_int8_t f) {
  for (u_int16_t i = 0; i < LEDS_PER_ROW; i++) {
    fadePixel(i, 0,f);
    fadePixel(i, ROW_NUM - 1,f);
  }

  for (u_int16_t j = 1; j < ROW_NUM - 1; j++) {
    fadePixel(0, j,f);
    fadePixel(LEDS_PER_ROW -1 , j,f);
  }
}

void fadePixel(u_int16_t x, u_int16_t y,u_int8_t f) {
    if (leds[posFor(x, y)].r > f) {
        leds[posFor(x, y)].r -= f;
    } else {
        leds[posFor(x, y)].r = 0;
    }
    if (leds[posFor(x, y)].g > f) {
        leds[posFor(x, y)].g -= f;
    } else {
        leds[posFor(x, y)].g = 0;
    }
    if (leds[posFor(x, y)].b > f) {
        leds[posFor(x, y)].b -= f;
    } else {
        leds[posFor(x, y)].b = 0;
    }
}

void horizontalSnake() {
    u_int16_t fadeBy = 5;

    u_int8_t h = random8();
    u_int8_t s = 255;
    u_int8_t v = 255;

    u_int16_t offset0 = random16(0, LEDS_PER_ROW);
    u_int16_t offset1 = random16(0, LEDS_PER_ROW);
    u_int16_t offset2 = random16(0, LEDS_PER_ROW);
    u_int16_t offset3 = random16(0, LEDS_PER_ROW);
    u_int16_t offset4 = random16(0, LEDS_PER_ROW);
    u_int16_t offset5 = random16(0, LEDS_PER_ROW);


    for (u_int16_t x = 0; x < 5 * LEDS_PER_ROW; x++) {
        leds[posFor((x + offset0) % LEDS_PER_ROW, 0)] = CHSV(h, s, v);
        leds[posFor((x + offset1) % LEDS_PER_ROW, 1)] = CHSV(h, s, v);
        leds[posFor((x + offset2) % LEDS_PER_ROW, 2)] = CHSV(h, s, v);
        leds[posFor((x + offset3) % LEDS_PER_ROW, 3)] = CHSV(h, s, v);
        leds[posFor((x + offset4) % LEDS_PER_ROW, 4)] = CHSV(h, s, v);
        leds[posFor((x + offset5) % LEDS_PER_ROW, 5)] = CHSV(h, s, v);
        fadeToBlackBy(leds, NUM_LEDS, fadeBy);

        render();   
        delay(30);
    }
}

void horizontalDrunkSnake() {
    u_int16_t fadeBy = 5;

    u_int8_t h = random(0,255);
    u_int8_t s = 255;
    u_int8_t v = 255;

    for (u_int16_t x = 0; x < 5 * LEDS_PER_ROW; x++) {
        h = random(0,255);
        leds[posFor(x % LEDS_PER_ROW, random16(0, ROW_NUM))] = CHSV(h, s, v);
        fadeToBlackBy(leds, NUM_LEDS, fadeBy);

        render();
        delay(30);
    }
}

void horizontalDrunkSnakeReverse() {
    u_int16_t fadeBy = 5;

    u_int8_t h = random(0,255);
    u_int8_t s = 255;
    u_int8_t v = 255;

    for (u_int16_t x = 5 * LEDS_PER_ROW; x > 0; x--) {
        h = random(0,255);
        leds[posFor(x % LEDS_PER_ROW, random16(0, ROW_NUM))] = CHSV(h, s, v);
        fadeToBlackBy(leds, NUM_LEDS, fadeBy);

        render();
        delay(30);
    }
}

void fadeRow(u_int16_t y, u_int8_t fadeBy) {
    for (u_int16_t i = 0; i < LEDS_PER_ROW; i++) {
        fadePixel(i, y, fadeBy);
    }
}

void frame_and_hori_snake()
{
    u_int16_t fadeBy = 5;
  
    u_int8_t h = random(0,255);
    u_int8_t s = 255;
    u_int8_t v = 255;

    u_int16_t offset0 = random16(0, LEDS_PER_ROW);
    u_int16_t offset1 = random16(0, LEDS_PER_ROW);
    u_int16_t offset2 = random16(0, LEDS_PER_ROW);
    u_int16_t offset3 = random16(0, LEDS_PER_ROW);
    u_int16_t offset4 = random16(0, LEDS_PER_ROW);
    u_int16_t offset5 = random16(0, LEDS_PER_ROW);


    for (u_int16_t x = 0; x < 5 * LEDS_PER_ROW; x++) {
        leds[posFor((x + offset0) % LEDS_PER_ROW, 0)] = CHSV(h, s, v);
        leds[posFor((x + offset1) % LEDS_PER_ROW, 1)] = CHSV(h, s, v);
        leds[posFor((x + offset2) % LEDS_PER_ROW, 2)] = CHSV(h, s, v);
        leds[posFor((x + offset3) % LEDS_PER_ROW, 3)] = CHSV(h, s, v);
        leds[posFor((x + offset4) % LEDS_PER_ROW, 4)] = CHSV(h, s, v);
        leds[posFor((x + offset5) % LEDS_PER_ROW, 5)] = CHSV(h, s, v);
        fadeToBlackBy(leds, NUM_LEDS, fadeBy);
        h = beatsin16(18,0,250);
        fill_frame(h+100,s,beatsin16(10,20,150));

        render();
        delay(30);
    }
}
void fill_frame(u_int8_t h, u_int8_t s, u_int8_t v)
{
    for (u_int16_t i = 0; i < LEDS_PER_ROW; i++) {
    leds[posFor(i,0)]=CHSV(h,s,v);
    leds[posFor(i,ROW_NUM-1)]=CHSV(h,s,v);
  }

  for (u_int16_t j = 1; j < ROW_NUM - 1; j++) {
        leds[posFor(0,j)]=CHSV(h,s,v);
        leds[posFor(LEDS_PER_ROW-1,j)]=CHSV(h,s,v);

  }
}

void randomLocation(u_int16_t* location) {
  location[0] = random16(0, LEDS_PER_ROW);
  location[1] = random16(0, ROW_NUM);
}

void randomDir(int16_t* direction) {
  direction[0] = random8(6) - 3;
  direction[1] = random8(6) - 3;
}

void wormPattern() {

  u_int16_t fadeBy = 5;

  u_int8_t hsv[] = { gHue, 255l, 255l};

  u_int16_t* l1 = new u_int16_t(2);
  int16_t*   d1 = new int16_t(2);
  randomLocation(l1);
  randomDir(d1);

  u_int16_t* l2 = new u_int16_t(2);
  int16_t*   d2 = new int16_t(2);
  randomLocation(l2);
  randomDir(d2);

  u_int16_t* l3 = new u_int16_t(2);
  int16_t*   d3 = new int16_t(2);
  randomLocation(l3);
  randomDir(d3);

  u_int16_t* l4 = new u_int16_t(2);
  int16_t*   d4 = new int16_t(2);
  randomLocation(l4);
  randomDir(d4);

  u_int16_t* l5 = new u_int16_t(2);
  int16_t*   d5 = new int16_t(2);
  randomLocation(l5);
  randomDir(d5);
  for (u_int16_t j = 0; j < ROW_NUM + 5; j++) {
    l1 = wormMove(l1, d1, hsv);
    l2 = wormMove(l2, d2, hsv);
    l3 = wormMove(l3, d3, hsv);
    l4 = wormMove(l4, d4, hsv);
    l5 = wormMove(l5, d5, hsv);
    fadeToBlackBy(leds, NUM_LEDS, fadeBy);
    render();
    delay(30);    
  }

  delete[] l1;
  delete[] l2;
  delete[] l3;
  delete[] l4;
  delete[] l5;

  delete[] d1;
  delete[] d2;
  delete[] d3;
  delete[] d4;
  delete[] d5;

}

u_int16_t* wormMove(u_int16_t* location, int16_t* direction, u_int8_t* hsv) {

  int newX = direction[0] + location[0];
  int newY = direction[1] + location[1];
  if (newY >= 0 && newY < ROW_NUM && newX >= 0 && newX < LEDS_PER_ROW) {
    leds[posFor(newX, newY)] = CHSV(hsv[0], hsv[1], hsv[2]);
    location[0] = newX;
    location[1] = newY;
  }
  return location;
}

void render()
{
for(int i = 0; i < NUM_LEDS; i++) {
        CRGB fastLedRGB = leds[i];
        int octowsColor = ledz.color(fastLedRGB.r, fastLedRGB.g, fastLedRGB.b);
        ledz.setPixel( i, octowsColor);
      }
  
  // send the 'leds' array out to the actual LED strip
  ledz.show();

}