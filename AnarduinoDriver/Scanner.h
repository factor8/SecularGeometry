
class Scanner : public SGEffect {
	public:
	
	Scanner(){};
	Scanner(int nodesTotal):SGEffect(nodesTotal){
		if (DEBUG) { DEBUG_OUTPUT.println("Initializing effect: Scanner");}
		init();
		// updatePrimary(color);
	};
	~Scanner(){};

	boolean step(){
					
  		if (_firstRun) {
			if (DEBUG) {DEBUG_OUTPUT.println("Beginning effect: ColorWipe");}
			_firstRun = 0;
		}

		blank();
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