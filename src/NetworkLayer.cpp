#include "NetworkLayer.h"
#include "Debug.h"
#include "passwd.h"

bool connected(){
    return WiFi.isConnected();
}

int8_t NetworkLayer::initWifi() {
  wl_status_t ret = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (ret == WL_CONNECTED){
    debugPrintLn("connected to wifi");
  } else if (ret == WL_IDLE_STATUS){
    debugPrintLn("maybe wrong ssid or so");
  }
  return EXIT_SUCCESS;
}

int8_t NetworkLayer::connectWifi() {
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