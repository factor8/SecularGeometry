
class Rainbow : public SGEffect {
	public:
	boolean randomize;
	Rainbow(){};
	Rainbow(uint32_t frequency,int nodesTotal):SGEffect(nodesTotal){
		init();
		_iterMax = 255;
	}
	~Rainbow();

	boolean step(){		
		if (_iter >= _iterMax) {
   			if (randomize)updatePrimary();
    		_iter = 0;     		
  		}

		for (int i=0; i < _nodesTotal; i++) {
    		q(i, wheel( (_iter) % 255));
    		// frame[i] = wheel( (_iter) % 255); 
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