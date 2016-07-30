
class ColorWipe : public SGEffect {
	public:
	boolean randomize;
	ColorWipe(){};
	ColorWipe(uint32_t color, uint32_t frequency,int nodesTotal):SGEffect(nodesTotal){
		init();
		updatePrimary(color);		
	}
	~ColorWipe();

	boolean step(){
		// Serial.println("Stepping");
		if (_iter >= _nodesTotal) {
   			if (randomize)updatePrimary();
    		_iter = 0; 
    		blank();   
  		}

  		q(_iter,_primary);
		
		// frame[_iter] = _primary;
		// frame[_iter] = randomWheel();
		
		// frame[_iter] = 200;

		// Serial.println(frame[_iter]);
		// Serial.println(colors[_iter],HEX);
		_iter++;

		return true;
	}; // overwritten in each effect class
	int test() {
		// for (int i=0; i < _nodesTotal; i++) {
      		
  //   	}
		return _nodesTotal;
		// return colors[4];
	}
};