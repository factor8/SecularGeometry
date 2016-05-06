#include <Arduino.h>
#include "Vector.h"
#include "Parent.h"

Vector<Parent*> children;
Child* childA = new Child(12);
Child* childB = new Child(20);

void setup() {
	Serial.begin(115200);
	Serial.println("Startng up...");
	int x,y;
	

	children.push_back(childA);
	children.push_back(childB);
	
	x = children[0]->test();
	y = children[1]->test();
	// 
	// x = child->test();
	Serial.println(children.size());
	Serial.println(x);
	Serial.println(y);
	Serial.println("Running...");
}
void loop() {

}