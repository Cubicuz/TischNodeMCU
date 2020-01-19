#include <EthernetServer.h>
#include <Dns.h>
#include <EthernetUdp.h>
#include <EthernetClient.h>
#include <Dhcp.h>
#include <Ethernet.h>

#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServerSecureBearSSL.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <WiFiServerSecure.h>
#include <WiFiServer.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureAxTLS.h>
#include <ESP8266WiFiAP.h>
#include <WiFiUdp.h>

#include <ArduinoJson.h>

#include <PubSubClient.h>

#include <Adafruit_NeoPixel.h>

/************ WIFI and MQTT Information (CHANGE THESE FOR YOUR SETUP) ******************/
#include "passwd.h"

/*********************************** NeoPixel Defintions ********************************/
#define LedDataPin 5
#define NUMPIXELX 30
//228 
//6
#define NUMPIXELY 15
//96
//9
#define NUMPIXEL (NUMPIXELX+NUMPIXELX+NUMPIXELY+NUMPIXELY)
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXEL, LedDataPin, NEO_GRBW + NEO_KHZ800);

/************* MQTT TOPICS (change these topics as you wish)  **************************/
const char* light_state_topic = "bruh/porch";
const char* light_set_topic = "bruh/porch/set";

const char* on_cmd = "ON";
const char* off_cmd = "OFF";
const char* effect = "solid";
String effectString = "longlonglongname";
bool changed = false;
/**************************************** global vars  *********************************/
WiFiClient espClient;
PubSubClient client(espClient);

bool stateOn = false;

byte realRed = 0;
byte realGreen = 0;
byte realBlue = 0;
byte realWhite = 0;

byte red = 0;
byte green = 0;
byte blue = 0;
byte white = 0;
byte brightness = 255;

/****************************************FOR JSON***************************************/
const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);
#define MQTT_MAX_PACKET_SIZE 512



/********************************** START SETUP WIFI*****************************************/
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/********************************** START CALLBACK*****************************************/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (!processJson(message)) {
    return;
  }

  if (stateOn) {

    realRed = map(red, 0, 255, 0, brightness);
    realGreen = map(green, 0, 255, 0, brightness);
    realBlue = map(blue, 0, 255, 0, brightness);
    realWhite = white; /* dont scale warmwhite */
  }
  else {

    realRed = 0;
    realGreen = 0;
    realBlue = 0;
    realWhite = 0;
  }

  Serial.println(effect);

  sendState();
}

/********************************** START PROCESS JSON*****************************************/
bool processJson(char* message) {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(message);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return false;
  }

  // State On Off
  if (root.containsKey("state")) {
    if (strcmp(root["state"], on_cmd) == 0) {
      stateOn = true;
    }
    else if (strcmp(root["state"], off_cmd) == 0) {
      stateOn = false;
    }
  }

  if (root.containsKey("color")) {
    red = root["color"]["r"];
    green = root["color"]["g"];
    blue = root["color"]["b"];
  }

  if (root.containsKey("white_value")) {
    white = root["white_value"];
  }

  if (root.containsKey("brightness")) {
    brightness = root["brightness"];
  }

  if (root.containsKey("effect")) {
    effect = root["effect"];
    effectString = effect;
  }
  changed = true;
  return true;
}

/********************************** START SEND STATE*****************************************/
void sendState() {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();

  root["state"] = (stateOn) ? on_cmd : off_cmd;
  JsonObject& color = root.createNestedObject("color");
  color["r"] = red;
  color["g"] = green;
  color["b"] = blue;

  root["brightness"] = brightness;
  root["effect"] = effectString.c_str();
  root["white_value"] = white;


  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

  client.publish(light_state_topic, buffer, true);
}

/********************************** START RECONNECT*****************************************/
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(SENSORNAME, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(light_set_topic);
      setColor(0, 0, 0, 0);
      sendState();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/********************************** START Set Color*****************************************/
void setColor(int inR, int inG, int inB, int inW) {

  
  for(uint16_t i=0; i<NUMPIXEL; i++) {
    strip.setPixelColor(i, strip.Color(inR, inG, inB, inW));
  }
  strip.show();

/*  Serial.println("Setting LEDs:");
  Serial.print("r: ");
  Serial.print(inR);
  Serial.print(", g: ");
  Serial.print(inG);
  Serial.print(", b: ");
  Serial.println(inB);*/
}
/********************************** START Setup *****************************************/

void setup() {
  // LED
  strip.begin();
  strip.setBrightness(25);
  strip.show(); // Initialize all pixels to 'off'

  Serial.begin(115200);

//#define testing
#ifdef testing
// put testcode here
  stateOn = true;
  effectString = "rain";
  red = 255;
#else
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println("Ready");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  randomSeed(analogRead(0));
#endif
}
/********************************** START MAIN LOOP*****************************************/
void loop() {
#ifndef testing
  if (!client.connected()) {
    reconnect();
  }

  if (WiFi.status() != WL_CONNECTED) {
    delay(1);
    Serial.print("WIFI Disconnected. Attempting reconnection.");
    setup_wifi();
    return;
  }

  if (stateOn){
    movingEffects();
    staticEffects();
  } else {
    setColor(0, 0, 0, 0);
  }


  client.loop();
#else

  
#endif
}

void movingEffects() {
  
  if (effectString == "rainbowtravel") {
    movingRainbow();
  }

  if (effectString == "rainbowtravelx4") {
    movingRainbowX4();
  }

  if (effectString == "fire") {
    
  }

  if (effectString == "colorfading") {
    
  }

  if (effectString == "nightrider") {
    chase();
  }

  if (effectString == "rain") {
    raining();
  }

  if (effectString == "raincolor") {
    rainingColor();
  }
  delay(10);
}

#define TRAVELDELAY 100
uint16_t travelposition = 0;
uint16_t travelDelayCtr = TRAVELDELAY;
void movingRainbow(){
  if (travelDelayCtr-- < 1){
    travelDelayCtr = TRAVELDELAY;
    travelposition = (1 + travelposition) % 256;
    staticRainbow(travelposition);
  }
}

void movingRainbowX4(){
  if (travelDelayCtr-- < 1){
    travelDelayCtr = TRAVELDELAY;
    travelposition = (1 + travelposition) % 256;
    staticRainbowX4(travelposition);
  }
}

#define chaseWidthHalf 2
#define chaseDelay 2
byte chaseCounter = 0;
void chase(){
  if (chaseCounter >= chaseDelay){
    chaseCounter = 0;
    travelposition = (1 + travelposition) % NUMPIXEL;
    uint32_t color = strip.Color(red, green, blue, white);
    for (uint16_t i=0;i<NUMPIXEL;i++){
      if ( (abs(i - travelposition) < chaseWidthHalf) or (abs(i - travelposition) > NUMPIXEL - chaseWidthHalf)){
        strip.setPixelColor(i, color);
      } else {
        strip.setPixelColor(i, 0);
      }
    }

    strip.show();
    
  } else {
    chaseCounter += 1;
  }
}

#define DROPWIDTH 2
uint8_t drops[NUMPIXEL];
uint8_t dropdelay = 10;
void raining(){
  // water evaporating
  for (uint16_t i = 0;i<NUMPIXEL;i++){
    if (drops[i] < 2){
      drops[i] = 0;
    } else {
      drops[i] -= 2;
    }    
  }

  // add new drop
  dropdelay--;
  if (dropdelay < 1){
    dropdelay = 5 + random(10);
    
    uint16_t newDropPosition = random(NUMPIXEL);
    uint8_t intensivity = random(128) + 127; // minimum half intensity
  
    drops[newDropPosition] = intensivity;
    for (uint16_t i = 0;i < DROPWIDTH;i++){
      intensivity /=2;
      drops[circleAddress(newDropPosition + i)] = intensivity;
      drops[circleAddress(newDropPosition - i)] = intensivity;
    }
  }
  for (uint16_t i = 0;i<NUMPIXEL;i++){
    strip.setPixelColor(i, strip.Color(0, 0, drops[i], white));
  }
  strip.show();
}

#define REDUCTION 2
uint8_t dropRed[NUMPIXEL];
uint8_t dropGreen[NUMPIXEL];
uint8_t dropBlue[NUMPIXEL];
void rainingColor(){
  // evaporaing
  reduceIntensity(dropRed, REDUCTION);
  reduceIntensity(dropGreen, REDUCTION);
  reduceIntensity(dropBlue, REDUCTION);

  // add new drop
  dropdelay--;
  if (dropdelay < 1){
    dropdelay = 5 + random(10);
    
    uint16_t newDropPosition = random(NUMPIXEL);
    uint8_t colorPosition = random(256);
    uint32_t color = Wheel(colorPosition);
    setNewDrop(dropRed, color >> 16, newDropPosition);
    setNewDrop(dropGreen, (color >> 8) & 255, newDropPosition);
    setNewDrop(dropBlue, (color >> 0) & 255, newDropPosition);

  }
  // apply color
  for (uint16_t i = 0;i<NUMPIXEL;i++){
    strip.setPixelColor(i, strip.Color(dropRed[i], dropGreen[i], dropBlue[i], 0));
  }
  strip.show();
}
inline void reduceIntensity(uint8_t leds [], uint8_t reduction){
  for (uint16_t i=0;i<NUMPIXEL;i++){
    if (leds[i] < reduction){
      leds[i] = 0;
    } else {
      leds[i] -= reduction;
    }
  }
}
inline void setNewDrop(uint8_t* leds, uint8_t intensivity, uint16_t index){
  leds[index] = intensivity;
  for (uint16_t i = 0; i< DROPWIDTH; i++){
    intensivity /= 2;
    leds[circleAddress(index + i)] = intensivity;
    leds[circleAddress(index - i)] = intensivity;
  }
}




void staticEffects() {

  if (!changed){
    return;
  } else {
    changed = false;
  }
  
  if (effectString == "solid"){
      setColor(realRed, realGreen, realBlue, realWhite);

  }
  
  if (effectString == "rainbow") {
    staticRainbow();
  }

  if (effectString == "rainbowx4") {
    staticRainbowX4();
  }

  if (effectString == "warm white") {
    setColor(0, 0, 0, realWhite);
  }
}

void staticRainbow(){
  staticRainbow(0);
}
void staticRainbow(uint8_t offset) {
  for (uint8_t i=0;i<NUMPIXEL; i++){
    strip.setPixelColor(i, Wheel(((i+offset)*255)/NUMPIXEL) | white); // i muss noch auf nen bereich von Pixelanzahl/255 gemappt werden
    
  }
  strip.show();

}

void staticRainbowX4(){
  staticRainbowX4(0);
}
void staticRainbowX4(uint8_t offset) {
  for (uint8_t i=0;i < NUMPIXELX; i++){
    uint32_t color = Wheel(((i+offset)*255)/NUMPIXELX) | white;
    strip.setPixelColor(i, color);
    strip.setPixelColor(NUMPIXELX + NUMPIXELY + i, color);
  }
  for (uint8_t i=0;i < NUMPIXELY; i++){
    uint32_t color = Wheel(((i+offset)*255)/NUMPIXELY) | white;
    strip.setPixelColor(NUMPIXELX + i, color);
    strip.setPixelColor(NUMPIXELX + NUMPIXELY + NUMPIXELX + i, color);
  }
  strip.show();
}

void showleds() {
  if (stateOn) {
    strip.show();
  }
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<NUMPIXEL; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<NUMPIXEL; i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

uint16_t circleAddress(int16_t pos){
  while (pos<0){
    pos += NUMPIXEL;
  }
  return  pos % NUMPIXEL;
}
