
class Rainbow : public SGEffect {
	public:

	Rainbow(){};
	Rainbow(int nodesTotal):SGEffect(nodesTotal){
		if (DEBUG) { DEBUG_OUTPUT.println("Initializing effect: Rainbow");}		
		init();		
		_iterMax = 255;
	};	
	~Rainbow(){};

	boolean step(){
		if (_firstRun) {
			if (DEBUG) {DEBUG_OUTPUT.println("Beginning effect: Rainbow");}
			_firstRun = 0;
		}

		for (int i=0; i < _nodesTotal; i++) {
    		q(i, wheel( (_iter) % 255));
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

// class Rainbow : public SGEffect {
// 	public:

// 	Rainbow(){};
// 	Rainbow(int nodesTotal):SGEffect(nodesTotal) {
// 		init();
// 		_iterMax = 255;
// 	};
// 	~Rainbow();

// 	boolean step(){
// 		if (_firstRun) {
// 			if (DEBUG) {DEBUG_OUTPUT.println("Beginning effect: Rainbow");}
// 			_firstRun = 0;
// 		}

// 		for (int i=0; i < _nodesTotal; i++) {
//     		q(i, wheel( (_iter) % 255));
//   		}		
// 		return true;
// 	}; 

// 	// overwritten in each effect class
// 	int test() {	
// 		// DEBUG_OUTPUT.print("Frequency:");	
// 		// DEBUG_OUTPUT.println(_frequency);
// 		// DEBUG_OUTPUT.print(_iter);
// 		// DEBUG_OUTPUT.print("/");
// 		// DEBUG_OUTPUT.println(_iterMax);
// 		// DEBUG_OUTPUT.print("Color");
// 		// DEBUG_OUTPUT.println(wheel( (_iter) % 255));
		
// 		// Serial.println(frame[0],HEX);
// 		// return _nodesTotal;
// 	}
// };