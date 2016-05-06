// This file is included from SGEffect.h
#include "SGEffect.h"
/*
 * The color wipe effect changes each pixel in turn
 * to the given color.
 */
#ifndef __ColorWipe__
#define __ColorWipe__

#define DEBUG_OUTPUT Serial

class ColorWipe: public SGEffect {
  public:
  	uint32_t** colors;
  	ColorWipe(){};
  	// ColorWipe(int nodesTotal);
    ColorWipe(int nodesTotal, uint32_t frequency, uint32_t color);
    ~ColorWipe();

   	int test() {return 1138;}

    boolean step();
};

#endif
