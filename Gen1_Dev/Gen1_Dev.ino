#include <SPI.h>
#include <Adafruit_WS2801.h>
// #include <Adafruit_NeoPixel.h>
// #include <TrueRandom.h>

#define DATAPIN   2 // Data pin for serial communication to shift registers
// #define DATAPIN   6 // NEOPIXEL mandatory data pin for serial communication to shift registers
#define CLOCKPIN  3 // Clock pin for serial communication to shift registers
#define TOUCHPIN  5 // Touch sensitive switch

// Grid DEF & Vars
#define pixelsTotal 40
boolean autoPilot = true;
long effect_duration = 300000;
uint8_t brightness = 100;

// Instantiate Controller. Num Pix Automatically Generated.
Adafruit_WS2801* strip = new Adafruit_WS2801(pixelsTotal, DATAPIN, CLOCKPIN);
// Adafruit_NeoPixel* strip = new Adafruit_NeoPixel(pixelsTotal, DATAPIN, NEO_GRB + NEO_KHZ800);

#define loopDelay 40 // framerate. 40ms = 24fps

// Alignment,Direction,Pattern Enumeration
enum {asc,desc,   horizontal,vertical,  strand,panel, mirrored, ring};

uint16_t orientation = vertical;
uint16_t direction = desc;
uint16_t panelsTotal = pixelsTotal; // Default

// nodeTimeStamps becomes colorPallette
uint32_t colors[pixelsTotal]; /// We may be removing this...
uint32_t colors_past[pixelsTotal];

uint8_t active[pixelsTotal]; // Right now this is just for active sparkle pixels.
uint16_t active_count = 0;

// Serial Vars
extern boolean DEBUG = 0; // Flag for debugging.
extern boolean verbose = 0; // Flag for verbose debugging.

// Transition Vars
boolean will_transition = 1;
boolean transitioning = 0;

boolean full_or_wheel = 1; // 0 for full color, 1 for wheel color.
boolean randomize = 0;
boolean first_run = 1; // Flag so that we can trigger only once per effect.

uint16_t transition_time = 1500; // time to transition in ms.
short current_transition_step = 0;
short transition_steps;

// CTRL Vars 
long 
  now,
  then,
  effect_start_time;

uint16_t /// set defaults
  iter,
  itermax,
  interval,
  intervalCount,
  frequency,
  effectMS,
  effectMS_counter,
  sustain,
  density,
  decay,
  phase,
  selector,               // what effect settings in churn()
  effect_id,              // the id of the effect
  future_selector,
  future_mode,
  mode,
  polkadots;

uint32_t 
  primary,
  secondary,
  tertiary;

uint8_t a = 100; // Using this for alpha reference in Fade Out; /// move up or fix as needed
uint8_t red,green,blue = 0; // Using this to fade in.

// Serial Input Vars
char messageBuffer[8];      // Can't forsee more than 8c*s as long as we stay away from long pattern titles.
uint8_t  bufferIndex = 0;   // This global manages the buffer index.
uint8_t  readMode = 0;      // Wait


// Touch Vars
boolean touching = false;
boolean overtimeTouch = false;

long firstTouch,touchDuration,lastTouchAction;

/// at some point we may need to create a function that returns pixelsTotal based on mode (panel/mirror/etc) 

// fill the dots one after the other with said color
// good for testing purposes
void flavorWipe() {

  if (first_run) {
    if (DEBUG) Serial.println(F("Beginning effect: flavorWipe"));
    itermax = panelsTotal;
    first_run=0;
  }

  q(iter, primary);

}

void fadeOut() {
  
  if (first_run) {    
    if (DEBUG) {Serial.println(F("Beginning effect: fadeOut"));}
    a = 100;
    first_run=0;
  }  
  
  if (a > 0) {
    
    a = a - 1;

    for(int i=0;i<panelsTotal;i++)  {
      q(i,color(strip->getPixelColor(i),a));
    }
  } else {
    a = 100;
  }
}

// // Just fill the strand with a color
// void flavorFill() {
  
//   if (first_run) {    
//     if (DEBUG) {Serial.println(F("Beginning effect: flavorFill"));}    
//     first_run=0;    
//   }

//   for (int i=0; i < pixelsTotal; i++) {
//     q(i, primary);
//   }

// }

// Just fill the strand with a color
void flavorFill() {
  
  if (first_run) {    
    if (DEBUG) {Serial.println(F("Beginning effect: flavorFill"));}    
    first_run=0;    
  }

  
  for (int i=0; i < panelsTotal; i++) {
    if (polkadots && i%2==0) {
      q(i, secondary);  
    } else {
      q(i, primary);  
    }
    
  }

}

void rainbow() {
  if (DEBUG && first_run) {Serial.println(F("Beginning effect: rainbow"));first_run=0;}
  // frequency = 200; /// unhardcode
  itermax = 255;

  for (int i=0; i < panelsTotal; i++) {
    q(i, Wheel( (iter) % 255));
  }  
}

void rainbowCycle() {
  if (DEBUG && first_run) {Serial.println(F("Beginning effect: rainbowCycle"));first_run=0;}

  // frequency = 200; /// unhardcode
  itermax = 255;

  for (int i=0; i < panelsTotal; i++) {
    // tricky math! we use each pixel as a fraction of the full 96-color wheel
    // (thats the i / strip.numPixels() part)
    // Then add in j which makes the color go around per pixel
    // the % 96 is to make the wheel cycle around
    q(i, Wheel( ((i * 256 / panelsTotal) + iter) % 256) );
  }
}

// Moves colors along the strand. Frequency of 200 or lower is a nice pace. Should be rapid and exciting
void colorCycle() {
  
  if (DEBUG && first_run) {Serial.println(F("Beginning Effect colorCycle..."));first_run=0;}  
  
  // Move the chain along but don't go too far.
  for (int i=panelsTotal-1; i>=0; i--) {      
        
    q(i+1,colors[i]); /// ring issue

  }
  
  // Add our new random color at the beginning.
  q(0,RandomWheel());
  
}

void colorFade() {
  
  if (first_run) {
    if (DEBUG)Serial.println(F("Beginning Effect colorFade..."));
    first_run=0;
  }
  
  uint32_t new_color,past_color;

  float transition_ratio = (float(iter) / float(itermax));
  short transition_percentage = transition_ratio * 100;

  // Update our color and stash past colors
  if (iter == 0) {
    updateSecondary(primary);
    updatePrimary(RandomWheel());    
  }

  new_color = color(primary,transition_percentage);
  past_color = color(secondary,100-transition_percentage);

  new_color = combine(new_color,past_color);

  for (int i=0; i < panelsTotal; i++) {  
    q(i, new_color);
  }
}


void colorCycleFade() {
  if (first_run) {
    if (DEBUG)Serial.println(F("Beginning Effect colorCycleFade..."));
    first_run=0;

    for (int i=panelsTotal; i>=0; i--) {
        colors_past[i] = colors[i-1];
    }      
    // Add our new random color at the beginning.
    colors_past[0] = RandomWheel();    
  }

  uint32_t new_color,past_color;
  
  float transition_ratio = (float(iter) / float(itermax));
  short transition_percentage = transition_ratio * 100;

  // Serial.print("Trans perc:");
  // Serial.println(transition_percentage);
  

  for (int i=panelsTotal-1; i>=0; i--) {

      new_color = colors_past[i];
      
    
      new_color = color(new_color,transition_percentage);
      past_color = color(colors_past[i+1],100-transition_percentage);

      new_color = combine(new_color,past_color); 
        

      
      q(i,new_color);    

  }

  if (iter == itermax) {              
    for (int i=panelsTotal-1; i>=0; i--) {
        colors_past[i] = colors_past[i-1];
    }      
    // Add our new random color at the beginning.
    colors_past[0] = RandomWheel();    
  }  

}


void sparkle(){
  
  if (first_run) {
    if (DEBUG) {Serial.println(F("Beginning Effect sparkle..."));first_run=0;}    
    
    for(int i=0;i<pixelsTotal;i++)  {      
      q(i,0);
      active[i] = 0;
    }
    active_count= 0;    
  } 
  // density = 10; /// unhardcode 
  // Serial.println(density);

  // if (iter == 0) updatePrimary();
  if (iter == 0 && interval == 0) updatePrimary();

  // Pop In
  if (iter % 12 == 0) {    /// unhardcode magic numbers
      
      if (active_count < panelsTotal) {
        uint16_t new_pixel;
        do {        
          new_pixel = R(0,panelsTotal);
        } while (active[new_pixel]);
          // updatePrimary();
          colors_past[new_pixel] = primary;
          active[new_pixel] = 200;
          active_count++;
      }
  }

  // Manage Fades
  for(int i=0; i < panelsTotal; i++ ) {    

    if (active[i] > 0) {      
      uint32_t pixel_color = 0;

      // if (i==2) {        
      //   Serial.print(i);
      //   Serial.print(": ");
      //   Serial.print(active[i]);
      //   Serial.print(", ");
      //   Serial.print(200-active[i]);
      //   Serial.print(", ");
      //   Serial.println(colors[i],HEX);
      // }

      if (active[i] > 100) {       
        pixel_color = color(colors_past[i],200-active[i]);        
      }

      if (active[i] <= 100) {
        pixel_color = color(colors_past[i],active[i]);        
      }      

      q(i,pixel_color);
      active[i] -= 3;

      if (active[i] < 3) {
          active_count--;
          active[i] = 0;
          q(i,0);
      }
    }    
  }
}

uint8_t decayList[pixelsTotal];
void pulseDecay(){
  
  if (first_run) {
    if (DEBUG) {Serial.println(F("Beginning Effect pulseDecay..."));first_run=0;}    

    itermax = 50;
    intervalCount = 2;
    iter = 0;  
  } 
  
  // if (interval == 0) updatePrimary();
  if (iter == 0 && interval == 0) {
  
    updatePrimary();
    active_count = 0;
    for(int i=0;i<pixelsTotal;i++)  {      
      q(i,0);
      active[i] = 0;
    }

    for (int a=0; a<panelsTotal; a++) {
      decayList[a] = a;
    }

    for (int a=0; a<=panelsTotal; a++) {
      
      uint8_t r = random(a,panelsTotal);
      uint8_t temp = decayList[a];
      decayList[a] = decayList[r];
      decayList[r] = temp;

      // Serial.print(decayList[a]);
      // Serial.print(",");
    }
    // Serial.print("\n");
  }

  // Serial.print(F("Interval is..."));
  // Serial.println(interval);

  // fade in
  if (interval == 0) {
    for (int i=0; i < panelsTotal; i++) {    
      q(i,color(primary,iter));
    }    

    // if (i==2) {        
          // Serial.print(iter);
          // Serial.print(": ");
    //       Serial.print(active[i]);
    //       Serial.print(", ");
    //       Serial.print(200-active[i]);
          // Serial.print(", ");
          // Serial.println(colors[2],HEX);
    //     }
  }

  if (interval > 0) {
    
    if (iter % 2 == 0) {    /// unhardcode magic numbers

      if (active_count <= panelsTotal) {
      
        // uint16_t new_pixel,safeword=0;
        // do {        
        //   new_pixel = R(0,panelsTotal+2);          
        //   // Serial.print(new_pixel);
        //   // Serial.print(", ");
        // } while (active[new_pixel] && safeword++ < 20);
        
        active[decayList[active_count]] = itermax;
        Serial.print("Fading out pixel ");
        Serial.println(decayList[active_count]);
        active_count++;
      }
    }
  
    // Manage Fades
    for(int i=0; i < panelsTotal; i++ ) {    

      if (active[i] > 1) {      
        uint32_t pixel_color = 0;

        if (active[i] <= 100) {
          pixel_color = color(primary,active[i]);        
        }      

        q(i,pixel_color);
        active[i] -= 5;

        if (active[i] < 5) {
          // active_count--;
          active[i] = 1;
          q(i,0);
        }
      }    
    }
  }

  // wrap it up
  if(interval >= 2 && active_count >= panelsTotal){      
      iter=itermax;      
  }
}

void candleFlame() {
  if (DEBUG && first_run) {Serial.println(F("Beginning effect: candleFlame"));first_run=0;}    

  int intensity = random(210, 255);
  
  for (int i=0; i < panelsTotal; i++) {
    
    q(i,color(intensity,(intensity*22)/100,0));

  }
}

void pulse() {
  if (DEBUG && first_run) {Serial.println(F("Beginning effect: pulse"));first_run=0;direction=1;}    

  uint16_t k;
  itermax = 60;

  if (direction == 1) {
    k = iter;
    if (k >= itermax) {direction = 0; }
  }else{
    k = itermax-iter;
    if (k <= 0) { 
      direction = 1; 
      updatePrimary(); 
    }
    
  }  

  for (int i=0; i < panelsTotal; i++) {    
    q(i,color(primary,k));
  }

}


/* -- LOAD UP OUR EFFECTS -- */
void (*menu[])() = {fadeOut,flavorFill,rainbow,rainbowCycle,colorCycle,pulseDecay,sparkle,colorFade,candleFlame,pulse};

uint8_t menu_count = 15; /// fix this to adjust the number of effects.

// #setup
void setup()
{
  Serial.begin(9600);
  randomSeed(analogRead(0));

  updatePrimary(color(159,0,255));
  secondary = color(255,255,255);
  tertiary = color(255,255,255);

  strip->begin();
  strip->show();

  mode = strand;

  frequency = 260;
  // intervalCount = 1;
  interval = 0;
  iter = 0;
  panelsTotal = pixelsTotal;
  itermax = pixelsTotal;
  
  first_run = 1; // This is the first run flag.

  // Randomize Selector
  // selector = R(1,5);
  
  // Default selector
  future_selector = 1;

  DEBUG = 1;
  verbose = 0;

  if (DEBUG) statusUpdate();  

  // traceStrip();
}


void loop() {

  now = millis();   // This moment is beautiful.
  
  if (now >= then+loopDelay) {

    then = now;

    if (selector != future_selector) churn();

    pour();       // Send the 1011 and let the people have some fun.

  }

  //  Listeners

  byte y = 10;
  while(Serial.available() > 0 && (y-- > 0)) { // Listen on the serial connection.
    char x = Serial.read();
    interceptSerial(x);
  }

  interceptTouch(); // Listen for touch

} 

void interceptTouch() {

 if (!touching) {
   if (readCapacitivePin(TOUCHPIN) < 3) {      
     return; 
   } else {
     // First Touch!
     if (DEBUG) Serial.println(F("First Touch!"));
     touching = true;
     firstTouch = now;
   }
 }  

 // How long have we been touching?
 touchDuration = now - firstTouch;

 // We've stopped touching now so...
 if (readCapacitivePin(TOUCHPIN) < 2) {
   // Are we overtime?
   if (overtimeTouch) {
     if (DEBUG) Serial.println(F("Ending overtime touch."));
     touching = false; overtimeTouch = false; return;
   }    
   
   // If it was not an accidental touch...
   if (touchDuration > 50) {
     
     // Debounce
     if (now - lastTouchAction > 300) {
       
       // Do touch action
       // if (autopilot == 0) {
         // toggleAutoPilot();
       // }
           
       // Change mode.
       future_selector = selector+1; /// messy

       if (DEBUG) Serial.print(F("User activated mode change."));

       lastTouchAction = now;
     } else {
       if (DEBUG) Serial.println(F("Debounce!"));
     }
     
     touching = false;
     
     return;
   }  

   // Accidental touch, take no action.
   if (DEBUG) Serial.println(readCapacitivePin(TOUCHPIN));
   if (DEBUG) Serial.println(F("Short Touch - No Action"));
   touching = false;
   return;
 
 // Or we haven't so check some other stuff...
 } else {

   // Like if we are overtime on our touch
   if (touchDuration >= 1000) { 
     
     if (!overtimeTouch) {
       // Overtime triggered.
       if (DEBUG) Serial.println(F("Overtime Triggered"));
       
       // Turn autopilot off.
       if (!autoPilot) {
         toggleAutoPilot();        

         // Notify that we are turning it on.
         flash();
         delay(100);
         flash();          
         
       } else {          
         // Turn the lamp off.
         future_selector = 0;
         toggleAutoPilot();
       }

       overtimeTouch = true;
     }   
   }
 }
}

// Just flash black for a short bit.
void flash() {
  for(int i=0; i<pixelsTotal; i++){
    strip->setPixelColor(i,color(0,0,0));
  }
  strip->show();
  delay(40);
  for(int i=0; i<pixelsTotal; i++){
    strip->setPixelColor(i,colors[i]);
  }
  strip->show();
}

void rest() {
  // never happens.
}

void updateMode() {
  if (future_mode != mode) {
    
    switch (future_mode) {
        case strand:
          panelsTotal = pixelsTotal;

          break;
        case panel:
          panelsTotal = pixelsTotal;

          break;
        default:
        break;
          
    }
    mode = future_mode;
  }
}

void updateSelector() {

    if (future_selector > menu_count-1) {
      if (DEBUG) Serial.println(F("Resetting selector because of overbounds...")); 
      if (autoPilot) {
        future_selector = 1;
      } else {
        future_selector = 0;
      }
      // selector = 0;
    } else {
      
      // Ready to randomize      
      if (randomize) { 
          selector = R(0,menu_count-1); 
        } else {
          selector = future_selector;
          // selector++;
          if (DEBUG) {Serial.print(F("Changing selector to: "));Serial.println(selector);}
      }
    }   

    effect_start_time = now;
}

void churn() {
  // this is where things change.
  // Update based on interval and phase.
      
    first_run = 1;
    interval = 0;
    iter = 0;
    polkadots = 0;
    // mode = strand;

    statusUpdate();

    ///
    // mode = panel;
    // panelsTotal = panelsCount;
    itermax = panelsTotal;

    // Update selector
    updateSelector();

    // selector = 4; /// hardcode

    switch (selector) {
      case 0:
        // fadeout...
        effect_id = 1;
        intervalCount = 0;
        effectMS = 1;
        itermax = 200;
        updatePrimary(color(0,0,0));
        // updatePrimary(RandomWheel());
        will_transition = 1;
        break;      
      case 1:
        will_transition = 1;
        // flavorFill() 
        effect_id = 8;
        intervalCount = 20;
        effectMS = 50;
        itermax = panelsTotal;            
        // polkadots = 1;
        updatePrimary(color(100,0,255)); // purple color
        // updatePrimary(color(255,120,120)); // peach color
        // updatePrimary(color(0,255,1)); // hop color
        // updatePrimary(color(255,120,120)); // peach color
        // updatePrimary(color(255,60,0)); // wheat color
        // updatePrimary(color(255,45,0)); // flame color
        // updatePrimary(color(1,90,255)); // water color
        // updatePrimary(color(250,50,1)); // gold color
        // updatePrimary(color(255,230,230)); // white color
        // updatePrimary(color(255,0,0)); // red color
        // updatePrimary(color(255,255,0)); // yellow color        
        // updatePrimary(color(0,0,255)); // blue color        
        break;
      // case 2:
      //   will_transition = 1;
      //   // flavorFill()
      //   effect_id = 1;
      //   intervalCount = 20;
      //   itermax = panelsTotal;
      //   // updatePrimary(color(1,0,255));
      //   // updatePrimary(color(0,255,1)); // hop color
      //   updatePrimary(color(255,90,0)); // wheat color
      //   break;    
      // case 3:
      //   will_transition = 1;
      //   // flavorFill()
      //   effect_id = 1;
      //   intervalCount = 20;
      //   itermax = panelsTotal;
      //   updatePrimary();
      //   // updatePrimary(color(0,255,1)); // hop color
      //   // updatePrimary(color(255,90,0)); // wheat color
      //   break;      
      case 2:
        will_transition = 1;
        // flavorFill()
        effect_id = 1;
        intervalCount = 20;
        itermax = panelsTotal;
      
        polkadots = 0;
        updatePrimary(color(100,0,255)); // purple color
        // updatePrimary(color(255,120,120)); // peach color
        // updatePrimary(color(255,100,0)); // yellow color        
        // updatePrimary(color(255,90,0)); // wheat color
        // updatePrimary(color(0,255,1)); // hop color

        // updatePrimary(color(255,230,230)); // white color
        // updateSecondary(color(255,0,0)); // red color
        // updateSecondary(color(100,0,255)); // purple color        
        // updateSecondary(color(250,50,0)); // orange color        
        // updateSecondary(color(1,120,255)); // water color
        // updateSecondary(color(1,1,255)); // blue color
        
        // updatePrimary();
        // updateSecondary();
        break;        
      case 3:
        // rainbow()
        effect_id = 2;
        effectMS = 400; /// this is set low for panels...
        intervalCount = 6;
        itermax = 255;
        will_transition = 1;
        // iter=100; // start at a different color than green /// Doesnt seem to work
        break;
      case 4:
        // rainbowCycle()
        effect_id = 3;
        effectMS = 50;
        intervalCount = 20;
        itermax = 255;        
        will_transition = 1;
        break;  
      case 7:
        // colorCycle()
        effect_id = 4;
        will_transition = 0;
        intervalCount = 60;
        effectMS = 300;
        itermax = panelsTotal;
        break;            
      case 6:
        // sparkle()
        effect_id = 6; //effect_id = 6; sparkle
        will_transition = 1;
        effectMS = 1;
        intervalCount = 10;
        itermax = 100;
        density = R(1,panelsTotal-1);
        decay = 20;
        updatePrimary(); // random color
        // updatePrimary(color(0,255,1)); // wheat color
        // updatePrimary(color(0,255,1)); // hop color
        //   updateSecondary(color(0,0,0));        
        break;                
      case 5:
        // colorFade()
        effect_id = 7;
        will_transition = 1;
        effectMS = 40;
        intervalCount = 20;
        itermax = 100;    
        break;
      // case 10:
      //   // colorCycleFade()
      //   effect_id = 5;
      //   iter = 100; // Start on a different color than green
      //   will_transition = 0;
      //   intervalCount = 20;
      //   effectMS = 1;
      //   itermax = 255;      
      //   break;          
           
      default:
        if (DEBUG) Serial.println(F("Hitting Default Selector Switch..."));      
        
        if (autoPilot) {
         future_selector = 1;
        } else {
          future_selector = 0;
        }
      
        break;
    }

    if (verbose) statusUpdate();    

    if (will_transition) {
      transitioning = 1;
      transition_steps = transition_time / loopDelay;

      if (DEBUG) {Serial.print("Transition Steps: ");Serial.println(transition_steps);}
      
      current_transition_step = 0;
      
      for(int i=0; i<pixelsTotal; i++){
        colors_past[i] = colors[i];
      }

    }

    effectMS_counter = 0;
    iter = 0;

}

void pour() { 

  // this is where we move forward.
  // update mode, interval, phase etc.
  
  if (autoPilot && selector != 0) {
    if (now-effect_start_time > effect_duration) {
      future_selector++;      
    }
  } 

  // Call our selected effect.
  effectMS_counter+=loopDelay;
  
  if (effectMS_counter >= effectMS) {

    if (iter < itermax) {
      iter += 1;
    } else {
      iter = 0;

      if (interval >= intervalCount) {
        interval = 0;        
      } else {
        interval++;
      }

      if (DEBUG) { Serial.print(F("Interval updated to "));Serial.println(interval);  }
    }


    (*menu[effect_id])();
    effectMS_counter = 0;   
  }
  

  // this is where we lay the goods.
  if (verbose && DEBUG) { Serial.println(F("Setting the strand ...")); }

  uint32_t new_color,past_color;
  float transition_ratio;
  short transition_percentage;

  if (transitioning) {
    current_transition_step++;

    if (current_transition_step < transition_steps) {          
      transition_ratio = (float(current_transition_step) / float(transition_steps));
      transition_percentage = transition_ratio * 100;
      // Serial.print("Trans step:");
      // Serial.println(current_transition_step);
      // Serial.print("Trans ratio:");
      // Serial.println(transition_ratio);
      // Serial.print("Trans perc:");
      // Serial.println(transition_percentage);
    } else {
      transitioning = 0;
    }  
  }

  for(int i=0; i<pixelsTotal; i++){

      new_color = colors[i];
      
      if (transitioning) {
          new_color = color(new_color,transition_percentage);
          past_color = color(colors_past[i],100-transition_percentage);
          new_color = combine(new_color,past_color); 
        
      }

      if (brightness!=100) {
        new_color = color(new_color,brightness);    
      }

      strip->setPixelColor(i,new_color);    
  }

  // this is where the magic happens.
  if (verbose && DEBUG) { Serial.println(F("Pouring deliciousness ...")); }
  strip->show();
}


// virtual void q() =0;
void q(uint16_t pos, uint32_t c) {    
  
  int p;

  // In here we need to be able to check the orientation, mode, and then check to see if we are compositing two effects.
  // flag for overlay / transition.  
    colors[pos] = c;  
  
}

// pixel runner for tracing.
void traceStrip() {

  for (int i = 0; i<pixelsTotal; i++) {
    strip->setPixelColor(i,0xFFFFFF);
    strip->show();
    delay(100);
    strip->setPixelColor(i,0);
    strip->show();
  }
}

// Serial Functions
void interceptSerial(char x) {

    //Flags, set read mode., begin
  
  if        ( x == '!' )    {   readMode  = 1;    }         //Set Selector
  else if   ( x == '@' )    {   readMode  = 2;    }         //Set Frequency
  else if   ( x == '#' )    {   readMode  = 3;    }         //Set 
  else if   ( x == '+' )    {   readMode  = 4;    }         //Shift Register IDs, separated by comma (no whitespace)
  else if   ( x == '-' )    {   readMode  = 5;    }         //Shift Register IDs, separated by comma (no whitespace)
  else if   ( x == '~' )    {   readMode  = 6;    }         //System Mode 
  else if   ( x == '*' )    {   readMode  = 7;    }         //System Mode   
  else if   ( x == '?' )    {   readMode  = 8;    }         //Status Update
  // else if    ( x == '/' )    {   getFiles();       }   
  // else if    ( x == '?' )    {   statusUpdate();   }     
  //Add custom flags here...
  
  //Finish up
  else if   (x == '.')    {   //...

    //This will update the global variables accordingly.
    switch(readMode){
      case 1:      sSetSelector();        break;
      case 2:      sSetFrequency();       break;
      case 3:      sSetBrightness();      break;
      case 4:      sSetMode();            break;
      // case 5:      setValveOff();      break;
      // case 6:      setMode();          break;      
      case 7:      toggleDebug();         break;
      case 8:      statusUpdate();        break;
      default:                            break;  
    }
    
      // lastSerialCMD = now;         //Used for switching to autoPilot
      readMode = 0;                   //We're done reading. (until another.)
      bufferIndex = 0;
      resetMessageBuffer();         
  }
  else { messageBuffer[bufferIndex++] = x; }        //Magic.

}

void sSetSelector() {
  uint8_t s = atoi(messageBuffer);
  setSelector(s);
}
void setSelector(uint8_t s) {
  // if (messageBuffer) /// do we need checking here? it's all going to change eventually.  

  future_selector = s;

  if (DEBUG) Serial.print(F("User updated selector: "));Serial.println(future_selector); /// Should this be future_selector?
  
}

void sSetFrequency() {
  uint16_t s = atoi(messageBuffer);
  setFrequency(s);
  if (DEBUG) Serial.print(F("User updated effectMS: "));Serial.println(effectMS);
}
void setFrequency(uint16_t s) {
  /// Add some bounds here.
  effectMS = s;
  
}
void sSetBrightness() {
  uint16_t b = atoi(messageBuffer);  
  setBrightness(b);
  if (DEBUG) {Serial.print(F("User updated brightness to: "));Serial.println(b);}
}
void sSetMode() {
  uint8_t s = atoi(messageBuffer);
  setSelector(s);
}
void setMode(uint8_t s) {
  // if (messageBuffer) /// do we need checking here? it's all going to change eventually.  

  future_mode = s;

  if (DEBUG) Serial.print(F("User updated mode: "));Serial.println(future_mode); /// Should this be future_selector?
  
}
void toggleAutoPilot() {
  if (autoPilot) {
    autoPilot = false;
    if (DEBUG) {Serial.print(F("Autopilot turning off. "));}
  } else {
    autoPilot = true;
    if (DEBUG) {Serial.print(F("Autopilot turning on. "));}
  }
}


void resetMessageBuffer(){
  memset( messageBuffer, '\0', sizeof(messageBuffer) );   
}

void statusUpdate() {
  // Serial.print(F("Is Dave there?"));
  Serial.println(F("<=== Creamery Status Update ===>")); 
  Serial.print(F("Selector: "));
  Serial.println(selector); 
  Serial.print(F("effect_id: "));
  Serial.println(effect_id); 
  Serial.print(F("Interval: "));
  Serial.println(interval); 
  Serial.print(F("Primary Color: "));
  Serial.println(primary,HEX); 
  Serial.print(F("effectMS: "));
  Serial.println(effectMS);
  
  Serial.println("-------");
  Serial.print(F("panelsTotal: "));
  Serial.println(panelsTotal);  

  Serial.print(F("Phase: "));
  Serial.println(phase);

  Serial.print(F("Free Ram: "));
  Serial.println(freeRam());

  Serial.print(F("DEBUG: "));
  Serial.println(DEBUG);
  Serial.print(F("VERBOSE: "));
  Serial.println(verbose);
  
  Serial.println(F("<===========>"));
  Serial.println(F(""));
  /// checking how many items are in the menu / effect list. 
  // for (int j=0;j<menu_count;j++) {
  //  Serial.println(j);  
  // }

  resetMessageBuffer();
}
void setBrightness(uint8_t b) {  
  // add some bounds checks here
  if (b <= 100 && b>=0) {    
    brightness = b;
  }
}

void toggleDebug() {if (DEBUG){DEBUG = 0;} else {DEBUG=1;}Serial.print(F("Toggling debug:"));Serial.println(DEBUG);}
void toggleVerbose() {if (verbose){verbose = 0;} else {verbose=1;}}

// http://playground.arduino.cc/Code/CapacitiveSensor
 uint8_t readCapacitivePin(int pinToMeasure) {

  // Variables used to translate from Arduino to AVR pin naming

  volatile uint8_t* port;
  volatile uint8_t* ddr;
  volatile uint8_t* pin;

  // Here we translate the input pin number from
  //  Arduino pin number to the AVR PORT, PIN, DDR,
  //  and which bit of those registers we care about.

  byte bitmask;
  port = portOutputRegister(digitalPinToPort(pinToMeasure));
  ddr = portModeRegister(digitalPinToPort(pinToMeasure));
  bitmask = digitalPinToBitMask(pinToMeasure);
  pin = portInputRegister(digitalPinToPort(pinToMeasure));

  // Discharge the pin first by setting it low and output
  *port &= ~(bitmask);
  *ddr  |= bitmask;
  delay(1);
  uint8_t SREG_old = SREG; //back up the AVR Status Register

  // Prevent the timer IRQ from disturbing our measurement
  noInterrupts();

  // Make the pin an input with the internal pull-up on
  *ddr &= ~(bitmask);
  *port |= bitmask;

  // Now see how long the pin to get pulled up. This manual unrolling of the loop
  // decreases the number of hardware cycles between each read of the pin,
  // thus increasing sensitivity.

  uint8_t cycles = 17;

  if (*pin & bitmask) { cycles =  0;}
  else if (*pin & bitmask) { cycles =  1;}
  else if (*pin & bitmask) { cycles =  2;}
  else if (*pin & bitmask) { cycles =  3;}
  else if (*pin & bitmask) { cycles =  4;}
  else if (*pin & bitmask) { cycles =  5;}
  else if (*pin & bitmask) { cycles =  6;}
  else if (*pin & bitmask) { cycles =  7;}
  else if (*pin & bitmask) { cycles =  8;}
  else if (*pin & bitmask) { cycles =  9;}
  else if (*pin & bitmask) { cycles = 10;}
  else if (*pin & bitmask) { cycles = 11;}
  else if (*pin & bitmask) { cycles = 12;}
  else if (*pin & bitmask) { cycles = 13;}
  else if (*pin & bitmask) { cycles = 14;}
  else if (*pin & bitmask) { cycles = 15;}
  else if (*pin & bitmask) { cycles = 16;}

  // End of timing-critical section; turn interrupts back on if they were on before, or leave them off if they were off before
  SREG = SREG_old;

  // Discharge the pin again by setting it low and output
  //  It's important to leave the pins low if you want to 
  //  be able to touch more than 1 sensor at a time - if
  //  the sensor is left pulled high, when you touch
  //  two sensors, your body will transfer the charge between
  //  sensors.

  *port &= ~(bitmask);
  *ddr  |= bitmask;

  return cycles;

}

/* Helper functions */

void updatePrimary(uint32_t c) {primary = c;}
void updateSecondary(uint32_t c) {secondary = c;}
void updateTertiary(uint32_t c) {tertiary = c;}

void updatePrimary() {if (full_or_wheel) {primary = RandomWheel();} else {primary = RandomColor();}}
void updateSecondary() {if (full_or_wheel) {secondary = RandomWheel();} else {secondary = RandomColor();}}
void updateTertiary() {if (full_or_wheel) {tertiary = RandomWheel();} else {tertiary = RandomColor();}}


// Update a specific flavor.
void updateFlavor(uint8_t flavor, uint32_t c) {
  switch (flavor) {
    case 1:
      primary = c;
      // do something
      break;
    case 2:
      secondary = c;
      // do something
      break;
    case 3:
      tertiary = c;
      // do something
    break;
    default:
      primary = c;
      // do something
    break;       
  }
}
// Update all flavors randomly.
void updateAssorted() {
  primary = RandomWheel();
  secondary = RandomWheel();
  tertiary = RandomWheel();
}

//Shorter random function, static helper. 
uint8_t R(uint8_t from, uint8_t to){ return random(from, to); } 
// uint8_t R(uint8_t from, uint8_t to){ return TrueRandom.random(from, to); } 
uint32_t RandomColor(){ return color(R(0,255), R(0,255), R(0,255)); }
uint32_t RandomWheel() { return Wheel(R(0,255)%255); }

uint32_t rgba(byte r, byte g, byte b, int a) {
  
  int rr = (r*a)/100;
  int gg = (g*a)/100;
  int bb = (b*a)/100;

  return color(rr,gg,bb);
}

// Create a 24 bit color value from R,G,B
uint32_t color(byte r, byte g, byte b, int a) {
  
  int rr = (r*a)/100;
  int gg = (g*a)/100;
  int bb = (b*a)/100;

  return color(rr,gg,bb);
}

// Create a 24 bit color value from R,G,B
uint32_t color(byte r, byte g, byte b) {
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;  
  c |= b;
  
    return c;
}

uint32_t color(uint32_t c, int a) {
  
  uint8_t r = extractRed(c);
  uint8_t g = extractGreen(c);
  uint8_t b = extractBlue(c);
    
  return rgba(r,g,b,a);
}

uint32_t alpha(uint32_t c, int a) {
  
  uint8_t r = extractRed(c);
  uint8_t g = extractGreen(c);
  uint8_t b = extractBlue(c);
    
  return rgba(r,g,b,a);
}

uint32_t combine(uint32_t color1,uint32_t color2) {
  
  uint16_t new_r = extractRed(color1) + extractRed(color2);
  uint16_t new_g = extractGreen(color1) + extractGreen(color2);
  uint16_t new_b = extractBlue(color1) + extractBlue(color2);

  if (new_r > 255) new_r = 255;
  if (new_g > 255) new_g = 255;
  if (new_b > 255) new_b = 255;

  return color(new_r,new_g,new_b);  
}

// Helpers to extract RGB from 32bit color. (/// This could be MACRO...)
uint8_t extractRed(uint32_t c) { return (( c >> 16 ) & 0xFF); } 
uint8_t extractGreen(uint32_t c) { return ( (c >> 8) & 0xFF ); } 
uint8_t extractBlue(uint32_t c) { return ( c & 0xFF ); }

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
   return color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
// This is Wheel with alpha.
uint32_t Wheel(byte WheelPos, double alpha) {
  if (WheelPos < 85) {
   return rgba(WheelPos * 3, 255 - WheelPos * 3, 0,alpha);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return rgba(255 - WheelPos * 3, 0, WheelPos * 3,alpha);
  } else {
   WheelPos -= 170; 
   return rgba(0, WheelPos * 3, 255 - WheelPos * 3,alpha);
  }
}

// Free Ram Output via Adafruit ///Move this into a testing utilities library.
int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
