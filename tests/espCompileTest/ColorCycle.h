
class ColorCycle : public SGEffect {
	public:

	ColorCycle(){};
	ColorCycle(int nodesTotal):SGEffect(nodesTotal){
		init();
		_iterMax = _nodesTotal;
	};
	~ColorCycle();

	boolean step(){
		if (_firstRun) {
			if (DEBUG) {Serial.println(F("Beginning effect: ColorCycle"));}
			_firstRun = 0;
		}

		
  		for (int i=_nodesTotal-1; i>=0; i--) {
			q(i+1,frame[i]);
		}		
		q(0,randomWheel());

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