/**
 * Wrapper for everything that goes to or comes from Homeassistant:
 *  also contains Wifi, MQTT; this could go into theyr own classes
 *  but i dont want to separate them right now
 */

#include <Arduino.h>
#include <MQTT.h>
#include <ESP8266WiFi.h>

#include "StripWrapper.h"
#include "passwd.h"

#define MQTT_BUFFER_SIZE 512
#define WIFI_TIMEOUT_IN_100MS 100

#define debugHomeassistant
#ifdef debugHomeassistant
#   define debugPrintf( ... ) Serial.print( __VA_ARGS )
#else
#   define debugPrintf( ... )
#endif

class Homeassistant
{
public:
    struct Status
    {
        union Color
        {
            uint32_t rgbw;
            struct channels
            {
                uint8_t r;
                uint8_t g;
                uint8_t b;
                uint8_t w;
            };
            
        };
        
    };
    
    /**
     * Strip musst be initialized
     */
    Homeassistant(const char *const * availableAnimations);
    ~Homeassistant();
    bool connect();
    bool reconnect();
    bool connected();
    bool sendStatus();
    void onStatusReceived();
private:
    /* data */
    const char *const * availableAnimations;
    MQTTClient mqttclient;
    WiFiClient netRef;

    /* functions */
    bool initWifi();
    bool connectWifi();
    bool initMQTT();
    bool connectMQTT();
    bool registerLight();
};

