// effect.cpp
#ifndef __SGEffect__
#define __SGEffect__

#include "Arduino.h"
#include "EasingLibrary.h"
#include "Vector.h"

// #include <functional> ///NOT SURE WHAT THIS IS

class SGEffect {
	
	public:
		
		uint32_t** colors;
		int _nodesTotal;

		SGEffect(){};
		SGEffect(int nodesTotal){};
		virtual ~SGEffect();

		virtual int test();
		virtual boolean step(); // overwritten in each effect class
		
		uint32_t p(int index); // Return color at pixel index.

		void 
			updatePrimary(), 
			updateSecondary(),
			updateTertiary(),
			updatePrimary(uint32_t c), 
			updateSecondary(uint32_t c),
			updateTertiary(uint32_t c),
			updateAssorted();

	protected:
		boolean _firstRun;
		boolean _fullOrWheel;
		
		uint16_t _iter;
		uint32_t _primary,_secondary,_tertiary;
		
		void init(int nodesTotal);

		uint32_t 
			wheel(byte WheelPos),
			randomColor(),
			randomWheel(),
			color(uint32_t c, int a),
			color(byte r, byte g, byte b),
			color(byte r, byte g, byte b, int a),
			rgba(byte r, byte g, byte b, int a),
			alpha(uint32_t c, int a);

		static 	uint8_t 
			extractRed(uint32_t c),
			extractGreen(uint32_t c),
			extractBlue(uint32_t c),
			R(uint8_t from, uint8_t to);
};

// Load up each of our custom effect generators...
#include "ColorWipe.h"

#endif //SGEffect

