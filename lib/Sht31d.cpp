/* ************************************************************************** */
/* *************     Configuration settings                ****************** */
/* ************************************************************************** */

#define ENABLE_DEBUG

#define SHT31D_ADDRESS 0x44

#define SHT31D_READING_DURATION_MS  150

/* *************     End configuration settings           ******************* */

#include "debug.h"

#include "I2C.h"
#include "Sht31d.h"


using namespace Esp8266Base;


bool ICACHE_FLASH_ATTR Sht31d::startMeasurement()
{
	debug(">>> Sht31d::startMeasurement()\n");

	I2C::ErrorCode nError = I2C::writeCommand(SHT31D_ADDRESS, 0x2400);

	bool bRet = false;

	if (I2C::Succeeded == nError)
	{
		m_tReading.start(SHT31D_READING_DURATION_MS, false, &measurementReady);
		bRet = true;
	}

	debug("<<< Sht31d::startMeasurement() returns %s\n", bRet ? "true":"false");

	return bRet;
}


bool ICACHE_FLASH_ATTR Sht31d::readData(float& temperature, float& humidity)
{
	debug(">>> Sht31d::readData()\n");

	bool bRet = true;

	uint8_t temp1 = 0, temp2 = 0, crc_temp = 0, hum1 = 0, hum2 = 0, crc_hum = 0;
	I2C::ErrorCode nError = I2C::readBytes(SHT31D_ADDRESS, temp1, temp2, crc_temp, hum1, hum2, crc_hum);

	if (I2C::Succeeded == nError)
	{
		if (crc_temp == calculateCrc(temp1, temp2))
		{
			uint16_t temp16 = (temp1 << 8) | temp2;
			uint32_t x = 175 * temp16;
			float y = x;
			temperature = y/65535 - 45;
		}
		else
		{
			printError("ERROR: CRC error in temperature in Sht31d::readData()\n");
			bRet = false;
		}

		if (crc_hum == calculateCrc(hum1, hum2))
		{
			uint16_t hum16 = (hum1 << 8) | hum2;

			uint32_t x = 100 * hum16;
			float y = x;
			humidity = y/65535;
		}
		else
		{
			printError("ERROR: CRC error in humidity in Sht31d::readData()\n");
			bRet = false;
		}
	}
	else
	{
		bRet = false;
	}

#ifdef ENABLE_DEBUG
	// workaround of missing %f format of printf
	int a = temperature*100;
	int b = humidity;
	debug("<<< Sht31d::readData(%d.%d, %d) returns %s\n", a/100, a%100, b, bRet ? "true":"false");
#endif
	return bRet;
}


uint8_t Sht31d::calculateCrc(uint8_t byte1, uint8_t byte2) const
{
	uint8_t bit;
	uint8_t crc = 0xFF;

	crc ^= byte1;
	for (bit = 8; bit > 0; --bit)
	{
		if (crc & 0x80)
			crc = (crc << 1) ^ 0x131;
		else
			crc = (crc << 1);
	}

	crc ^= byte2;
	for (bit = 8; bit > 0; --bit)
	{
		if (crc & 0x80)
			crc = (crc << 1) ^ 0x131;
		else
			crc = (crc << 1);
	}

	return crc;
}

