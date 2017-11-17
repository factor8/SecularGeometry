#include "SGFileServer.h"

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length){}

SGFileServer::~SGFileServer() {}

int SGFileServer::persist() {

	if (isSetup()) {
	webServer->handleClient();
	socketServer->loop();
	}	
	
}
void SGFileServer::init() {

	// Defaults
	// _status = WIFISTATUS_INITIALIZING;
	// _mode = 0;


	if(SPIFFS.begin()) {
		if (DEBUG) Serial.println("File system mounted");
		_mounted = true;
	} else {
		if (DEBUG) Serial.println("File system failed");
		_mounted = false;
	}

	if (DEBUG) {
		Dir dir = SPIFFS.openDir("/");
		while (dir.next()) {    
			String fileName = dir.fileName();
			size_t fileSize = dir.fileSize();
			if (DEBUG) Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
		}
		if (DEBUG) Serial.printf("\n");
	}

  /// Make this configurable.
  // _dnsName = "SG_" + String(ESP.getChipId()).substring(4);
  
  //SERVER INIT
  webServer.reset(new ESP8266WebServer(WEB_PORT));

  //list directory
  webServer->on("/list", HTTP_GET, std::bind(&SGFileServer::handleFileList, this));
  //load editor
  webServer->on("/edit", HTTP_GET, [&](){ 
  		if (DEBUG) Serial.println("Try for edit...");    ///test code 
		if(!handleFileRead("/edit.htm")) webServer->send(404, "text/plain", "FileNotFound");
  });
  // //create file
  // webServer->on("/edit", HTTP_PUT, handleFileCreate);
  // //delete file
  // webServer->on("/edit", HTTP_DELETE, handleFileDelete);
  // //first callback is called after the request has ended with all parsed arguments
  // //second callback handles file uploads at that location
  // webServer->on("/edit", HTTP_POST, [&](){ webServer->send(200, "text/plain", ""); }, std::bind(&SGFileServer::handleFileUpload, this));
   
  webServer->onNotFound(std::bind(&SGFileServer:: handleNotFound, this));
  webServer->begin();
  if (DEBUG) Serial.println("HTTP server started");

  socketServer.reset(new WebSocketsServer(WS_PORT));
  socketServer->onEvent(std::bind(&SGFileServer::webSocketEvent, this));
  // socketServer->onEvent(webSocketEvent);
  socketServer->begin();
  if (DEBUG) Serial.println("WebSocket server started");  


  _setup = true;

}

WebSocketServerEvent SGFileServer::webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
	if (DEBUG) Serial.println("WebSocket Event Triggered.");  
	
}

void SGFileServer::configureServers(IPAddress ip) {

}


boolean SGFileServer::loadConfig(String path) {
   
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

void SGFileServer::handleFileList() {
	if(!webServer->hasArg("dir")) {webServer->send(500, "text/plain", "BAD ARGS"); return;}
	
	String path = webServer->arg("dir");
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
  	webServer->send(200, "text/json", output);
}

boolean SGFileServer::createFile(String name) {
	

	return false;
}
boolean SGFileServer::wipeFiles() {
	if (SPIFFS.format()) {
		if (DEBUG) Serial.println("File system wipe.");
		return true;
	}
	if (DEBUG) Serial.println("File system failed to wipe.");
	return false;
}

boolean SGFileServer::handleFileRead(String path) {
   if (DEBUG) Serial.println("handleFileRead: " + path);
   if(path.endsWith("/")) path += "index.htm";
	  String contentType = getContentType(path);
   String pathWithGz = path + ".gz";
   
   if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
	  if(SPIFFS.exists(pathWithGz))
		 path += ".gz";
	  File file = SPIFFS.open(path, "r");
	  size_t sent = webServer->streamFile(file, contentType);
	  file.close();
	  return true;
   }
   return false;
}

boolean SGFileServer::handleFileUpload(){
  if(webServer->uri() != "/edit") return false;
  HTTPUpload& upload = webServer->upload();
  
  if(upload.status == UPLOAD_FILE_START){
	
	String filename = upload.filename;
	if(!filename.startsWith("/")) filename = "/"+filename;
	Serial.print("handleFileUpload Name: "); Serial.println(filename);
	fileUpload = SPIFFS.open(filename, "w");
	filename = String();
  
  } else if(upload.status == UPLOAD_FILE_WRITE){
  
	//Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
	if(fileUpload) fileUpload.write(upload.buf, upload.currentSize);
  
  } else if(upload.status == UPLOAD_FILE_END){
	if(fileUpload) fileUpload.close();
	Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }

  /// we need more checks before we say 'true'.
  return true;
}

void SGFileServer::handleNotFound() {
   if ((isCaptivePortal())) { 
	  return;
   }
   if(!handleFileRead(webServer->uri())) {
	  String message = "File Not Found\n\n";
	  message += "URI: ";
	  message += webServer->uri();
	  message += "\nMethod: ";
	  message += ( webServer->method() == HTTP_GET ) ? "GET" : "POST";
	  message += "\nArguments: ";
	  message += webServer->args();
	  message += "\n";

	  for ( uint8_t i = 0; i < webServer->args(); i++ ) {
		 message += " " + webServer->argName ( i ) + ": " + webServer->arg ( i ) + "\n";
	  }
	  webServer->send ( 404, "text/plain", message );
   }
}

boolean SGFileServer::isMdns(String str) {
   if(str.substring(str.length() - 6) == ".local")
	  return true;
   return false;
}

boolean SGFileServer::isSetup() { 
   return _mounted;
}

boolean SGFileServer::isIp(String str) {
   for (int i = 0; i < str.length(); i++) {
	  int c = str.charAt(i);
	  if (c != '.' && (c < '0' || c > '9')) {
		 return false;
	  }
   }
   return true;
}

boolean SGFileServer::isCaptivePortal() {
   if ( !isIp(webServer->hostHeader()) && !isMdns(webServer->hostHeader()) ) { 
	  if (DEBUG) Serial.println("Request redirected to captive portal");
	  webServer->sendHeader("Location", String("http://" + String(_dnsName) + ".local/" ), true);
	  webServer->send ( 302, "text/plain", ""); 
	  webServer->client().stop();
	  return true;
   }
   return false;
}

String SGFileServer::getContentType(String filename) {
   if(webServer->hasArg("download")) return "application/octet-stream";
   else if(filename.endsWith(".htm")) return "text/html";
   else if(filename.endsWith(".html")) return "text/html";
   else if(filename.endsWith(".css")) return "text/css";
   else if(filename.endsWith(".js")) return "application/javascript";
   else if(filename.endsWith(".png")) return "image/png";
   else if(filename.endsWith(".gif")) return "image/gif";
   else if(filename.endsWith(".jpg")) return "image/jpeg";
   else if(filename.endsWith(".ico")) return "image/x-icon";
   else if(filename.endsWith(".wav")) return "audio/wav";
   else if(filename.endsWith(".mp3")) return "audio/mpeg";
   else if(filename.endsWith(".ogg")) return "audio/ogg";
   else if(filename.endsWith(".xml")) return "text/xml";
   else if(filename.endsWith(".pdf")) return "application/x-pdf";
   else if(filename.endsWith(".zip")) return "application/x-zip";
   else if(filename.endsWith(".gz")) return "application/x-gzip";
   else if(filename.endsWith(".json")) return "application/json";
   return "text/plain";
}


//format bytes
String SGFileServer::formatBytes(size_t bytes){
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