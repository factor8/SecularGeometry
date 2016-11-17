

// Secular Geometry Lantern Code
// By Jordan Layman 2016
// www.seculargeometry.com
// Idea Fab Labs, Chico // Santa Cruz

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <Hash.h>
#include <SPI.h>
#include <Adafruit_WS2801.h>

// fill the dots one after the other with said color
// good for testing purposes
void flavorWipe();
void fadeOut();
void flavorFill();
void rainbow();
void rainbowCycle();
void colorCycle();
void colorFade();
void colorCycleFade();
void sparkle();
void candleFlame();

/* -- LOAD UP OUR EFFECTS -- */
void (*menu[])() = {fadeOut,flavorFill,rainbow,rainbowCycle,colorCycle,colorCycleFade,sparkle,colorFade,candleFlame};

// #setup
void setup();
void loop();

void interceptTouch();

// Just flash black for a short bit.
void flash();
void rest();
void updateMode();
void updateSelector();
void churn();
void pour();
void q(uint16_t pos, uint32_t c);
void interceptSerial(char x);

void sSetSelector();
void setSelector(uint8_t s);

void sSetFrequency();
void setFrequency(uint16_t s);
void sSetBrightness();
void sSetMode();
void setMode(uint8_t s);
void toggleAutoPilot();
void resetMessageBuffer();
void statusUpdate();
void setBrightness(uint8_t b);
void forwardSelector();
void backSelector();

void toggleDebug();
void toggleVerbose();

uint8_t readCapacitivePin(int pinToMeasure);

/* Helper functions */
void updatePrimary(uint32_t c);
void updateSecondary(uint32_t c);
void updateTertiary(uint32_t c);

void updatePrimary();
void updateSecondary();
void updateTertiary();

// Update a specific flavor.
void updateFlavor(uint8_t flavor, uint32_t c);
// Update all flavors randomly.
void updateAssorted();

//Shorter random function, static helper. 
uint8_t R(uint8_t from, uint8_t to);
uint32_t RandomColor();
uint32_t RandomWheel();

uint32_t rgba(byte r, byte g, byte b, int a);
uint32_t color(byte r, byte g, byte b, int a);
uint32_t color(byte r, byte g, byte b);
uint32_t color(uint32_t c, int a);
uint32_t alpha(uint32_t c, int a);
uint32_t combine(uint32_t color1,uint32_t color2);
uint32_t Wheel(byte WheelPos);
uint32_t Wheel(byte WheelPos, double alpha);

// Helpers to extract RGB from 32bit color. (/// This could be MACRO...)
uint8_t extractRed(uint32_t c);
uint8_t extractGreen(uint32_t c);
uint8_t extractBlue(uint32_t c);

// -- Wifi // File System Functions
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
String formatBytes(size_t bytes);
String getContentType(String filename);
bool handleFileRead(String path);
void handleFileUpload();
void handleFileDelete();
void handleFileCreate();
void handleFileList();

bool loadConfig();
bool saveConfig();

String freeRam (); 