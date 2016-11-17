#ifndef SG_FILESERVER
#define SG_FILESERVER

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <FS.h>

#include <ArduinoJson.h>

// #include "SGConfigFile.h"

// enum SGFileServer_STATUS {WIFISTATUS_INITIALIZING,WIFISTATUS_CONNECTING,WIFISTATUS_APSETUP,WIFISTATUS_CONNECTED,WIFISTATUS_DISABLED,WIFISTATUS_FAILED};
// enum SGFileServer_MODE {WIFI,WIFI_AP,};

class SGFileServer {
	public:

		boolean DEBUG = true;

		SGFileServer(){};
		virtual ~SGFileServer();

		void
			configureServers(IPAddress ip),
			connected(),
			webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);

		String       		
			getDNS(),
			getIP(),
			formatBytes(size_t bytes);

		boolean 
			loadConfig(String path),
			listFiles(),
			listFiles(String path),
			createFile(String name),
			wipeFiles(),
			isSetup(),
			updateConnection(String ssid,String pass),
			updateDNS(String ssid,String pass);
			
		
		int 
			persist();

		void
			init(),
			handleNotFound();

	protected:
		boolean 
			_mounted,
			_setup;

		uint8_t 
				_status, 
			_mode,
			_channel;
	  
		String 
			_dnsName,
			_ip;      		

		unsigned long
			_start,
			_timeout;      

		String getContentType(String filename);

		boolean 
			handleFileRead(String path),        	
			isCaptivePortal(),
			isIp(String str),
			isMdns(String str);


		std::unique_ptr <ESP8266WebServer> webServer;
		std::unique_ptr <WebSocketsServer> socketServer;
		std::unique_ptr <DNSServer>        dnsServer;

		/// These are defaults but also need to be added to config
		byte DNS_PORT = 53;
		byte WEB_PORT = 80;
		byte WS_PORT = 81;

};

#endif SG_FILESERVER