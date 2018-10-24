extern "C" {
	#include "osapi.h"
	#include "driver/uart.h"
	#include "driver/uart_register.h"
	#include "ets_sys.h"
	void ICACHE_FLASH_ATTR uart1_write_char(char c);
}

#include "EspNowUartGateway.h"
#include "EspWifi.h"


using namespace Esp8266Base;

EspNowUartGateway& ICACHE_FLASH_ATTR EspNowUartGateway::getInstance()
{
	// create and return the one instance of the class
	static EspNowUartGateway sGW;
	return sGW;	
}


ICACHE_FLASH_ATTR EspNowUartGateway::EspNowUartGateway()
{
	m_timerStillAlive.timeOut.connect(this, &EspNowUartGateway::sendImStillAlive, Signal::DirectConnection);
	m_timerStillAlive.start(IM_STILL_ALIVE_TIMEOUT*1000);
}



void ICACHE_FLASH_ATTR EspNowUartGateway::toUart1(const char* msg)
{
	os_printf("%c%s%c", '\0', msg, '\0');

	m_timerStillAlive.start(IM_STILL_ALIVE_TIMEOUT*1000);
}


void ICACHE_FLASH_ATTR EspNowUartGateway::sendImStillAlive(void*)
{
	toUart1(IM_STILL_ALIVE_TEXT);
}


void ICACHE_FLASH_ATTR EspNowUartGateway::EspNow2uart1(void* pEspNowMsg)
{
	m_timerStillAlive.stop();

	EspWifi::EspNowMessage* message = reinterpret_cast<EspWifi::EspNowMessage*>(pEspNowMsg);

	if (message)
	{
		//{"from":"12-34-56-78-90","payload":{"temp":"21","hum":"55"}}
		char buffer[300];
		os_sprintf(buffer, "{\"from\":\"%02x-%02x-%02x-%02x-%02x-%02x\",\"payload\":%s}", MAC2STR(message->from()), message->data());
		
		int jsonlen = os_strlen(buffer);
		uart1_write_char('\0');
		for (int i = 0; i <= jsonlen; ++i)
		{
			uart1_write_char(buffer[i]);
		}
	}

	m_timerStillAlive.start(IM_STILL_ALIVE_TIMEOUT*1000);
}
