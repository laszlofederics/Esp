#ifndef I2CI_H_INCLUDED
#define I2CI_H_INCLUDED

extern "C"
{
	#include "c_types.h"
}

namespace Esp8266Base
{

/*! \class I2C
    \brief Functions to drive the I2C Bus

   This class provides basic function to use the I2C bus as a master.
 */
class I2C
{
public:

	enum ErrorCode
	{
		Succeeded = 0,
		NoAddressAcknowledgment = 1,
		NoCommandAcknowledgment = 2
	};
	
	/*! Initializes the I2C bus.
	*/
	static void ICACHE_FLASH_ATTR init();

	/*! Writes a command to the I2C bus.
	*/
	static ErrorCode ICACHE_FLASH_ATTR writeCommand(uint8_t address, uint16_t command);
	
	/*! Read one byte from the I2C bus
	*/
	static ErrorCode ICACHE_FLASH_ATTR readBytes(uint8_t address, uint8_t& byte1);

	/*! Read two bytes from the I2C bus
	*/
	static ErrorCode ICACHE_FLASH_ATTR readBytes(uint8_t address, uint8_t& byte1, uint8_t& byte2);

	/*! Read three bytes from the I2C bus
	*/
	static ErrorCode ICACHE_FLASH_ATTR readBytes(uint8_t address, uint8_t& byte1, uint8_t& byte2, uint8_t& byte3);

	/*! Read four bytes from the I2C bus
	*/
	static ErrorCode ICACHE_FLASH_ATTR readBytes(uint8_t address, uint8_t& byte1, uint8_t& byte2, uint8_t& byte3, uint8_t& byte4);

	/*! Read five bytes from the I2C bus
	*/
	static ErrorCode ICACHE_FLASH_ATTR readBytes(uint8_t address, uint8_t& byte1, uint8_t& byte2, uint8_t& byte3, uint8_t& byte4, uint8_t& byte5);

	/*! Read six bytes from the I2C bus
	*/
	static ErrorCode ICACHE_FLASH_ATTR readBytes(uint8_t address, uint8_t& byte1, uint8_t& byte2, uint8_t& byte3, uint8_t& byte4, uint8_t& byte5, uint8_t& byte6);
};

}

#endif
