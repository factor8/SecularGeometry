
class RainbowCycle : public SGEffect {
	public:
	
	RainbowCycle(){};
	RainbowCycle(int nodesTotal):SGEffect(nodesTotal){	
		if (DEBUG) { DEBUG_OUTPUT.println(F("Initializing effect: RainbowCycle"));}		
		init();
		this->_iterMax = 255*5;
	};
	RainbowCycle(uint32_t frequency,int nodesTotal):SGEffect(frequency,nodesTotal){init();}
	~RainbowCycle();

	boolean step(){
		if (_firstRun) {
			if (DEBUG) {Serial.println(F("Beginning effect: RainbowCycle"));}
			_firstRun = 0;
		}

		for (int i=0; i < _nodesTotal; i++) {
    		q(i,wheel( ((i * 256 / _nodesTotal) + _iter) % 256) );
  		}
		
		// if (_iter == 0 || _iter == 250) status();
		
		return true;
	}; // overwritten in each effect class
	
	int test() {		
		return _nodesTotal;
	}
};