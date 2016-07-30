#include "SGEffect.h"

// SGEffect::SGEffect(int nodesTotal):_nodesTotal(nodesTotal) {

// }

void SGEffect::init() {
	for(int i=0; i<_nodesTotal; i++){
		frame.push_back(0);
	}

}

void SGEffect::blank(){
	for (int i=0; i < _nodesTotal; i++) {
  		frame[i] = 0;
	}
}
uint32_t SGEffect::p(int index) {
	if (index > 0 && index < _nodesTotal)
	return frame[index];
}

void SGEffect::q(uint16_t pos, uint32_t color) {
	int p;

  // In here we need to be able to check the orientation, mode, and then check to see if we are compositing two effects.  



  	if (pos > _nodesTotal) {
		  Serial.println("OUT OF BOUNDS");    
		  return;
    }
    frame[pos] = color;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t SGEffect::wheel(byte WheelPos) {
  if (WheelPos < 85) {
   return color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

//Shorter random function, static helper. 
uint8_t SGEffect::R(uint8_t from, uint8_t to){ return random(from, to); } 
uint32_t SGEffect::randomColor(){ return color(R(0,255), R(0,255), R(0,255)); }
uint32_t SGEffect::randomWheel() { return wheel(R(0,255)%255); }


uint32_t SGEffect::rgba(byte r, byte g, byte b, int a) {
  
  int rr = (r*a)/100;
  int gg = (g*a)/100;
  int bb = (b*a)/100;

  return color(rr,gg,bb);
}

// Create a 24 bit color value from R,G,B
uint32_t SGEffect::color(byte r, byte g, byte b, int a) {
  int rr = r*a;
  int gg = g*a;
  int bb = b*a;

  return color(rr,gg,bb);
}

// Create a 24 bit color value from R,G,B
uint32_t SGEffect::color(byte r, byte g, byte b) {
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;  
  c |= b;
  
    return c;
}

uint32_t SGEffect::color(uint32_t c, int a) {
  
  uint8_t r = extractRed(c);
  uint8_t g = extractGreen(c);
  uint8_t b = extractBlue(c);
    
  return rgba(r,g,b,a);
}

uint32_t SGEffect::alpha(uint32_t c, int a) {
  
  uint8_t r = extractRed(c);
  uint8_t g = extractGreen(c);
  uint8_t b = extractBlue(c);
    
  return rgba(r,g,b,a);
}

// Helpers to extract RGB from 32bit color. (/// This could be MACRO...)
uint8_t SGEffect::extractRed(uint32_t c) { return (( c >> 16 ) & 0xFF); } 
uint8_t SGEffect::extractGreen(uint32_t c) { return ( (c >> 8) & 0xFF ); } 
uint8_t SGEffect::extractBlue(uint32_t c) { return ( c & 0xFF ); }

void SGEffect::updatePrimary(uint32_t c) {_primary = c;}
void SGEffect::updateSecondary(uint32_t c) {_secondary = c;}
void SGEffect::updateTertiary(uint32_t c) {_tertiary = c;}
void SGEffect::updatePrimary() {if (_fullOrWheel) {_primary = randomWheel();} else {_primary = randomColor();}}
void SGEffect::updateSecondary() {if (_fullOrWheel) {_secondary = randomWheel();} else {_secondary = randomColor();}}
void SGEffect::updateTertiary() {if (_fullOrWheel) {_tertiary = randomWheel();} else {_tertiary = randomColor();}}

// Update all flavors randomly.
void SGEffect::updateAssorted() {
  _primary = randomWheel();
  _secondary = randomWheel();
  _tertiary = randomWheel();
}

