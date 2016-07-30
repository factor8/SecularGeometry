
class ColorFill : public SGEffect {
	public:

	ColorFill(){};
	ColorFill(int nodesTotal):SGEffect(nodesTotal){
		if (DEBUG) { DEBUG_OUTPUT.println("Initializing effect: ColorFill");}		
		init();		
		_iterMax = _nodesTotal;
		_frequency = 10000; /// This could be weird but we really don't need to refresh this often.
	};	
	~ColorFill(){};

	boolean step(){
		if (_firstRun) {
			if (DEBUG) {DEBUG_OUTPUT.println("Beginning effect: ColorFill");}
			_firstRun = 0;
		}

		for (int i=0; i < _nodesTotal; i++) {
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