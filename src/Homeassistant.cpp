#include "Homeassistant.h"

Homeassistant::Homeassistant(const char *const *availableAnimations)
    : mqttclient(MQTT_BUFFER_SIZE),
      availableAnimations(availableAnimations)
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
}

RETVAL Homeassistant::reconnect()
{
  //check wifi
  //check MQTT
  //check Homeassistant

}

RETVAL Homeassistant::connected()
{
  if (!WiFi.isConnected()){
    return HA_WIFI_NOT_CONNECTED | HA_MQTT_NOT_CONNECTED;
  }
  if (!mqttclient.connected()){
    return HA_MQTT_NOT_CONNECTED;
  }

}

RETVAL Homeassistant::sendStatus()
{
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
    for (uint8_t i = 0; i < sizeof(tischleds.availableAnimations) / sizeof(*tischleds.availableAnimations); i++)
    {
        effectList.add(tischleds.availableAnimations[i]);
    }

    char output[MQTT_BUFFER_SIZE];
    if (serializeJson(doc, output) > MQTT_BUFFER_SIZE)
    {
        debugPrintLn("ARRAY OUT OF BOUNDS");
        return;
    }

    debugPrintf("doc Memoryusage: %u\n", doc.memoryUsage);
    debugPrintf("mqtt memory: %u\n", strlen(output));
    debugPrintLn(output);

    bool retVal = homeassistant.publish("homeassistant/light/table/config", output);

    debugPrintf("publish: ");
    debugPrintLn(retVal);

}
