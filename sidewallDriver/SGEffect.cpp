#include "SGEffect.h"

SGEffect::~SGEffect() {}

void SGEffect::init() {
	// Defaults
  for(int i=0; i<_nodesTotal; i++){
		frame.push_back(0);
	}    
  _iterMax = _nodesTotal;
  _willTransition = 1;
  _fullOrWheel = 1;

    /// Do we want to have this set defaults for iter, itermax, randomize, etc.?
}

void SGEffect::blank(){
	for (int i=0; i < _nodesTotal; i++) {
  		frame[i] = 0;
	}
}
void SGEffect::reset(){
    for (int i=0; i < _nodesTotal; i++) {
        frame[i] = 0;
    }
    _firstRun = 1;
    _iter = 0;

}
uint32_t SGEffect::p(int index) {
	if (index >= 0 && index < _nodesTotal)
	return frame[index];
}

void SGEffect::render() {
    unsigned long now = millis();
  
    if (!_isActive || (now - _lastRun) < _frequency) {
        return;
    }

    // Run our child effect code.
    step();

    if (_iter < _iterMax) {
        _iter++;
    } else {
        _iter = 0;

        if (_interval < _intervalMax) {
            _interval++;
        } else {
            _interval = 0;        
        }
        // if (DEBUG) { Serial.print("Interval updated to ");Serial.println(interval);  }
    }

  _lastRun = millis();
}

boolean SGEffect::step(){}
int SGEffect::test(){}

void SGEffect::q(uint16_t pos, uint32_t color) {
	int p;

  // In here we need to be able to check the orientation, mode, and then check to see if we are compositing two effects.  

  	if (pos > _nodesTotal) {
		Serial.println("OUT OF BOUNDS");    
		return;
    }
    frame[pos] = color;
}

Vector<uint32_t> SGEffect::exportFrame() {
  return frame;
}
void SGEffect::importFrame(Vector<uint32_t> f) {
  if (f.size() == frame.size()) {
    frame = f;
  } else {
    if (DEBUG) { Serial.print("Frame Import Mismatch: ");Serial.println(f.size());  }
  }  
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

uint32_t SGEffect::combine(uint32_t color1,uint32_t color2) {
  
  uint16_t new_r = extractRed(color1) + extractRed(color2);
  uint16_t new_g = extractGreen(color1) + extractGreen(color2);
  uint16_t new_b = extractBlue(color1) + extractBlue(color2);

  if (new_r > 255) new_r = 255;
  if (new_g > 255) new_g = 255;
  if (new_b > 255) new_b = 255;

  return color(new_r,new_g,new_b);  
}

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

// Update all flavors randomly.
void SGEffect::updateFrequency(uint32_t f) {_frequency = f;}
void SGEffect::updateIter(uint16_t i){_iter = i;}
void SGEffect::updateIterMax(uint16_t i){_iterMax = i;}
void SGEffect::updateIntervalMax(uint16_t i){_intervalMax = i;}
void SGEffect::updateWillTransition(boolean flag){_willTransition = flag;}


void SGEffect::updateRandom(boolean flag) {_randomize = flag;}
// Activate effect
void SGEffect::activate() {
    // if (DEBUG) { Serial.print("Activating effect ID: ");Serial.println(effect_id);  }
    _isActive = 1;
}
// Activate effect
void SGEffect::deactivate() {
    // if (DEBUG) { Serial.print("Deactivating effect ID: ");Serial.println(effect_id);  }
    _isActive = 0;
}

// Activate effect
void SGEffect::status() {
  Serial.println();
  // frequency, iter, lastRun, firstRun, primary, scondary effect_id?

  // Serial.print("Is Dave there?");
  Serial.println("<=== Effect Status Update ===>"); 
  Serial.print("Iter: ");
  Serial.println(_iter); 
  Serial.print("IterMax: ");
  Serial.println(_iterMax); 
  Serial.print("Interval: ");
  Serial.println(_interval); 
  Serial.print("IntervalMax: ");
  Serial.println(_intervalMax); 
  Serial.print("Frequency: ");
  Serial.println(_frequency);
  Serial.print("firstRun: ");
  Serial.println(_firstRun);
  Serial.print("_lastRun: ");
  Serial.println(_lastRun);
  Serial.print("_willTransition: ");
  Serial.println(_willTransition);
  
  Serial.print("Primary Color: ");
  Serial.println(_primary,HEX); 
  Serial.print("Secondary Color: ");
  Serial.println(_secondary,HEX); 
  Serial.print("Tertiary Color: ");
  Serial.println(_tertiary,HEX); 
  
  Serial.println("-------");
  Serial.print("panelsTotal: ");
  Serial.println(_nodesTotal);  

  // Serial.print("Phase: "));
  // Serial.println(phase);

  Serial.println("<===========>");
  Serial.println("");
  
  // for (int j=0;j<menu_count;j++) {
  //  Serial.println(j);  
  // }

}