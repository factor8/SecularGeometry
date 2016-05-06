#include <Arduino.h>
#include "Vector.h"
#include "Parent.h"

Vector<Parent*> children;
// Parent* child = new Parent();
Child* child = new Child(7);

void setup() {
	Serial.begin(115200);
	Serial.println("Startng up...");
	int x;
	
	// children.push_back(child);
	// x = children[0]->test();
	// 
	x = child->test();
	
	Serial.println(x);
	Serial.println("Running...");
}
void loop() {

}