// TODO add specular highlight and shading
// TODO morph parameter space, e.g. changing the number of sides etc.

#include<FastLED.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

// esp32 pins: onboard LED 5
// clean pin with no PWM on boot: 4

#define DATA_PIN 4
#define NUM_LEDS 60

// scaling factor 0-255 for brightness. 
// with max draw current of 2.2 and 120 LEDs (60 paired) 
// we don't want to draw more than (2.2/(0.05 * 120))*255 = 93
#define MAX_BRIGHTNESS 83

// palette from pico8
const uint32_t colours[] = {
//  0xFF004D,
//  0xFFA300,
//  0xFFEC27,
//  0x29ADFF,
//  0x83769C,
//  0x00E436,
//  0xFF77A8,
//  0xFFCCAA,
//  0x1D2B53,
//  0x7E2553,
//  0x008751,
//  0xAB5236,
//  0x5F574F,
//  0xC2C3C7,

  // colours generated from here: http://medialab.github.io/iwanthue/
  0xeb4eff,
  0xd3ff1e,
  0x7d0062,
  0x01a82a,
  0x0155a4,
  0xffc848,
  0x16002b,
  0xc3ffb2,
  0xba1b00,
  0x02bdb1,
  0xff867d,
  0x6c4800,
  0xffbaf3,
  0xffe1a7,
  0x000000,
  0xffffff
};

CRGBArray<NUM_LEDS> strip;
int sides = 10;    // twister sides
int maxLength = (int) round(sin(PI/sides) * NUM_LEDS);
int prevx = 0;

// if you're too fast, the pixels don't update?
// I have heard FastLED can update at 60FPS
int MAX_FPS = 120;

float ROT_SPEED = 0.03;

double theta = 0.0;
double amplitude = (double) NUM_LEDS;

long microsPerFrame = (int) (1000000.0/MAX_FPS);
long endLastFrame;

void setup() {

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif
  pinMode(DATA_PIN, OUTPUT);

  FastLED.setBrightness(MAX_BRIGHTNESS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(strip, NUM_LEDS);
  clearNow();
  endLastFrame = micros();
}

void loop() {

  // for each side, if it's in the front (positive angular difference),
  // render a line from lowest to highest index
  
  for (int i=0; i<NUM_LEDS; i++) {
    strip[i] = CRGB(0, 0, 0);
  }
  
  double theta2 = theta + ((sin(theta*6)) * 1.4);
  //amplitude = ((float) NUM_LEDS) - ((millis() / 50) % 20);
  //int offsetx = NUM_LEDS - ((int) amplitude / 2);
  for (int side = 0; side < sides; side++) {
    float t = side * TWO_PI / sides;
    int x = (int) round(((sin(t + theta2) + 1) / 2) * amplitude);
    drawLine(prevx + 1, x - 1, side);
    //drawLine(prevx, prevx, 15);
    
    prevx = x;
  }
  
  theta += ROT_SPEED;
  
  FastLED.show();
  FastLED.delay(0); 

  // wait until time for next frame if we're finished early
  long microWait = microsPerFrame - (micros() - endLastFrame);
  if (microWait > 0) {
    delayMicroseconds(microWait);
  }

  endLastFrame = micros();
}

/*
 * Draws a line in the given colour from x1 to x2 iff x1 < x2.
 */
void drawLine(int x1, int x2, int colourIndex) {
  
  int length = x2 - x1;
  if (length > 0) {
    // TODO make darken value from angle of incidence from lightsource (front on)
    // using Lambert cosine law, maybe move out of this function and just take the color in
    //int darken = (int) map(length, 0, maxLength+1, 128, 254);

    CRGB colour = CRGB(colours[colourIndex]);// %= darken;
    
    for (int i=x1; i<=x2; i++) {
      strip[i] = colour;
    }
  }
}

void clearNow() {
  FastLED.clear();
  FastLED.show();
}
