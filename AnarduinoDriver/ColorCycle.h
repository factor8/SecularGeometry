#ifndef EFF_ColorCycle
#define EFF_ColorCycle

class ColorCycle : public SGEffect {
	public:
	ColorCycle(){};
	ColorCycle(int nodesTotal):SGEffect(nodesTotal){
		if (DEBUG) { DEBUG_OUTPUT.println("Initializing effect: ColorCycle");}		
		init();		
		_iterMax = 255;
	};	
	~ColorCycle(){};

	boolean step(){
		if (_firstRun) {
			if (DEBUG) {DEBUG_OUTPUT.println("Beginning effect: ColorCycle");}
			_firstRun = 0;
		}
		
  		for (int i=_nodesTotal-1; i>=0; i--) {
			q(i+1,frame[i]);
		}		
		q(0,randomWheel());

		// test();
		return true;
	}; 

	// overwritten in each effect class
	int test() {		
	}
};

#endif
		

