#include "stdint.h"


	void neo6mGPS_begin(void * send(void*,uint32_t), void* receive(void*,uint32_t));
	void setupGPS(uint32_t baud, uint16_t hertz);
	//void disableAllNmea();
	//void enableAllNmea();
	void setSentence(char NMEA_num, bool enable);
	//void changeBaud(uint32_t baud);
	void changeFreq(uint16_t hertz);
	bool available();
