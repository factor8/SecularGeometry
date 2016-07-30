
class ColorWipe : public SGEffect {
	public:

	ColorWipe(){};
	ColorWipe(int nodesTotal):SGEffect(nodesTotal){
		if (DEBUG) { DEBUG_OUTPUT.println(F("Initializing effect: ColorWipe"));}		
		init();		
		_iterMax = _nodesTotal;
	};
	// ColorWipe(uint32_t color, uint32_t frequency,int nodesTotal):SGEffect(frequency,nodesTotal){
	// 	updatePrimary(color);
	// 	init();
	// 	_iterMax = _nodesTotal;
	// }
	~ColorWipe();

	boolean step(){
		if (_firstRun) {
			if (DEBUG) {Serial.println(F("Beginning effect: ColorWipe"));}
			_firstRun = 0;
		}

		q(_iter,_primary);

		// Serial.println("Stepping");
		if (_iter == _nodesTotal) {
   			if (_randomize) updatePrimary();    		
    		blank();
  		}

		return true;
	}; 

	// overwritten in each effect class
	int test() {
		// for (int i=0; i < _nodesTotal; i++) {
      		
  //   	}
		return _nodesTotal;
		// return colors[4];
	}
};