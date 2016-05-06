#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_WS2801.h>

// #include "Vector.h"
#include "SGEffect.h"

// #include "ColorWipe.h"

#define DATAPIN   2 // Data pin for serial communication to shift registers
#define CLOCKPIN  3 // Clock pin for serial communication to shift registers
#define TOUCHPIN  5 // Touch sensitive switch

#define pixelsTotal 20
#define frameRate 40 // 40ms = 25fps


Vector<SGEffect*> effects;

Adafruit_WS2801* strip = new Adafruit_WS2801(pixelsTotal, DATAPIN, CLOCKPIN);

ColorWipe* effectA = new ColorWipe(100,0xFFF000,pixelsTotal);
ColorWipe* effectB = new ColorWipe(100,0xFFF000,pixelsTotal);

long now,then;
uint32_t* colors[pixelsTotal];

// #setup
void setup()
{
  Serial.begin(115200);
  Serial.println("Setting Up...");
  // Serial.setDebugOutput(true);
 
  effects.push_back(effectA);
  effects.push_back(effectB);

  strip->begin();
  strip->show();

}

void loop() {

	now = millis();   // This moment is beautiful.
  
	if (now >= then+frameRate) {

    	then = now;    

    	// if (selector != future_selector) churn();

      // int x = effects[1]->test();
      int x = effectA->test();

      Serial.println(x);


      // for(iter = this->effects.begin(); iter != this->effects.end(); ) {
      // // Ask effect to render
      //   if ( (*iter)->render() ) {
      //   }
      // }


    	// effectA->step(); // Send the 1011 and let the people have some fun.

      // effects[0]->step();
  		
  		uint32_t new_color = 0xFFF000;
    	for(int i=0; i<pixelsTotal; i++){    

    		// new_color = effectA->p(i);
        new_color = 0;
    		
    		Serial.println(new_color,HEX);
    		// if (brightness!=100) {
      			// new_color = color(new_color,brightness);
    		// }
          
		    // Load up the stip.
		    strip->setPixelColor(i,new_color);
		}
  		strip->show();
  	}

  //  Listeners

  // 	byte y = 10;
  // 	while(Serial.available() > 0 && (y-- > 0)) { // Listen on the serial connection.
		// char x = Serial.read();
		// interceptSerial(x);
  // 	}

}
