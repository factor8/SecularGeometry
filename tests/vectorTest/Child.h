// #include "Parent.h"
class Child : public Parent {
	public:
	uint32_t* colors;
	Child(){};
	Child(int nodesTotal):Parent(nodesTotal),colors(new uint32_t[nodesTotal]){	

	// Child(int nodesTotal):_nodesTotal(nodesTotal),colors(new uint32_t[nodesTotal]){
		

		// _nodesTotal = nodesTotal;
		for (int i=0; i < _nodesTotal; i++) {
      		colors[i] = i;
    	}
  		// blank();
	}

	~Child();
	int test() {
		// for (int i=0; i < _nodesTotal; i++) {
      		
  //   	}
		return _nodesTotal;
		return colors[4];
	}
};