#include "Homeassistant.h"
#include <ArduinoJson.h>


Homeassistant::Homeassistant(StripWrapper * strip)
    :strip(strip),
    mqttclient(MQTT_BUFFER_SIZE)
{
  initWifi();
  initMQTT();
}

RETVAL Homeassistant::connect()
{
  RETVAL ret=0;
  ret=connectWifi();
  if (ret != 0){
    return ret;
  }
  ret=connectMQTT();
  if (ret != 0){
    return ret;
  }
  ret=registerLight();
  if (ret != 0){
    return ret;
  }
  return EXIT_SUCCESS;
}

RETVAL Homeassistant::reconnect()
{
  //check wifi
  //check MQTT
  //check Homeassistant
  return EXIT_SUCCESS;
}

RETVAL Homeassistant::connected()
{
  if (!WiFi.isConnected()){
    return EXIT_HA_WIFI_NOT_CONNECTED | EXIT_HA_MQTT_NOT_CONNECTED;
  }
  if (!mqttclient.connected()){
    return EXIT_HA_MQTT_NOT_CONNECTED;
  }
  return EXIT_SUCCESS;
}

RETVAL Homeassistant::sendStatus(Status s)
{
  return 0;
}

void Homeassistant::onStatusReceived()
{

}

RETVAL Homeassistant::initWifi()
{
    wl_status_t ret = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    if (ret != 0)
    {
        debugPrintf("Error initWifi code: %i", ret);
        return -1;
    }
    return 0;
}

RETVAL Homeassistant::connectWifi()
{
    debugPrintf("checking wifi...");
    uint16_t timeoutCtr = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        timeoutCtr++;
        if (timeoutCtr > WIFI_TIMEOUT_IN_100MS)
        {
            debugPrintf(" timeout\n");
            return -1;
        }
        debugPrintf(".");
        delay(100);
    }
    debugPrintf("\n");
    return 0;
}

RETVAL Homeassistant::initMQTT()
{
    mqttclient.begin(MQTT_ADDRESS, MQTT_PORT, netRef);
    return EXIT_SUCCESS;
}

RETVAL Homeassistant::connectMQTT()
{
    debugPrintf("connect mqtt...");
    while (!mqttclient.connect(DEVICENAME, MQTT_USERNAME, MQTT_PASSWORD))
    {
        debugPrintf(".");
        delay(1000);
    }
    mqttclient.publish("test", "test");
    debugPrintf("\n");
    return EXIT_SUCCESS;
}

RETVAL Homeassistant::registerLight()
{
      // reset existing configuration
    //homeassistant.publish("homeassistant/light/table/config", "");
    //homeassistant.loop();

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
    for (uint8_t i = 0; i < strip->availableAnimationCount / strip->availableAnimationCount; i++)
    {
        effectList.add(strip->availableAnimations[i]);
    }

    char output[MQTT_BUFFER_SIZE];
    if (serializeJson(doc, output) > MQTT_BUFFER_SIZE)
    {
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
