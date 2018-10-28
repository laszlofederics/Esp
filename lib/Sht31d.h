#ifndef SHT31D_H_INCLUDED
#define SHT31D_H_INCLUDED

#include "Signal.h"
#include "Timer.h"

namespace Esp8266Base
{

class Sht31d
{
public:
		
	bool ICACHE_FLASH_ATTR startMeasurement();
	
	bool ICACHE_FLASH_ATTR readData(float& temp, float& hum);
	
	Signal measurementReady;

	
	
private:
	
	uint8_t calculateCrc(uint8_t byte1, uint8_t byte2) const;
	
	Timer m_tReading;
};

}

#endif
