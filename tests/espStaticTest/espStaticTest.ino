// Secular Geometry Lantern Code
// By Jordan Layman 2016
// www.seculargeometry.com
// Idea Fab Labs, Chico // Santa Cruz

#include "Driver.h"
#define DATAPIN   13 // Data pin for serial communication to shift registers
#define CLOCKPIN  14 // Clock pin for serial communication to shift registers
#define TOUCHPIN  4 // Touch sensitive switch

const char* ssid= "LEDpaint";
const char* pass= "betafish";
const char* host= "sg-lantern";

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WebSocketsServer webSocket = WebSocketsServer(81);

// holds the current upload
File fsUploadFile;

// Grid DEF & Vars
#define panelsX 1
#define panelsY 1
#define pixelsX 5
#define pixelsY 3
#define pixelsTotal 20
#define panelsCount 20
boolean autoPilot = true;
long effect_duration = 100000;

// Instantiate Controller. Num Pix Automatically Generated.
Adafruit_WS2801* strip = new Adafruit_WS2801(pixelsTotal, DATAPIN, CLOCKPIN);
// Adafruit_NeoPixel* strip = new Adafruit_NeoPixel(pixelsTotal, DATAPIN, NEO_GRB + NEO_KHZ800);

// Buckyball
byte panels[panelsCount][10] = { {0,1,2,3,4,0,0,0,0,0}, {5,6,7,8,9,10,11,12,13,14}, {15,16,17,18,19,20,21,22,23,24}, 
                      {25,26,27,28,29,29,29,29,29,29}, {30,30,30,30,30,30,30,30,30,30} };

// Neopixel Matrix
// byte panels[8][pixelsX] = { {0,8,16,24,32},{1,9,17,25,33},{2,10,18,26,34},{3,11,19,27,35},
                     // {4,12,20,28,36},{5,13,21,29,37},
                     // {6,14,22,30,38},{7,15,23,31,39}};

// For the next buckyball
// byte panels[6][10] = { {0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0}, 
                      // {0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0}, {30,30,30,30,30,30,30,30,30,30} };

#define loopDelay 40 // framerate. 25ms = 40fps


uint8_t brightness = 100;

// Alignment,Direction,Pattern Enumeration
enum {asc,desc,   horizontal,vertical,  strand,panel, mirrored, ring};

uint16_t orientation = vertical;
uint16_t direction = desc;
uint16_t panelsTotal = pixelsTotal; // Default

// nodeTimeStamps becomes colorPallette
uint32_t colors[pixelsTotal]; /// We may be removing this...
uint32_t colors_past[pixelsTotal];

uint16_t active[pixelsTotal]; // Right now this is just for active sparkle pixels.
uint16_t active_count = 0;

// Serial Vars
boolean DEBUG = 1; // Flag for debugging.
boolean verbose = 0; // Flag for verbose debugging.

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


// Websocket Event Handler WSEH
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
    } 
        break;
    case WStype_TEXT:
        Serial.printf("[%u] get Text: %s\n", num, payload);

        if(payload[0] == '0') {flash();}
        if(payload[0] == '1') {webSocket.sendTXT(num,"1");}
        if(payload[0] == '!') {
          
          if(payload[1] == '>'){
            forwardSelector();
            Serial.println("forward selector");
          } else if(payload[1] == '<'){
            backSelector();
            Serial.println("backward selector");
          } else {
            /// not optimized... char to int is what we need
            uint8_t s = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
            setSelector(s);    
          }
          
        }
        if(payload[0] == '@') {
            uint8_t f = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
            setFrequency(f);
            webSocket.sendTXT(num, "Frequency set to "+String(effectMS));
        }
        if(payload[0] == '#') {
            uint8_t b = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
            setBrightness(b);            
            webSocket.sendTXT(num, "Brightness set to "+String(brightness));
        }
        if(payload[0] == 'P') {
            uint32_t p = (uint32_t) strtol((const char *) &payload[1], NULL, 16);
            Serial.println(p);

            updatePrimary(p);
            webSocket.sendTXT(num, "Primary updated");
        }
        
        if(payload[0] == 'G') {     
          /// Probably replace this with a switch statement. 
          if(payload[1]=='!'){
            // send message to client
            Serial.printf("Get selector request...Replying: %d\n",selector);
            webSocket.sendTXT(num, String(selector));    
          }
        }
        break;
  }

}

void churn() {
  // this is where things change.
  // Update based on interval and phase.
      
    first_run = 1;
    interval = 0;
    iter = 0;
    polkadots = 0;
    // mode = strand;

    ///
    // mode = panel;
    // panelsTotal = panelsCount;
    itermax = panelsTotal;

    // Update selector
    updateSelector();

    // selector = 0; /// hardcode

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
        effect_id = 1;
        intervalCount = 20;
        itermax = panelsTotal;    
        // updatePrimary(color(0,100,255)); // purple color
        // updatePrimary(color(100,0,255)); // purple color
        // updatePrimary(color(0,255,1)); // hop color
        // updatePrimary(color(255,90,0)); // wheat color
        // updatePrimary(color(1,90,255)); // water color
        updatePrimary(color(255,0,0)); // red color
        // updatePrimary(color(255,255,0)); // yellow color
        break;    
      case 2:
        will_transition = 1;
        // flavorFill() polkadots
        effect_id = 1;
        intervalCount = 20;
        itermax = panelsTotal;
      
        polkadots = 1;
        updatePrimary();
        updateSecondary();                
        break;        
      case 3:
        // rainbow()
        effect_id = 2;
        effectMS = 400; /// this is set low for panels...
        intervalCount = 6;
        itermax = 255;
        will_transition = 1;        
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
        effect_id = 6;
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
      case 8:
        // colorCycleFade()
        effect_id = 8;
        // iter = 100; // Start on a different color than green
        will_transition = 1;
        intervalCount = 20;
        effectMS = 60;
        itermax = 255;      
        break;          
           
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

  // setup new array and add the new color.
  uint32_t nextc[panelsTotal];
  nextc[0] = RandomWheel();
  
  // Move the chain along.
  for (int i=0; i<panelsTotal-1; i++) {
    /// we will need to account for "panels".
    nextc[i+1] = colors[i];
  }
  
  // load up the new array
  for (int i=0; i<panelsTotal; i++) {              
      q(i,nextc[i]); 
  }
}

// Same as colorCycle but fading.
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

  if (iter == 0 && interval == 0) updatePrimary();

  // Pop In
  if (iter % 3 == 0) {    /// unhardcode magic numbers
      
      if (active_count < density) {
        uint16_t new_pixel;
        do {        
          new_pixel = R(0,panelsTotal);                    
        } while (active[new_pixel]);
          q(new_pixel,primary);        
          active[new_pixel] = 1;
          active_count++;
      }    
  }

  // Fade Out
  for(int i=0; i < panelsTotal; i++ ) {    
    if (active[i]) {
      uint32_t pixel_color = colors[i];
      pixel_color = color(pixel_color,95);
      q(i,pixel_color);
      if (!pixel_color) {
        active[i] = 0;
        active_count--;
      }
    }

  }

}

void candleFlame() {
  if (DEBUG && first_run) {Serial.println(F("Beginning effect: candleFlame"));first_run=0;}    

  int intensity = random(210, 255);
  
  for (int i=0; i < panelsTotal; i++) {
    
    q(i,color(intensity,(intensity*30)/100,0));

  }
}

/* -- LOAD UP OUR EFFECTS -- */
// void (*menu[])() = {fadeOut,flavorFill,rainbow,rainbowCycle,colorCycle,colorCycleFade,sparkle,colorFade,candleFlame};

uint8_t menu_count = 9; /// fix this to adjust the number of effects.

// #setup
void setup()
{
  Serial.begin(115200);
  Serial.print("\n");
  Serial.setDebugOutput(true);

  SPIFFS.begin();
  if (DEBUG) {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
  
  // Load config.
  loadConfig();
  if (!loadConfig()) {
    Serial.printf("Config failed to load.\n");
    // setup defaults


  }

  //WIFI INIT
  Serial.printf("Connecting to %s\n", ssid);
  if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.begin(ssid, pass);
  }
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); /// 
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  MDNS.begin(host);
  Serial.print("Open http://");
  Serial.print(host);
  Serial.println(".local/edit to see the file browser");
  

  httpUpdater.setup(&server);


  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  //SERVER INIT
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, [](){
    if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, [](){
    String json = "{";
    json += "\"heap\":"+String(ESP.getFreeHeap());
    json += ", \"analog\":"+String(analogRead(A0));
    json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  server.begin();
  Serial.println("HTTP server started");

  // -- Color

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

  if (DEBUG) statusUpdate();
  
  
  // pixel runner for tracing.
  
  // for (int i = 0; i<pixelsTotal; i++) {
  //   strip->setPixelColor(i,0xFFFFFF);
  //   strip->show();
  //   delay(100);
  //   strip->setPixelColor(i,0);
  //   strip->show();
  // }

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

  if (now >= then+30) { interceptTouch(); } // Listen for touch ///might not need this.
  
  server.handleClient();
  webSocket.loop();
} 

void interceptTouch() {

  if (!touching) {
    if (readCapacitivePin(TOUCHPIN) <= 1) {      
      return; 
    } else {
      // First Touch!
      //if (DEBUG) Serial.println(F("First Touch!"));
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
        forwardSelector();

        if (DEBUG) Serial.print(F("User activated mode change."));

        lastTouchAction = now;
      } else {
        if (DEBUG) Serial.println(F("Debounce!"));
      }
      
      touching = false;
      
      return;
    }  

    // Accidental touch, take no action.
    //if (DEBUG) Serial.println(F("Short Touch - No Action"));
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
          panelsTotal = panelsCount;

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
void forwardSelector(){
  future_selector = selector+1;
}
void backSelector(){
  future_selector = selector-1;
}


// This where the magic happens
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
        phase++;
        if (DEBUG) { Serial.println("\n");  }
        if (DEBUG && verbose) { Serial.printf("Phase updated to %d\n",phase);  }      
      } else {
        interval++;
      }

      if (DEBUG) { Serial.printf(".%d",interval);  }
      if (DEBUG && verbose) { Serial.printf("Interval updated to %d\n",interval);  }
    }

    (*menu[effect_id])();

    if (DEBUG && verbose) Serial.println("Passed pouring"); ///
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
          
    // Load up the stip.
    strip->setPixelColor(i,new_color);    

    if (DEBUG && verbose) { Serial.println(new_color,HEX); }

  }

  // this is where the magic happens.
  if (verbose && DEBUG) { Serial.println(F("Pouring deliciousness ...")); }
  strip->show();
}


// Queue up a color 'c' into the color array at position 'pos'.
void q(uint16_t pos, uint32_t c) {    
  
  int p;

  // In here we need to be able to check the orientation, mode, and then check to see if we are compositing two effects.
  // flag for overlay / transition.

  if (mode == strand) {
    /// Still need orientation and direction statements here
    colors[pos] = c;
  } else 
  if (mode == mirrored) { /// Mirror mode should be seperate and augment any of the other modes. ie any mode could be mirrod.
    // left/right mirror mode.
    // p = mirror(pos);
  } else 
  if (mode == panel) {
    
    
    if (pos >= panelsTotal) {
      Serial.println("OUT OF BOUNDS");    
      }
    for (int i=0;i<panelsTotal;i++) {             
      colors[panels[pos][i]] = c;
    }
    // left/right mirror mode.
    // p = mirror(pos);
  

  // } else   
  // if  (mode == matrix) { // Make each panel do the same thing.
      
  //   // Serial.println(orientation);
  //   // Serial.print(F("--- "));Serial.println(pos);
  //   if (orientation == horizontal) {
  //     for (int i=0;i<pixelsTotal/panelsX;i++) {       
  //       p = pixelsY*pos+i;
        
  //       // p = pixelsY*pos+i;
  //       // strip->setPixelColor(p, c);
  //       // Serial.println(p);
  //     }
  //   } else if (orientation == vertical) {
  
  //     for (int i=0;i<pixelsTotal/pixelsY;i++) {
  //       /// I think these are for the Fan
  //       // p = (i%2) ? (panelsX*i)+(pixelsY-1)-(pos) : (i*panelsX)+(pos);
  //       // This one might be better....
  //       // p = (i%2) ? (pixelsY*i)+(pixelsY-1)-(pos) : (i*pixelsY)+(pos);
        
  //       // This panel mapping is for dodecahedron cells       
  //       colors[pos] = c;
      
  //       // Serial.println(p);
  //     }
  //   }
  
  }  

  // if (transitioning) {
    // colors_future[p] = c;
  // } else {
    // colors[p] = c;
  // }

  
  // strip->setPixelColor(p,c);
}

// Serial Functions
void interceptSerial(char x) {

    //Flags, set read mode., begin
  
  if        ( x == '!' )    {   readMode  = 1;    }         //Set Selector
  else if   ( x == '@' )    {   readMode  = 2;    }         //Set Frequency
  else if   ( x == '#' )    {   readMode  = 3;    }         //Set Brightness
  else if   ( x == '+' )    {   readMode  = 4;    }         //Set Mode
  else if   ( x == '-' )    {   readMode  = 5;    }         //
  else if   ( x == '~' )    {   readMode  = 6;    }         //Reload Config
  else if   ( x == '*' )    {   readMode  = 7;    }         //Toggle Debug
  else if   ( x == '?' )    {   readMode  = 8;    }         //Status Update
  // else if    ( x == '/' )    {   getFiles();       }       //    
  // else if    ( x == '?' )    {   statusUpdate();   }       //  
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
      case 6:      loadConfig();          break;      
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
  Serial.println(F("<=== Status Update ===>")); 
  Serial.print(F("Selector: "));
  Serial.println(selector); 
  Serial.print(F("Interval: "));
  Serial.println(interval); 
  Serial.print(F("Primary Color: "));
  Serial.println(primary,HEX); 
  Serial.print(F("effectMS: "));
  Serial.println(effectMS);
  Serial.print(F("Density: "));
  Serial.println(density);
  Serial.print(F("Decay: "));
  Serial.println(decay);
  Serial.print(F("Brightness: "));
  Serial.println(brightness);

  // wifi info
  Serial.printf("SSID %s",ssid);
  Serial.printf("HOST", host);

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


uint8_t readCapacitivePin(int pinToMeasure) {

  uint8_t total = 0;
  
  for(uint8_t i = 0; i < 6; i++) {

    pinMode(pinToMeasure, OUTPUT);

    digitalWrite(pinToMeasure, LOW);

    //delay(1);

    //noInterrupts();

    pinMode(pinToMeasure, INPUT_PULLUP);

    uint8_t cycles = 17;

    if (digitalRead(pinToMeasure)) { cycles =  0;}
    else if (digitalRead(pinToMeasure)) { cycles =  1;}
    else if (digitalRead(pinToMeasure)) { cycles =  2;}
    else if (digitalRead(pinToMeasure)) { cycles =  3;}
    else if (digitalRead(pinToMeasure)) { cycles =  4;}
    else if (digitalRead(pinToMeasure)) { cycles =  5;}
    else if (digitalRead(pinToMeasure)) { cycles =  6;}
    else if (digitalRead(pinToMeasure)) { cycles =  7;}
    else if (digitalRead(pinToMeasure)) { cycles =  8;}
    else if (digitalRead(pinToMeasure)) { cycles =  9;}
    else if (digitalRead(pinToMeasure)) { cycles = 10;}
    else if (digitalRead(pinToMeasure)) { cycles = 11;}
    else if (digitalRead(pinToMeasure)) { cycles = 12;}
    else if (digitalRead(pinToMeasure)) { cycles = 13;}
    else if (digitalRead(pinToMeasure)) { cycles = 14;}
    else if (digitalRead(pinToMeasure)) { cycles = 15;}
    else if (digitalRead(pinToMeasure)) { cycles = 16;}

    //interrupts();

    digitalWrite(pinToMeasure, LOW);

    pinMode(pinToMeasure, OUTPUT);

    if(cycles < 1 || cycles > 11) { // outside threshold - resample
      i--;
    } else {
       total = total + cycles;
    }
    //return cycles;
  }
  return((total/5));
}

// ==== Webserver Functions

//format bytes

String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload(){
  if(server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    if(fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) fsUploadFile.close();
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}

void handleFileDelete(){
  if(server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate(){
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if(!server.hasArg("dir")) {server.send(500, "text/plain", "BAD ARGS"); return;}
  
  String path = server.arg("dir");
  Serial.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}


bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r"); ///This might break if config doesn't exist...
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("\nFailed to parse config file");
    return false;
  }

  // Load vars
  ssid = json["ssid"];
  pass = json["pass"];
  // host = json["host"];

  autoPilot = (json["auto"] == "true") ? true : false;
   
  
  Serial.println("\nConfig Loaded.");
  return true;
}

// bool generateFile(String filename, String array) {}

bool saveConfig() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
  json["ssid"] = ssid;
  json["pass"] = pass;
  json["host"] = host;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("\nFailed to open config file for writing.");
    return false;
  }

  json.printTo(configFile);
  return true;
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
  int rr = r*a;
  int gg = g*a;
  int bb = b*a;

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
String freeRam () 
{ 
  return String(ESP.getFreeHeap());
}

