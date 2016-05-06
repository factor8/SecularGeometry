#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_WS2801.h>

#include "SGEffect.h"

#define DATAPIN   2 // Data pin for serial communication to shift registers
#define CLOCKPIN  3 // Clock pin for serial communication to shift registers
#define TOUCHPIN  5 // Touch sensitive switch
#define pixelsTotal 20

#define frameRate 24 // 40ms = 25fps

Adafruit_WS2801* strip = new Adafruit_WS2801(pixelsTotal, DATAPIN, CLOCKPIN);

long now,then;
uint8_t _selector;

enum eName {eColorWipe,eRainbow,eRainbowCycle,eScanner};
Vector<SGEffect*> effects;

uint8_t effectList[5] = {eScanner,eColorWipe,eColorWipe,eRainbow,eRainbowCycle};

uint8_t panels[panelTotal][ringTotal];

// Rainbow* childB = new Rainbow(100,pixelsTotal);

void setup() {
	Serial.begin(115200);
	Serial.println("Starting up...");
	
	strip->begin();
  	strip->show();

  	// pixieSerial.begin(115200); // Pixie REQUIRES this baud rate
  	// pixieStrip.setBrightness(100);

	int x,y;
	
	for(int i=0; i<sizeof(effectList)-1; i++){
    	//SGEffect* effect = NULL;
	    switch (effectList[i]) {
	        case eColorWipe:
	          // effect = new ColorWipe(0xFF0000,100,pixelsTotal);
	          effects.push_back(new ColorWipe(0xFF0000,100,pixelsTotal));
	          break;
	        case eRainbow:
	          effects.push_back(new Rainbow(100,pixelsTotal));
	          break;
	        case eRainbowCycle:
	          effects.push_back(new RainbowCycle(100,pixelsTotal));
	          break;  
	        case eScanner:
	          effects.push_back(new Scanner(0xFF0000,100,NUMNEOPIX));
	          break;    
	        default:
	        	break;
	          // do something
	    }
	    // if (effect!=NULL) effects.push_back(effect);
	}

	if (!loadConfig()) {
	}

	// effects.push_back(childA);
	// effects.push_back(childB);
	
	x = effects[0]->test();
	y = effects[1]->test();
	// 
	// x = child->test();
	Serial.println(effects.size());
	Serial.println(x);
	Serial.println(y);
	Serial.println("Running...");
}
void loop() {
	
	now = millis();   // This moment is beautiful.

	if (now >= then+frameRate) {
		then = now;

		// childA->step();
		// if (effects[1]->step()) {
			// Serial.println("Stepping");
			// Serial.println(effects[0]->test());
			// effects[0]->p(i);
			// effects[0]->p(1);
		// }	
		for (int e=0;e<effects.size();e++) {
			if (effects[e]->isActive())effects[e]->step();
		}

		uint32_t new_color = 0;
		for(int i=0; i<pixelsTotal; i++){    
			new_color = effects[0]->p(i);
			strip->setPixelColor(i,new_color);
		}
		// strip->setPixelColor(0,effects[0]->p(1));
  		strip->show();
  	}
}

boolean loadConfig() {
	return false;
}

/*
class SGController {
	public:
		uint16_t _frameRate; // framerate. 25ms = 40fps

		SGController(){};
		SGController(uint16_t* effectList,){};
		virtual ~SGController(){};

		virtual boolean render();
		virtual boolean loadConfig();
		void loadEffect();
		void setBrightness();
		void setEffect();



	protected:

		enum eName {eColorWipe,eRainbow,eRainbowCycle};
		Vector<SGEffect*> _effects;

		uint8_t _brightness;
		uint16_t _selector;
		boolean _transitioning = 0;
};
*/