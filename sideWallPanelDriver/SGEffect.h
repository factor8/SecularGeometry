#include <Arduino.h>
#include "Vector.h"

class SGEffect {
	public:
	
	boolean DEBUG = true;

	Vector<uint32_t> frame;

	SGEffect(){};	
	SGEffect(int nodesTotal):_nodesTotal(nodesTotal),_isActive(0),_iter(0),_frequency(0){};
	SGEffect(int frequency, int nodesTotal):_nodesTotal(nodesTotal),_isActive(0),_iter(0),_frequency(frequency){};	

	virtual ~SGEffect();

	// Run child step() and handle timing and intervals.
	void render();
	
	// Quick clear all of frame.
	void blank();	
	
	// Debug outputs
	void status();
	virtual int test();


	boolean isActive(){return _isActive;};
	boolean willTransition(){return _willTransition;};
	uint32_t p(int index); // Return color at pixel index.

	void 
		updatePrimary(), 
		updatePrimary(uint32_t c), 
		updateSecondary(),		
		updateSecondary(uint32_t c),
		updateTertiary(),
		updateTertiary(uint32_t c),
		updateAssorted(),
		updateFrequency(uint32_t f),
		updateRandom(boolean flag),
		updateIter(uint16_t i),
		updateIterMax(uint16_t i),
		updateIntervalMax(uint16_t i),		
		updateWillTransition(boolean flag),
		importFrame(Vector<uint32_t> f),		
		activate(),
		deactivate(),
		reset();
		/// Could also have default() for resetting everything. Calls init and reset? maybe ony init() is necessary

	Vector<uint32_t> exportFrame();

	static uint32_t 
			wheel(byte WheelPos),
			randomColor(),
			randomWheel(),
			color(uint32_t c, int a),
			color(byte r, byte g, byte b),
			color(byte r, byte g, byte b, int a),
			rgba(byte r, byte g, byte b, int a),
			alpha(uint32_t c, int a),
			combine(uint32_t color1,uint32_t color2);

	static 	uint8_t 
		extractRed(uint32_t c),
		extractGreen(uint32_t c),
		extractBlue(uint32_t c),
		R(uint8_t from, uint8_t to);	

	protected:
		boolean 
			_isActive,
			_firstRun,			
			_fullOrWheel,
			_willTransition,
			_randomize;

		int _nodesTotal;		
		uint32_t _primary,_secondary,_tertiary;
		uint16_t _iter,_iterMax,_interval,_intervalMax;
		uint8_t _brightness = 100;

		unsigned long _lastRun = 0;
		unsigned long _effectDuration;
		uint32_t _frequency;

		/// virtual
		void 
			init(),			
			q(uint16_t pos, uint32_t color);	

		virtual boolean step(); // overwritten in each effect class

};

#include "ColorFill.h"
#include "ColorWipe.h"
#include "ColorCycle.h"
#include "Rainbow.h"
#include "RainbowCycle.h"
#include "Scanner.h"
#include "RollDie.h"

