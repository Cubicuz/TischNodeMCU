/**
 * Wrapper for everything that goes to or comes from Homeassistant:
 *  also contains Wifi, MQTT; this could go into theyr own classes
 *  but i dont want to separate them right now
 */
#ifndef HOMEASSISTANT_HEADER_GUARD
#define HOMEASSISTANT_HEADER_GUARD
#include <Arduino.h>
#include <MQTT.h>
#include <ESP8266WiFi.h>

#include "StripWrapper.h"
#include "passwd.h"

#define MQTT_BUFFER_SIZE 1024
#define WIFI_TIMEOUT_IN_100MS 100

#define debugHomeassistant
#ifdef debugHomeassistant
#   define debugPrintf( ... ) Serial.printf( __VA_ARGS__ )
#   define debugPrintLn( x ) Serial.println( x )
#else
#   define debugPrintf( ... )
#   define debugPrintLn( x )
#endif

// retvals
typedef int8_t RETVAL;
#define EXIT_HA_WIFI_NOT_CONNECTED 1
#define EXIT_HA_MQTT_NOT_CONNECTED 2


class Homeassistant
{
public:
    struct Status
    {
        union Color
        {
            uint32_t rgbw;
            struct Channels
            {
                uint8_t r;
                uint8_t g;
                uint8_t b;
                uint8_t w;
            } channels;
            
        } color;
        const char * animation;
        bool operator==(const Status &s) const;
    };
    
    /**
     * Strip musst be initialized
     */
    Homeassistant(StripWrapper * strip);
    ~Homeassistant();
    RETVAL connect();
    RETVAL reconnect();

    /**
     * return bitcode homeassistant, MQTT, WiFi
     */
    RETVAL connected();
    RETVAL sendStatus(const Status &s);
    void onStatusReceived(const Status &s);
private:
    /* data */
    StripWrapper * strip;
    Status status;
    WiFiClient netRef;
    MQTTClient mqttclient;


    /* functions */
    RETVAL initWifi();
    RETVAL connectWifi();
    RETVAL initMQTT();
    RETVAL connectMQTT();
    RETVAL registerLight();

};

#endif // HOMEASSISTANT_HEADER_GUARD