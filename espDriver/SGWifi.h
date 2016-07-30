#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include "Vector.h"

// #define DEBUG_OUTPUT Serial

enum SGWIFI_STATUS {WIFISTATUS_INITIALIZING,WIFISTATUS_CONNECTING,WIFISTATUS_APSETUP,WIFISTATUS_CONNECTED,WIFISTATUS_DISABLED,WIFISTATUS_FAILED};
// enum SGWIFI_MODE {WIFI,WIFI_AP,};

class SGWifi {
   public:
      SGWifi(){};
      virtual ~SGWifi();

      boolean connected();

      String 
         getSSID(),
         getDNS();

      boolean 
         updateConnection(String ssid,String pass),
         updateDNS(String ssid,String pass),
         isMdns(String str),
         isIP(String str);

      void 
         init();
         
      int 
         persist();

   protected:
      String 
         _ssid,
         _pass,
         _dns,
         _fallbackSSID,
         _fallbackPass;

      uint8_t 
         _status, 
         _mode,
         _channel;
      
      unsigned long
         _start,
         _timeout;      

      std::unique_ptr <DNSServer>        dnsServer;
      // std::unique_ptr <ESP8266WebServer> webServer;
      // std::unique_ptr <WebSocketsServer> wsServer;

      byte DNS_PORT = 53;
      byte WEB_PORT = 80;      
      byte WS_PORT = 81;

};