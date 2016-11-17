#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsClient.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <Hash.h>
// #include "SGWifi.h"

// Serial Vars
uint8_t DEBUG = 1; // Levels for debugging.

// Time Vars 
long 
  now,
  then;

uint16_t /// set defaults
  iter,
  itermax,
  interval,
  intervalCount, 
  phase;

boolean first_run;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WebSocketsClient webSocket;
// SGWifi* wifi = new SGWifi();

// holds the current upload
File fsUploadFile;

// Serial Input Vars
char messageBuffer[8];      // Can't forsee more than 8c*s as long as we stay away from long pattern titles.
uint8_t  bufferIndex = 0;   // This global manages the buffer index.
uint8_t  readMode = 0;      // Wait


// #setup
void setup();
void loop();
void burn();
void flash();
void flash(uint16_t d);
void spiral();

// -- Wifi // File System Functions
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
String formatBytes(size_t bytes);
String getContentType(String filename);
bool handleFileRead(String path);
void handleFileUpload();
void handleFileDelete();
void handleFileCreate();
void handleFileList();

void interceptSerial(char x);
void statusUpdate();
void toggleDebug();
void toggleVerbose();


