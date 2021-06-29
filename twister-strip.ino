// TODO add specular highlight and shading
// TODO morph parameter space, e.g. changing the number of sides etc.

#include<FastLED.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

// esp32 pins: onboard LED 5
// clean pin with no PWM on boot: 4

#define DATA_PIN 4
#define STATUS_LED_PIN 2
#define NUM_LEDS 120

// if you're too fast, the pixels don't update?
// I have heard FastLED can update at 60FPS
#define MAX_FPS 2

// palette from pico8
const uint32_t colours[] = {
  0xFF004D,
  0xFFA300,
  0xFFEC27,
  0x29ADFF,
  0x83769C,
  0x00E436,
  0xFF77A8,
  0xFFCCAA,
  0x1D2B53,
  0x7E2553,
  0x008751,
  0xAB5236,
  0x5F574F,
  0xC2C3C7,
  0x000000,
  0xffffff
};

// globals

CRGBArray<NUM_LEDS> strip;
int sides = 4;    // twister sides
int maxLength = (int) round(sin(PI/sides) * NUM_LEDS);
int prevx = 0;

float theta = 0.0;
float amplitude = (float) NUM_LEDS;

long microsPerFrame = (int) (1000000.0/MAX_FPS);
long endLastFrame;

void setup() {
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif
  // Serial.begin(115200);
  pinMode(DATA_PIN, OUTPUT);

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
  
  for (int side = 0; side < sides; side++) {
    float t = side * TWO_PI / sides;
    int x = (int) round(((sin(t + theta) + 1) / 2) * amplitude);
    drawLine(prevx, x, side);
    // Serial.print(side * 20);
    // Serial.print(",");
    // Serial.print(prevx);
    // Serial.print(",");
    // Serial.print(x);
    // Serial.println();
    prevx = x;
  }
  
  
  theta += 0.01;
  //theta += sin(theta/5)*0.08;
  
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
    int darken = (int) map(length, 0, maxLength, 128, 255);
    // Serial.print(maxLength);
    // Serial.print(",");
    // Serial.println(darken);
    CRGB colour = CRGB(colours[colourIndex]) %= darken;
    
    for (int i=x1; i<=x2; i++) {
      strip[i] = colour;
    }
  }
}

void clearNow() {
  FastLED.clear();
  FastLED.show();
}
