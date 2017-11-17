
#include <Arduino.h>

// #include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <Hash.h>
#include <WebSocketsServer.h>
// #include <WiFiClient.h>

#include "SGEffect.h"
#include "SGWifi.h"
#include "SGFileServer.h"
#include "SGConfigFile.h"


#define frameRate 24 // 40ms = 25fps

long now,then;
uint8_t _selector = 0;
uint8_t _pastSelector, _futureSelector;

// Serial Input Vars
char messageBuffer[8];      // Can't forsee more than 8c*s as long as we stay away from long pattern titles.
uint8_t  bufferIndex = 0;   // This global manages the buffer index.
uint8_t readMode = 0;           // Wait.

enum eName {eOff,eColorWipe,eRainbow,eRainbowCycle,eScanner};

Vector<SGEffect*> effects;

// Server Vars
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WebSocketsServer webSocket = WebSocketsServer(81);

// Touch Vars
boolean touching = false;
boolean overtimeTouch = false;
long firstTouch,touchDuration,lastTouchAction;

// Transition vars
boolean transitioning = 0;
uint16_t transitionTime = 1500; // time to transition in ms. ///move this into SGEffect i think
short currentTransitionStep = 0;
short transitionSteps;

boolean loadConfigFile();
void configure();
void sSetSelector();
void setSelector(uint8_t s);
void sSetFrequency();
void setFrequency(uint16_t s);
void sSetBrightness();
void setBrightness(uint8_t b);
void toggleDebug();
void interceptSerial(char x);
void interceptTouch();
void toggleAutomatic();
void statusUpdate();
void resetMessageBuffer(){ memset( messageBuffer, '\0', sizeof(messageBuffer) ); }
uint8_t readCapacitivePin(int pinToMeasure);
void flash();
boolean drive();

// -- Wifi // File System Functions
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
#include "WebServer.h"
