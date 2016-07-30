
#include <Arduino.h>
#include "SGEffect.h"

#define frameRate 24 // 40ms = 25fps
// int pixelsTotal;
// boolean DEBUG = 0;
// boolean automatic = 0;

long now,then;
uint8_t _selector = 0;
uint8_t _pastSelector, _futureSelector;

// Serial Input Vars
char messageBuffer[8];      // Can't forsee more than 8c*s as long as we stay away from long pattern titles.
uint8_t  bufferIndex = 0;   // This global manages the buffer index.
uint8_t readMode = 0;           // Wait.

enum eName {eOff,eColorWipe,eRainbow,eRainbowCycle,eScanner};

Vector<SGEffect*> effects;

uint8_t effectList[7] = {eOff,eScanner,eColorWipe,eColorWipe,eRainbow,eRainbowCycle,eScanner};

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
