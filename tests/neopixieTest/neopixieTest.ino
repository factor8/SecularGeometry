#include <Arduino.h>
// #include <SPI.h>
// #include <Adafruit_WS2801.h>

#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <Adafruit_Pixie.h>

#include "SGEffect.h"
#include "colorTools.h"

#define NEOPIXPIN 14 // Pin number for NeoPixels output
#define NUMPIXIES 24  // Number of Pixies in the strip

#define NUMNEOPIX 384 // Number of NeoPixels in the strip
#define PIXIEPIN  13 // Pin number for SoftwareSerial output

#define panelTotal 24
#define ringTotal 16

#define frameRate 24 // 40ms = 25fps

Adafruit_NeoPixel* strip = new Adafruit_NeoPixel(NUMNEOPIX, NEOPIXPIN, NEO_GRB + NEO_KHZ800);

SoftwareSerial pixieSerial(-1, PIXIEPIN);
Adafruit_Pixie* pixieStrip = new Adafruit_Pixie(NUMPIXIES, &pixieSerial);

long now,then;
uint8_t _selector,_brightness;


enum eName {eColorWipe,eRainbow,eRainbowCycle,eScanner};
Vector<SGEffect*> effects;

// uint8_t effectList[5] = {eColorWipe,eColorWipe,eRainbow,eRainbowCycle,eScanner};
uint8_t effectList[1] = {eColorWipe};

uint8_t panels[panelTotal][ringTotal];

// Rainbow* childB = new Rainbow(100,pixelsTotal);


void setup() {
	Serial.begin(115200);
	Serial.println("Starting up...");
	
	pixieSerial.begin(115200); // Pixie REQUIRES this baud rate

	strip->begin();
  	strip->show();  	

  	strip->setBrightness(64);
  	// pixieStrip->setBrightness(100);

  	_selector = 0;

  	Serial.println("Mapping...");
	int x,y;
	
	int k=0;
	for(int i=0; i<panelTotal; i++){
		for(int j=0; j<ringTotal; j++){
	   	 	panels[i][j] = k;
	   	 	k = k+1;
		}
	}    

	Serial.println("Loading Effects...");
	for(int i=0; i<sizeof(effectList)-1; i++){
    	//SGEffect* effect = NULL;
	    switch (effectList[i]) {
	        case eColorWipe:
	          // effect = new ColorWipe(0xFF0000,100,pixelsTotal);
	          effects.push_back(new ColorWipe(0xFF0000,100,panelTotal));
	          break;
	        case eRainbow:
	          effects.push_back(new Rainbow(100,panelTotal));
	          break;
	        case eRainbowCycle:
	          effects.push_back(new RainbowCycle(100,panelTotal));
	          break;  
	        case eScanner:
	          effects.push_back(new Scanner(0xFF0000,100,panelTotal));
	          break;    
	        default:
	        	break;
	          // do something
	    }
	    // if (effect!=NULL) effects.push_back(effect);
	}

	Serial.println("Loading Config...");
	if (!loadConfig()) {
	}

	// effects.push_back(childA);
	// effects.push_back(childB);
	
	Serial.println("Running Tests...");
	
	// x = effects[0]->test();
	// y = effects[1]->test();
	// 
	// x = child->test();
	// Serial.println(effects.size());
	// Serial.println(x);
	// Serial.println(y);
	
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

		Serial.println("Stepping...");
		for (int e=0;e<effects.size();e++) {
			// if (effects[e]->isActive())effects[e]->step();
			// Serial.println(effects[e]->p(10));
		}
		
		

		Serial.println("Loading Pixels...");
		uint32_t new_color = 0xFF0000;	

		for(uint16_t k=0; k<256*5; k++) {

			/// Mode goes here.
			// if (mode = panels) {}
			for(int i=0; i<panelTotal; i++){
				// new_color = effects[_selector]->p(i);
				new_color = wheel(((i * 256 / panelTotal) + k) & 255);
				
				for (int j=0;j<16;j++) {
					// new_color = wheel(j);
					strip->setPixelColor((16*i)+j,new_color);
					
					// strip->setPixelColor((16*i)+j,new_color);
					// Serial.print((16*i)+j);
					// Serial.print(" . ");
				}
				Serial.println("");

				// pixieStrip->setPixelColor(i,new_color);
			}
			// strip->setPixelColor(0,effects[0]->p(1));		
			// Serial.println("Showing...");
  			strip->show();
  		}

  		Serial.println("Go around again...");
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