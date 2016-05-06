// This file is included from Effect.cpp
#include "ColorWipe.h"

ColorWipe::ColorWipe(int nodesTotal, uint32_t frequency, uint32_t color): SGEffect(nodesTotal), colors(new uint32_t[nodesTotal]) {
    // _nodesTotal = nodesTotal;
    for (int i=0; i < _nodesTotal; i++) {
        colors[i] = 0;
    }  

    _primary = color;
  // iter = 0;
}

boolean ColorWipe::step() {
    // DEBUG_OUTPUT.println("testdrive");
  
  if (_firstRun) {
    DEBUG_OUTPUT.println(F("Beginning effect: flavorWipe"));
    _firstRun=0;
  }

  // Flavor Wipe
  // for (int i=0; i < _nodesTotal; i++) {
  //     colors[i] = _primary;  
  //   }  

  if (_iter >= _nodesTotal) {
    updatePrimary();
    _iter = 0;    
  }

  // DEBUG_OUTPUT.println(_iter);
    
    colors[_iter] = _primary;
    DEBUG_OUTPUT.println(colors[_iter],HEX);
    
    _iter++;

    // q(iter, primary);

  return true;
}

