#ifndef SG_CONFIGFILE
#define SG_CONFIGFILE

#include <Arduino.h>
#include <ArduinoJson.h>
// #include <FS.h>

class SGConfigFile {
	public:
	
	boolean DEBUG = true;

	SGConfigFile(){};	

	virtual ~SGConfigFile();
	
	// Debug outputs
	void status();
	virtual int test();

	boolean 
		load(String path),
		save();

	protected:
		 // File _config;

		void 
			init();
};

#endif SG_CONFIGFILE