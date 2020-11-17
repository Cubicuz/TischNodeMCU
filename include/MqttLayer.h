#ifndef MQTTLAYER_HEADER_GUARD
#define MQTTLAYER_HEADER_GUARD
#include <MQTT.h>

#define MQTT_BUFFER_SIZE 1024


class MqttLayer {
public:
  MqttLayer();

  int8_t initMQTT(Client &c);
  int8_t connectMQTT(MQTTClientCallbackAdvanced cb);
  int8_t loop();
  bool connected();

private:
  MQTTClient mqttclient;

};

#endif // MQTTLAYER_HEADER_GUARD