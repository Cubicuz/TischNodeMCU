#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Homeassistant.h"
#define NUM_LEDS 30
#define DATA_PIN 0

#include "StripWrapper.h"

#define DEBUG

Adafruit_NeoPixel strip(NUM_LEDS, DATA_PIN, NEO_GRBW + NEO_KHZ800);
StripWrapper tischleds(&strip);
Homeassistant hs(&tischleds);


void setup()
{
  // put your setup code here, to run once:
  Serial.begin(19200);
  Serial.println();
  strip.begin();
  hs.begin();
  hs.connect();
}

void loop()
{
    tischleds.animate();
    hs.loop();
}