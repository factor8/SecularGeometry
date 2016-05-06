
class RainbowCycle : public SGEffect {
	public:
	boolean randomize;
	RainbowCycle(){};
	RainbowCycle(uint32_t frequency,int nodesTotal):SGEffect(nodesTotal){
		init();
		_iterMax = 255;
	}
	~RainbowCycle();

	boolean step(){		
		if (_iter >= _iterMax) {
   			// if (randomize)updatePrimary();
    		_iter = 0;     		
  		}

		for (int i=0; i < _nodesTotal; i++) {
    		q(i,wheel( ((i * 256 / _nodesTotal) + _iter) % 256) );
  		}
		
		// Serial.println(frame[_iter]);
		// Serial.println(colors[_iter],HEX);
		_iter++;

		return true;
	}; // overwritten in each effect class
	
	int test() {		
		return _nodesTotal;
	}
};