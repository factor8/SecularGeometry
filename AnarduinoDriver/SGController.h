// TEST TEMPLATE FOR CONTROLLER CLASS

class SGController {
	public:
		uint16_t _frameRate = 24; // framerate. 25ms = 40fps

		SGController(){};
		SGController(uint16_t* effectList){};
		virtual ~SGController(){};

		virtual boolean render();
		virtual boolean loadConfig();
		void loadEffect();
		void setBrightness();
		void setEffect();
		void setSelector();		

	protected:

		enum eName {eColorWipe,eRainbow,eRainbowCycle};		
		Vector<SGEffect*> _effects;
		Vector<SGOutput*> _outputs;		 

		uint8_t _brightness;
		uint16_t _selector;
		boolean _transitioning = 0;
};

