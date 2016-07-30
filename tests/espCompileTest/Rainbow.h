
class Rainbow : public SGEffect {
	public:

	Rainbow(){};
	Rainbow(int nodesTotal):SGEffect(nodesTotal){
		init();
		_iterMax = 255;};
	Rainbow(uint32_t frequency,int nodesTotal):SGEffect(frequency,nodesTotal){
		init();		
	}
	~Rainbow();

	boolean step(){
		if (_firstRun) {
			if (DEBUG) {Serial.println(F("Beginning effect: Rainbow"));}
			_firstRun = 0;
		}

		for (int i=0; i < _nodesTotal; i++) {
    		q(i, wheel( (_iter) % 255));
  		}

		// test();
		return true;
	}; // overwritten in each effect class
	
	int test() {	
		Serial.print("Frequency:");	
		Serial.println(_frequency);
		Serial.print(_iter);
		Serial.print("/");
		Serial.println(_iterMax);
		Serial.print("Color");
		Serial.println(wheel( (_iter) % 255));
		
		// Serial.println(frame[0],HEX);
		// return _nodesTotal;
	}
};