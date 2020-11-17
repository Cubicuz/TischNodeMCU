#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "HomeAssistant.h"
#define NUM_LEDS 30
#define DATA_PIN D3
#define MOSFET_PIN D1

#include "StripWrapper.h"

#define DEBUG

Adafruit_NeoPixel strip(NUM_LEDS, DATA_PIN, NEO_GRBW + NEO_KHZ800);
StripWrapper tischleds(&strip);
Homeassistant hs(&tischleds);

bool stripOn;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(19200);
  Serial.println();
  pinMode(MOSFET_PIN, OUTPUT);
  strip.begin();
  hs.begin();
  hs.connect();
  hs.setStatusReceivedCallback([&](const Status &s){
    if (s.on){
      if (!stripOn){
        stripOn = true;
        digitalWrite(MOSFET_PIN, HIGH);
      }
      tischleds.setAnimation(s.animation);
      tischleds.setBrightness(s.brightness);
      tischleds.setColor(s.color.channels.r, s.color.channels.g, s.color.channels.b, s.color.channels.w);
    } else {
      if (stripOn){
        stripOn = false;
        digitalWrite(MOSFET_PIN, LOW);
      }
    }
  });
}

void loop()
{
    hs.loop();
}