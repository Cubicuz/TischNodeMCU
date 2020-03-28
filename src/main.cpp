#include <Arduino.h>
#include <MQTT.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include "animations.h"
#include "passwd.h"

MQTTClient homeassistant(300);
WiFiClient netRef;

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
  
  Serial.print("doc Memoryusage: ");
  Serial.print(doc.memoryUsage());
  Serial.println();
  char output[256];
  Serial.print("mqtt memory: ");
  Serial.print(serializeJson(doc, output));
  Serial.print(" ");
  Serial.println(strlen(output));
  Serial.println(output);
  Serial.print("publish: ");
  Serial.println(homeassistant.publish("homeassistant/light/table/config", output));
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
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!homeassistant.connected()){
    connectWifi();
    connectHomeassistant();
  }
  homeassistant.loop();
}