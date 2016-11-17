#include "SGWifi.h"

SGWifi::~SGWifi() {}

int SGWifi::persist() {

	// If we aren't connected, let's get there...
	if (_status != WIFISTATUS_CONNECTED) {
		unsigned long _now = millis();

		if (_status == WIFISTATUS_DISABLED) return _status;
		if (_status == WIFISTATUS_INITIALIZING) {
			Serial.println("Station initializing.");
			_status = WIFISTATUS_CONNECTING;
			_start = millis();
			_timeout = 20 * 1000; // 20 seconds /// get from Config

			WiFi.mode(WIFI_STA);

			if(_ssid) {
	            if(_pass) {
					WiFi.begin(_ssid.c_str(), _pass.c_str());
	            } else {
					WiFi.begin(_ssid.c_str());
				}
			} else {
				WiFi.begin();
			}
			return _status;
		}
	
		// If we trying to connect, let's get there...
		if (_status == WIFISTATUS_CONNECTING) {
			
			if(WiFi.status() != WL_CONNECTED) {
				
				if(millis() - _start >= _timeout) { 
					// default sta connection timed out ... start default ap mode
					if (DEBUG) Serial.println("Station timed out...");

					// wifi_status = WIFISTATUS_FAILED; ///2
					
					_status = WIFISTATUS_APSETUP;

					
		            _start = millis();
		            _timeout = 120 * 1000; // 2 minutes
		            WiFi.mode(WIFI_AP); // start AP
	            	_fallbackSSID = "ESP_" + String(ESP.getChipId()).substring(4);;

	            	boolean ap;
	            	
	            	if (_fallbackPass != "") {
	               		ap = WiFi.softAP(_fallbackSSID.c_str(), _fallbackPass.c_str(), _channel);
	               	}else{
	               		ap = WiFi.softAP(_fallbackSSID.c_str(), NULL, _channel);
	               	}

	               	if (ap) {
	               		if (DEBUG) Serial.println("AP started");
               		} else {
                  		if (DEBUG) Serial.println("AP failed"); // AP failed ... shutdown wifi
                  		_status = WIFISTATUS_DISABLED;
                  		WiFi.mode(WIFI_OFF);
                  		return _status;
	               	}

					return _status;
				} else {
					// Serial.print(".");
					return _status; // keep trying...
				}

			} else { // sta is connected 	
				if (DEBUG) Serial.println("Station is connected...");			
				_mode = 1; ///I think this means we are not AP
	            _status = WIFISTATUS_CONNECTED;            
				return _status;
			}			
		}
		if (_status == WIFISTATUS_APSETUP) {
			if(WiFi.softAPgetStationNum() == 0) { // no connections to AP
	            if(_now - _start >= _timeout) { // AP connection timed out 
	                if (DEBUG) Serial.println("AP timed out "); // AP timed out ... shutdown wifi
	                _status = WIFISTATUS_FAILED;
	                WiFi.mode(WIFI_OFF);
	                return _status;
	            }
	        } else {
		        _status = WIFISTATUS_CONNECTED; // we have an AP connection
		    }
		}
		
		return _status;	
	}

	// WiFi is connected 
	
	// do DNS for AP
	// if(_mode != 1); 
      // dnsServer->processNextRequest();
}

void SGWifi::init() {

	// Defaults
	_status = WIFISTATUS_INITIALIZING;
	_mode = 0;
	_start = 0; 
	_timeout = 0;
	_channel = 11;
}

boolean SGWifi::isIP(String str) {
   for (int i = 0; i < str.length(); i++) {
      int c = str.charAt(i);
      if (c != '.' && (c < '0' || c > '9')) {
         return false;
      }
   }
   return true;
}