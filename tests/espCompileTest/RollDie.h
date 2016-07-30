#include <CircularEase.h>

class rollDie : public SGEffect {
	public:

	rollDie(){};
	rollDie(int nodesTotal):SGEffect(nodesTotal){init();};
	rollDie(uint32_t frequency,int nodesTotal):SGEffect(frequency,nodesTotal){
		if (DEBUG) { DEBUG_OUTPUT.println(F("Initializing effect: RollDie"));}
		init();
		_iterMax = 255;
	}
	~rollDie();

	boolean step(){		
		if (_iter >= _iterMax) {
   			if (_randomize)updatePrimary();
    		_iter = 0;     		
  		}

		for (int i=0; i < _nodesTotal; i++) {
    		q(i, wheel( (_iter) % 255));
    		// frame[i] = wheel( (_iter) % 255); 
  		}
		
		if (_iter == _nodesTotal) {
   			if (_randomize) updatePrimary();
  		}

		return true;
	}; // overwritten in each effect class
	
	int test() {		
		return _nodesTotal;
	}

	protected:
		CircularEase* ease;
		uint8_t die_number;
		uint16_t die_roll_pace,die_roll_steps, die_flash_duration;
		long die_roll_duration,die_role_start_time;
		long die_check_time;
		boolean die_roll_finished,die_flash;
};