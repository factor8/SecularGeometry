
class PulseRunner : public SGEffect {
	public:

	PulseRunner(){};
	PulseRunner(int nodesTotal):SGEffect(nodesTotal){
		if (DEBUG) { Serial.println("Initializing effect: PulseRunner");}		
		init();		
		_iterMax = _nodesTotal;
		_frequency = 60; /// This could be weird but we really don't need to refresh this often.
		stepwidth = 24;
		blur = 6;
	};	
	~PulseRunner(){};

	boolean step(){
		if (_firstRun) {
			if (DEBUG) {Serial.println("Beginning effect: PulseRunner");}
			_firstRun = 0;
		}		
		
		for (int i=0; i < _nodesTotal/stepwidth; i++) {
    		
    		// for (int j = 0; j < blur	)

    		q(i,_primary);
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