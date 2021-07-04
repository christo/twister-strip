// TODO add specular highlight and shading
// TODO morph parameter space, e.g. changing the number of sides etc.

#include<FastLED.h>

#include <DNSServer.h>
#include <ESPUI.h>

#include <WiFi.h>
// secrets:
#include "wifi-password.h"

const char *hostname = "twister";
const char *ssid = "twister";
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
int currentEffectLabel;

// if you're too fast, the pixels don't update?
// I have heard FastLED can update at 60FPS
// undefine for no delay
//#define MAX_FPS 120

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
int sides = 6;    // twister sides
int maxLength = (int) round(sin(PI/sides) * NUM_LEDS);
int prevx = 0;


float rotSpeed = 0.02;

double theta = 0.0;
double amplitude = (double) NUM_LEDS;

#if defined(MAX_FPS)
long microsPerFrame = (int) (1000000.0/MAX_FPS);
#endif

long endLastFrame;
enum Effect { TWISTER, BUBBLES, STARS, WAVES };

Effect effect = WAVES;

void setup() {
  pinMode(DATA_PIN, OUTPUT);
  
  Serial.begin(115200);
  delay(200);
  setupWeb();

  FastLED.setBrightness(MAX_BRIGHTNESS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(strip, NUM_LEDS);
  clearNow();
  endLastFrame = micros();
}

void setupWeb() {
  //ESPUI.setVerbosity(Verbosity::VerboseJSON);
  WiFi.setHostname(hostname);
  WiFi.begin(SSID, WIFIPASS);
  uint8_t timeout = 10;

  // Wait for connection until timeout
  do {
    delay(500);
    timeout--;
  } while (timeout && WiFi.status() != WL_CONNECTED);

  // not connected -> create hotspot
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("\n\nCreating hotspot");

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(ssid);

    timeout = 10;

    do {
      delay(500);
      Serial.print(".");
      timeout--;
    } while (timeout);
  }
  dnsServer.start( DNS_PORT, "*", apIP );
  Serial.println( "\n\nWiFi parameters:" );
  Serial.print( "Mode: " );
  Serial.println( WiFi.getMode() == WIFI_AP ? "Station" : "Client" );
  Serial.print( "IP address: " );
  Serial.println( WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP() );
  Serial.print("MAC: ");
  Serial.print(mac[5],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.println(mac[0],HEX);
  setupUi();
}

void setupUi() {

  currentEffectLabel = ESPUI.label("Effect:", ControlColor::Emerald, "loading...");
  
  ControlColor c = ControlColor::Turquoise;

  ESPUI.button("stars", &starsButton, c, "stars");
  ESPUI.button("twister", &twisterButton, c, "twister");
  ESPUI.button("waves", &wavesButton, c, "waves");
  ESPUI.button("bubbles", &bubblesButton, c, "bubbles");

  int sliderId = ESPUI.slider("speed", &speedSlider, ControlColor::Peterriver, 10);

  ESPUI.begin(" twister");
  ESPUI.sliderContinuous = true;
  ESPUI.updateControlValue(sliderId, String((int) rotSpeed * 100));
}

void speedSlider(Control *sender, int type) {
  rotSpeed = (float) constrain(sender->value.toInt(), 1, 10) / 100;
}

void starsButton(Control *sender, int type) {
  if (type == B_DOWN) {
    effect = STARS;
    ESPUI.print(currentEffectLabel, "stars");
  }
}

void wavesButton(Control *sender, int type) {
  if (type == B_DOWN) {
    effect = WAVES;
    ESPUI.print(currentEffectLabel, "waves");
  }
}

void bubblesButton(Control *sender, int type) {
  if (type == B_DOWN) {
    effect = BUBBLES;
    ESPUI.print(currentEffectLabel, "bubbles");
  }
}

void twisterButton(Control *sender, int type) {
  if (type == B_DOWN) {
    effect = TWISTER;
    ESPUI.print(currentEffectLabel, "twister");
  }
}


void loop() {

  // for each side, if it's in the front (positive angular difference),
  // render a line from lowest to highest index
  
  for (int i=0; i<NUM_LEDS; i++) {
    strip[i] = CRGB(0, 0, 0);
  }

  switch(effect) {
    case Effect::TWISTER : twister(); break;
    case Effect::BUBBLES : bubbles(); break;
    case Effect::STARS   : stars(); break;
    case Effect::WAVES   : waves(); break;
    default: waves(); break;
  }
  
  theta += rotSpeed;
  
  FastLED.show();
  FastLED.delay(0); 

  #if defined(MAX_FPS)
    // wait until time for next frame if we're finished early
    long microWait = microsPerFrame - (micros() - endLastFrame);
    if (microWait > 0) {
      delayMicroseconds(microWait);
    }
  
    endLastFrame = micros();
  #endif

}

void bubbles() {
  // draw random circles, keep buffer of in-progress circles
}

void waves() {
  for(int i=0; i<14; i++) {
    int x = (int) ((sin(theta * (i+1) / 3) + 1) * NUM_LEDS / 2);
    strip[x] = CRGB(colours[i]);
  }
}

void stars() {
  // random starfield
  int numStars = random(1, 3);
  
  for (int i=0; i < numStars; i++) {
    int x = random(0, NUM_LEDS);
    strip[x] = CRGB(255 - x, 255 - x, 255); // light blue radial gradient
  }
}

// classic demoscene effect
void twister() {
  double theta2 = theta + ((sin(theta*6)) * 1.4);
  //amplitude = ((float) NUM_LEDS) - ((millis() / 50) % 20);
  //int offsetx = NUM_LEDS - ((int) amplitude / 2);
  for (int side = 0; side < sides; side++) {
    float t = side * TWO_PI / sides;
    int x = (int) round(((sin(t + theta2) + 1) / 2) * amplitude);
    drawLine(prevx + 2, x - 2, side);
    //drawLine(prevx, prevx, 15);
    
    prevx = x;
  }
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
