#include "Homeassistant.h"

Homeassistant::Homeassistant(const char *const *availableAnimations)
    : mqttclient(MQTT_BUFFER_SIZE),
      availableAnimations(availableAnimations)
{
}

bool Homeassistant::connect()
{
}

bool Homeassistant::reconnect()
{
}

bool Homeassistant::connected()
{
}

bool Homeassistant::sendStatus()
{
}

void Homeassistant::onStatusReceived()
{
}

bool Homeassistant::initWifi()
{
    wl_status_t ret = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    if (ret != 0)
    {
        debugPrintf("Error initWifi code: %i", ret);
        return false;
    }
    return true;
}

bool Homeassistant::connectWifi()
{
    debugPrintf("checking wifi...");
    uint16_t timeoutCtr = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        timeoutCtr++;
        if (timeoutCtr > WIFI_TIMEOUT_IN_100MS)
        {
            debugPrintf(" timeout\n");
            return false;
        }
        debugPrintf(".");
        delay(100);
    }
    debugPrintf("\n");
    return true;
}

bool Homeassistant::initMQTT()
{
    mqttclient.begin(MQTT_ADDRESS, MQTT_PORT, netRef);
}

bool Homeassistant::connectMQTT()
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

bool Homeassistant::registerLight()
{
}
