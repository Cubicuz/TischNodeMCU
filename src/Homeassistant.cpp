#include "Homeassistant.h"
#include <ArduinoJson.h>

Homeassistant::Homeassistant(StripWrapper *strip)
    : strip(strip), mqttclient(MQTT_BUFFER_SIZE) {
  if (instance){
    Serial.println("ERROR: two instances of Homeassistant !!!");
    return;
  }
  instance = this;
}

Homeassistant::~Homeassistant(){
  instance = NULL;
}

RETVAL Homeassistant::begin(){
  RETVAL ret = initWifi();
  if (ret != EXIT_SUCCESS){
    return ret;
  }
  return initMQTT();
}

void Homeassistant::loop(){
  mqttclient.loop();
}

RETVAL Homeassistant::connect() {
  RETVAL ret = 0;
  ret = connectWifi();
  if (ret != 0) {
    return ret;
  }
  ret = connectMQTT();
  if (ret != 0) {
    return ret;
  }
  ret = registerLight();
  if (ret != 0) {
    return ret;
  }
  return EXIT_SUCCESS;
}

RETVAL Homeassistant::reconnect() {
  // check wifi
  // check MQTT
  // check Homeassistant
  return EXIT_SUCCESS;
}

RETVAL Homeassistant::connected() {
  if (!WiFi.isConnected()) {
    return EXIT_HA_WIFI_NOT_CONNECTED | EXIT_HA_MQTT_NOT_CONNECTED;
  }
  if (!mqttclient.connected()) {
    return EXIT_HA_MQTT_NOT_CONNECTED;
  }
  return EXIT_SUCCESS;
}

bool Status::operator==(const Status &s) const {
  if (this->animation == s.animation) {
    if(this->on == s.on){
      return this->color.rgbw == s.color.rgbw;
    }
  }
  return false;
}

RETVAL Homeassistant::setStatusReceivedCallback(
    std::function<void(const Status &s)> fkt) {
  onStatusReceived = fkt;
  return EXIT_SUCCESS;
}

RETVAL Homeassistant::initWifi() {
  wl_status_t ret = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (ret != WL_CONNECTED) {
    debugPrintf("Error initWifi code: %i", ret);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

RETVAL Homeassistant::connectWifi() {
  debugPrintf("checking wifi...");
  uint16_t timeoutCtr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    timeoutCtr++;
    if (timeoutCtr > WIFI_TIMEOUT_IN_100MS) {
      debugPrintf(" timeout\n");
      return EXIT_TIMEOUT;
    }
    debugPrintf(".");
    delay(100);
  }
  debugPrintf("\n");
  return EXIT_SUCCESS;
}

RETVAL Homeassistant::initMQTT() {
  mqttclient.begin(MQTT_ADDRESS, MQTT_PORT, netRef);
  return EXIT_SUCCESS;
}

RETVAL Homeassistant::connectMQTT() {
  debugPrintf("connect mqtt...");
  while (!mqttclient.connect(DEVICENAME, MQTT_USERNAME, MQTT_PASSWORD)) {
    debugPrintf(".");
    delay(1000);
  }
  instance = this;
  mqttclient.onMessageAdvanced(Homeassistant::mqttCallback);
  mqttclient.subscribe("homeassistant/light/table/set");
  mqttclient.publish("test", "test");
  debugPrintf("\n");
  return EXIT_SUCCESS;
}

RETVAL Homeassistant::sendStatus(const Status &s) {
  if (status == s) {
    return EXIT_SUCCESS;
  }
  DynamicJsonDocument doc(MQTT_BUFFER_SIZE);
  if (s.on){
    doc["state"] = "on";
    auto color = doc.createNestedObject("color");
    color["r"] = s.color.channels.r;
    color["g"] = s.color.channels.g;
    color["b"] = s.color.channels.b;
    doc["white_value"] = s.color.channels.w;
    doc["brightness"] = s.brightness;
    doc["effect"] = s.animation;
  } else {
    doc["state"] = "off";
  }
  char output[MQTT_BUFFER_SIZE];
  if (serializeJson(doc, output) > MQTT_BUFFER_SIZE) {
    debugPrintLn("ARRAY OUT OF BOUNDS");
    return EXIT_FAILURE;
  }
  debugPrintf("doc Memoryusage: %u\n", doc.memoryUsage());
  debugPrintf("mqtt memory: %u\n", strlen(output));
  debugPrintLn(output);

  bool retVal = mqttclient.publish("homeassistant/light/table/state", output);

  debugPrintf("publish: ");
  debugPrintLn(retVal);
  return EXIT_SUCCESS;

}

void Homeassistant::mqttCallback(MQTTClient *client, char topic[],
                                 char payload[], int payload_length) {
  debugPrintLn("callback");
  Status s;
  s = instance->status;
  DynamicJsonDocument doc(128);
  DeserializationError e = deserializeJson(doc, payload, payload_length);
  if (e != DeserializationError::Ok) {
    Serial.print(e.c_str());
  }
  if (strcmp(doc["state"], "ON") == 0) {
    s.on = true;
    if (doc.containsKey("color")) {
      Serial.println("color:");
      Serial.print("r: ");
      uint8_t red = doc["color"]["r"];
      uint8_t green = doc["color"]["g"];
      uint8_t blue = doc["color"]["b"];
      Serial.println(red);
      s.color.channels.r = red;
      s.color.channels.g = green;
      s.color.channels.b = blue;
    }
    if (doc.containsKey("white_value")) {
      s.color.channels.w = doc["white_value"];
    }
    if (doc.containsKey("brightness")) {
      s.brightness = doc["brightness"];
    }
    if (doc.containsKey("effect")) {
      const char * effect = doc["effect"];
      effect = instance->strip->sameAnimationNameButMyPointer(effect);
      if (effect == nullptr){
        Serial.println("animation is invalid");
        return;
      }
      debugPrintLn(effect);
      s.animation = effect;
    }
  } else {
    s.on = false;
  }
  if (s == instance->status){
    return;
  }
  instance->status = s;
  instance->onStatusReceived(instance->status);
}

RETVAL Homeassistant::registerLight() {
  // reset existing configuration
  // homeassistant.publish("homeassistant/light/table/config", "");
  // homeassistant.loop();

  DynamicJsonDocument doc(MQTT_BUFFER_SIZE);
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
  for (uint8_t i = 0;
       i < strip->availableAnimationCount / strip->availableAnimationCount;
       i++) {
    effectList.add(strip->availableAnimations[i]);
  }

  char output[MQTT_BUFFER_SIZE];
  if (serializeJson(doc, output) > MQTT_BUFFER_SIZE) {
    debugPrintLn("ARRAY OUT OF BOUNDS");
    return EXIT_FAILURE;
  }
  debugPrintf("doc Memoryusage: %u\n", doc.memoryUsage());
  debugPrintf("mqtt memory: %u\n", strlen(output));
  debugPrintLn(output);

  bool retVal = mqttclient.publish("homeassistant/light/table/config", output);

  debugPrintf("publish: ");
  debugPrintLn(retVal);
  return EXIT_SUCCESS;
}
