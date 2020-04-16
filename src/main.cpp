#include <Arduino.h>
#include <MQTT.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#define NUM_LEDS 30
#define DATA_PIN 0 

#include "passwd.h"

#include "stripwrapper.h"

#define DEBUG

MQTTClient homeassistant(300);
WiFiClient netRef;
Adafruit_NeoPixel strip(NUM_LEDS, DATA_PIN, NEO_GRBW + NEO_KHZ800);
StripWrapper tischleds(&strip);

void connectWifi(){
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\n");
}

void initHomeassistant(){
  homeassistant.begin(MQTT_ADDRESS, netRef);
}

void connectHomeassistant(){
  Serial.print("connect mqtt...");
  while (!homeassistant.connect(DEVICENAME, MQTT_USERNAME, MQTT_PASSWORD)){
    Serial.print(".");
    delay(1000);
  }
  homeassistant.publish("test", "test");
  Serial.print("\n");
}

void registerLight(){
  DynamicJsonDocument doc(256);
  doc["~"] = "homeassistant/light/table";
  doc["name"] = "Tisch";
  doc["unique_id"] = "table_light";
  doc["cmd_t"] = "~/set";
  doc["stat_t"] = "~/state";
  doc["schema"] = "json";
  doc["brightness"] = true;
  doc["rgb"] = true;
  doc["white_value"] = true;
  doc["optimistic"] = true;
  doc["effect"] = true;
  JsonArray effectList = doc.createNestedArray("effect_list");
  effectList.add("rainbow");
  effectList.add("rainbowmove");
  effectList.add("raining");

  const uint outputSize = 256;
  char output[outputSize];
  if (serializeJson(doc, output) > outputSize){
    Serial.println("ARRAY OUT OF BOUNDS");
    return; 
  }

#ifdef DEBUG
  Serial.print("doc Memoryusage: ");
  Serial.print(doc.memoryUsage());
  Serial.println();
  Serial.print("mqtt memory: ");
  Serial.println(strlen(output));
  Serial.println(output);
#endif

  bool retVal = homeassistant.publish("homeassistant/light/table/config", output);
#ifdef DEBUG
  Serial.print("publish: ");
  Serial.println(retVal);
#endif
}

void mqttCallback(MQTTClient *client, char topic[], char payload[], int payload_length){
  Serial.println("callback");
  DynamicJsonDocument doc(128);
  DeserializationError e = deserializeJson(doc, payload, payload_length);
  if (e != DeserializationError::Ok){
    Serial.print(e.c_str());
  }
  if (strcmp(doc["state"], "ON") == 0){
    if (doc.containsKey("color")){
      Serial.println("color:");
      Serial.print("r: ");
      byte red = doc["color"]["r"];
      byte green = doc["color"]["g"];
      byte blue = doc["color"]["b"];
      Serial.println(red);
    }
    if (doc.containsKey("white_value")){
      
    }
    if (doc.containsKey("brightness")){

    }
    if (doc.containsKey("effect")){

    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  Serial.println();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  homeassistant.begin(MQTT_ADDRESS, MQTT_PORT, netRef);
  connectWifi();
  connectHomeassistant();
  registerLight();
  homeassistant.onMessageAdvanced(mqttCallback);
  homeassistant.subscribe("homeassistant/light/table/set");
  strip.begin();
  tischleds.setAnimation(tischleds.RAINBOWMOVING);
  strip.setPixelColor(2, 255, 0, 0);
  strip.show();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!homeassistant.connected()){
    connectWifi();
    connectHomeassistant();
  }
  delay(100);
  tischleds.animate();
  homeassistant.loop();
}