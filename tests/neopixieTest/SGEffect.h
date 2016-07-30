#include <Arduino.h>
#include "Vector.h"

#define DEBUG_OUTPUT Serial

class SGEffect {
	public:
	
	Vector<uint32_t> frame;

	SGEffect(){};	
	SGEffect(int nodesTotal):_nodesTotal(nodesTotal),_isActive(0),_iter(0){};

	~SGEffect();

	void blank();	

	virtual boolean step(); // overwritten in each effect class
	virtual int test();

	boolean isActive(){return _isActive;};
	uint32_t p(int index); // Return color at pixel index.

	void 
		updatePrimary(), 
		updateSecondary(),
		updateTertiary(),
		updatePrimary(uint32_t c), 
		updateSecondary(uint32_t c),
		updateTertiary(uint32_t c),
		updateAssorted();

	uint32_t 
		wheel(byte WheelPos),
		randomColor(),
		randomWheel(),
		color(uint32_t c, int a),
		color(byte r, byte g, byte b),
		color(byte r, byte g, byte b, int a),
		rgba(byte r, byte g, byte b, int a),
		alpha(uint32_t c, int a);


	protected:
		boolean 
			_isActive,
			_firstRun,
			_fullOrWheel,
			_willTransition;

		int _nodesTotal;		
		uint32_t _primary,_secondary,_tertiary;
		uint16_t _iter,_iterMax;

		virtual void 
			init(),
			q(uint16_t pos, uint32_t color);

		
		static 	uint8_t 
			extractRed(uint32_t c),
			extractGreen(uint32_t c),
			extractBlue(uint32_t c),
			R(uint8_t from, uint8_t to);
};

#include "ColorWipe.h"
#include "Rainbow.h"
#include "RainbowCycle.h"
#include "RollDie.h"
#include "Scanner.h"
