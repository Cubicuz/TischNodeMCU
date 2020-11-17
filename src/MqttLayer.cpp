#include "MqttLayer.h"
#include "passwd.h"
#include "Debug.h"

MqttLayer::MqttLayer():
  mqttclient(MQTT_BUFFER_SIZE){
}

int8_t MqttLayer::initMQTT(Client &c) {
    mqttclient.begin(MQTT_ADDRESS, MQTT_PORT, c);
}

int8_t MqttLayer::connectMQTT(MQTTClientCallbackAdvanced cb) {
  debugPrintf("connect mqtt...");
  while (!mqttclient.connect(DEVICENAME, MQTT_USERNAME, MQTT_PASSWORD)) {
    debugPrintf(".");
    delay(1000);
  }
  mqttclient.onMessageAdvanced(cb);
  mqttclient.subscribe("discovery/light/table/set");
  mqttclient.publish("test", "test");
  debugPrintf("\n");
  return EXIT_SUCCESS;
}

int8_t MqttLayer::loop() {
    mqttclient.loop();
}

bool MqttLayer::connected() {
  return mqttclient.connected();
}