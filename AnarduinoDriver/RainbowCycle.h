

class RainbowCycle : public SGEffect {
	public:

	RainbowCycle(){};
	RainbowCycle(int nodesTotal):SGEffect(nodesTotal){
		if (DEBUG) { DEBUG_OUTPUT.println("Initializing effect: RainbowCycle");}		
		init();		
		_iterMax = 255*5;
	};	
	~RainbowCycle(){};

	boolean step(){
		if (_firstRun) {
			if (DEBUG) {DEBUG_OUTPUT.println("Beginning effect: RainbowCycle");}
			_firstRun = 0;
		}

		for (int i=0; i < _nodesTotal; i++) {
    		q(i,wheel( ((i * 256 / _nodesTotal) + _iter) % 256) );
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



// #ifndef EFF_RainbowCycle
// #define EFF_RainbowCycle

// class RainbowCycle : public SGEffect {
// 	public:

// 	RainbowCycle(){};
// 	RainbowCycle(int nodesTotal):SGEffect(nodesTotal){
// 		if (DEBUG) { DEBUG_OUTPUT.println("Initializing effect: RainbowCycle");}		
// 		init();
// 		_iterMax = 255*5;
// 	};	
// 	~RainbowCycle();

// 	boolean step(){
// 		if (_firstRun) {
// 			if (DEBUG) {DEBUG_OUTPUT.println("Beginning effect: RainbowCycle");}
// 			_firstRun = 0;
// 		}

// 		for (int i=0; i < _nodesTotal; i++) {
//     		q(i,wheel( ((i * 256 / _nodesTotal) + _iter) % 256) );
//   		}

//   		return true;
// 	}; 

// 	// overwritten in each effect class
// 	int test() {		
// 	}
// };

// #endif
