
class Parent {
	public:
	uint32_t* colors;
	int _nodesTotal;
	Parent(){};
	// :colors(new uint32_t[nodesTotal])
	// Parent(int nodesTotal){}
	Parent(int nodesTotal):_nodesTotal(nodesTotal){};
	// Parent(int nodesTotal)::_nodesTotal(_nodesTotal){
		// uint32_t arr[nodesTotal];

			// uint32_t n;
			// colors = sizeof(uint32_t n) * nodesTotal;
	// };
	~Parent();

	void blank(){
		for (int i=0; i < _nodesTotal; i++) {
      		colors[i] = 0xFFFFFF;
    	}
	}

	virtual int test();
};

#include "Child.h"