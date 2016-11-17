espConfigTest.ino
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>

boolean DEBUG = true;

void setup() {

	Serial.begin(115200);
  	Serial.println("Starting up...");

  	if(SPIFFS.begin()) {
      Serial.println("File system mounted");
   } else {
     Serial.println("File system failed");
   }

   loadConfig()
}

void loop() {

}

JsonObject* SGFileServer::loadConfig(String path) {
   
   char input[32];
   File configFile = SPIFFS.open(path, "r");
   if (!configFile) {
      if (DEBUG) Serial.println("Failed to open config file.");
      return false;
   }

   size_t size = configFile.size();
   if (size > 1024) {
      if (DEBUG) Serial.println("Config file size is too large");
      return false;
   }

   std::unique_ptr<char[]> buf(new char[size]);
   configFile.readBytes(buf.get(), size);

   StaticJsonBuffer<200> jsonBuffer;
   JsonObject& json = jsonBuffer.parseObject(buf.get());

   if (!json.success()) {
      if (DEBUG) Serial.println("Failed to parse config file");
      return false;
   }
   
   // strcpy(input, json["wifi_name"]);
   // wifi_name = String(input);
   // strcpy(input, json["wifi_ssid"]);
   // wifi_ssid = String(input);
   // strcpy(input, json["wifi_password"]);
   // wifi_password = String(input);

   return true;
}
