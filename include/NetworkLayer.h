#ifndef NETWORKLAYER_HEADER_GUARD
#define NETWORKLAYER_HEADER_GUARD
#include <ESP8266WiFi.h>

#define WIFI_TIMEOUT_IN_100MS 100
#define EXIT_TIMEOUT 3

class NetworkLayer {
public:
  bool connected();

private:
  WiFiClient netRef;

  int8_t initWifi();
  int8_t connectWifi();
};

#endif // NETWORKLAYER_HEADER_GUARD