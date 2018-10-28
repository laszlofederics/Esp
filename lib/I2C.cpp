
#include "I2C.h"

#include "debug.h"

extern "C"
{
	#include <driver/i2c_master.h>
}

using namespace Esp8266Base;

I2C::ErrorCode ICACHE_FLASH_ATTR I2C::writeCommand(uint8_t address, uint16_t command)
{
	ErrorCode nRet = Succeeded;
	
	i2c_master_start();

	uint8_t address_shifted = (address << 1);

	i2c_master_writeByte(address_shifted);
	if (!i2c_master_checkAck())
	{
		i2c_master_stop();
		nRet = NoAddressAcknowledgment;
		printError("ERROR: I2C::writeCommand(0x%02x, 0x%04x) returns NoAddressAcknowledgment\n", address, command);
	}
	else
	{
		i2c_master_writeByte(command >> 8);
		if (!i2c_master_checkAck())
		{
			i2c_master_stop();
			nRet = NoCommandAcknowledgment;
			printError("ERROR: I2C::writeCommand(0x%02x, 0x%04x) returns NoCommandAcknowledgment\n", address, command);
		}
		else
		{
			i2c_master_writeByte(command & 0xFF);
			if (!i2c_master_checkAck())
			{
				i2c_master_stop();
				nRet = NoCommandAcknowledgment;
				printError("ERROR: I2C::writeCommand(0x%02x, 0x%04x) returns NoCommandAcknowledgment\n", address, command);
			}
			else
			{
				i2c_master_stop();
				nRet = Succeeded;
			}
		}
	}
	
	return nRet;
}


I2C::ErrorCode ICACHE_FLASH_ATTR I2C::readBytes(uint8_t address, uint8_t& byte1)
{
	ErrorCode nRet = Succeeded;
	
	i2c_master_start();

	uint8_t address_shifted = ((address << 1) | 1) & 0xFF;

	i2c_master_writeByte(address_shifted);
	if (!i2c_master_checkAck())
	{
		i2c_master_stop();
		nRet = NoAddressAcknowledgment;
		printError("ERROR: I2C::readBytes(0x%02x) returns NoAddressAcknowledgment\n", address);
	}
	else
	{
		byte1 = i2c_master_readByte();
		i2c_master_send_nack();
		i2c_master_stop();
		
		nRet = Succeeded;
	}
	
	return nRet;
}

I2C::ErrorCode ICACHE_FLASH_ATTR I2C::readBytes(uint8_t address, uint8_t& byte1, uint8_t& byte2)
{
	ErrorCode nRet = Succeeded;
	
	i2c_master_start();

	uint8_t address_shifted = ((address << 1) | 1) & 0xFF;

	i2c_master_writeByte(address_shifted);
	if (!i2c_master_checkAck())
	{
		i2c_master_stop();
		nRet = NoAddressAcknowledgment;
		printError("ERROR: I2C::readBytes(0x%02x) returns NoAddressAcknowledgment\n", address);
	}
	else
	{
		byte1 = i2c_master_readByte();
		i2c_master_send_ack();
		byte2 = i2c_master_readByte();
		i2c_master_send_nack();
		i2c_master_stop();
		
		nRet = Succeeded;
	}
	
	return nRet;
}

I2C::ErrorCode ICACHE_FLASH_ATTR I2C::readBytes(uint8_t address, uint8_t& byte1, uint8_t& byte2, uint8_t& byte3)
{
	ErrorCode nRet = Succeeded;
	
	i2c_master_start();

	uint8_t address_shifted = ((address << 1) | 1) & 0xFF;

	i2c_master_writeByte(address_shifted);
	if (!i2c_master_checkAck())
	{
		i2c_master_stop();
		nRet = NoAddressAcknowledgment;
		printError("ERROR: I2C::readBytes(0x%02x) returns NoAddressAcknowledgment\n", address);
	}
	else
	{
		byte1 = i2c_master_readByte();
		i2c_master_send_ack();
		byte2 = i2c_master_readByte();
		i2c_master_send_ack();
		byte3 = i2c_master_readByte();
		i2c_master_send_nack();
		i2c_master_stop();
		
		nRet = Succeeded;
	}
	
	return nRet;
}

I2C::ErrorCode ICACHE_FLASH_ATTR I2C::readBytes(uint8_t address, uint8_t& byte1, uint8_t& byte2, uint8_t& byte3, uint8_t& byte4)
{
	ErrorCode nRet = Succeeded;
	
	i2c_master_start();

	uint8_t address_shifted = ((address << 1) | 1) & 0xFF;

	i2c_master_writeByte(address_shifted);
	if (!i2c_master_checkAck())
	{
		i2c_master_stop();
		nRet = NoAddressAcknowledgment;
		printError("ERROR: I2C::readBytes(0x%02x) returns NoAddressAcknowledgment\n", address);
	}
	else
	{
		byte1 = i2c_master_readByte();
		i2c_master_send_ack();
		byte2 = i2c_master_readByte();
		i2c_master_send_ack();
		byte3 = i2c_master_readByte();
		i2c_master_send_ack();
		byte4 = i2c_master_readByte();
		i2c_master_send_nack();
		i2c_master_stop();
		
		nRet = Succeeded;
	}
	
	return nRet;
}

I2C::ErrorCode ICACHE_FLASH_ATTR I2C::readBytes(uint8_t address, uint8_t& byte1, uint8_t& byte2, uint8_t& byte3, uint8_t& byte4, uint8_t& byte5)
{
	ErrorCode nRet = Succeeded;
	
	i2c_master_start();

	uint8_t address_shifted = ((address << 1) | 1) & 0xFF;

	i2c_master_writeByte(address_shifted);
	if (!i2c_master_checkAck())
	{
		i2c_master_stop();
		nRet = NoAddressAcknowledgment;
		printError("ERROR: I2C::readBytes(0x%02x) returns NoAddressAcknowledgment\n", address);
	}
	else
	{
		byte1 = i2c_master_readByte();
		i2c_master_send_ack();
		byte2 = i2c_master_readByte();
		i2c_master_send_ack();
		byte3 = i2c_master_readByte();
		i2c_master_send_ack();
		byte4 = i2c_master_readByte();
		i2c_master_send_ack();
		byte5 = i2c_master_readByte();
		i2c_master_send_nack();
		i2c_master_stop();
		
		nRet = Succeeded;
	}
	
	return nRet;
}

I2C::ErrorCode ICACHE_FLASH_ATTR I2C::readBytes(uint8_t address, uint8_t& byte1, uint8_t& byte2, uint8_t& byte3, uint8_t& byte4, uint8_t& byte5, uint8_t& byte6)
{
	ErrorCode nRet = Succeeded;
	
	i2c_master_start();

	uint8_t address_shifted = ((address << 1) | 1) & 0xFF;

	i2c_master_writeByte(address_shifted);
	if (!i2c_master_checkAck())
	{
		i2c_master_stop();
		nRet = NoAddressAcknowledgment;
		printError("ERROR: I2C::readBytes(0x%02x) returns NoAddressAcknowledgment\n", address);
	}
	else
	{
		byte1 = i2c_master_readByte();
		i2c_master_send_ack();
		byte2 = i2c_master_readByte();
		i2c_master_send_ack();
		byte3 = i2c_master_readByte();
		i2c_master_send_ack();
		byte4 = i2c_master_readByte();
		i2c_master_send_ack();
		byte5 = i2c_master_readByte();
		i2c_master_send_ack();
		byte6 = i2c_master_readByte();
		i2c_master_send_nack();
		i2c_master_stop();
		
		nRet = Succeeded;
	}
	
	return nRet;
}


void ICACHE_FLASH_ATTR I2C::init()
{
	i2c_master_gpio_init();
	i2c_master_init();
}
