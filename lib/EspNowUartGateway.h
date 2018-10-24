#ifndef ESPNOW_UART_GATEWAY_H_INCLUDED
#define ESPNOW_UART_GATEWAY_H_INCLUDED

/* ************************************************************************** */
/* *************     Configuration settings                ****************** */
/* ************************************************************************** */

/* If the gateway doesn't receive ESP-Now messages for a long time, then we don't send anything to the UART1 for a long time.
   The receiver side (host computer) doesn't receive anything, and might think that we are broken or mailfunctioning.
   To avoid this, the gateway can send regurarly "Im still alive" messages, if we don't receive ESP-Now messages for a long time.
   The emitted text is defined with this makro.
*/
#define IM_STILL_ALIVE_TEXT "Im still alive\n"

/* The "Im still alive" message will be transmitted to UART1, if we didn't transmit anything for IM_STILL_ALIVE_TIMEOUT nr of seconds
*/
#define IM_STILL_ALIVE_TIMEOUT 60

/* *************     End configuration settings           ******************* */



#include "Signal.h"
#include "Timer.h"

namespace Esp8266Base
{

/* \class EspNowUartGateway
   \brief ESP-now ---> UART1 gateway

   A simple one directional ESP-now UART1 gateway. If this class receives an ESP-Now message, then transmits it on the TX pin (GPIO2) of UART1.
   The content of the ESP-now message must be a \0 terminated ASCII string.
   The message will converted into JSON format: {"from":"12-34-56-78-90","payload":xxxxx}, where xxxxx is the content of the original ESP-now message.
   Each message on the UART1 is surrounded by a \0.
   If the gateway doesn't receive ESP-now messages for a long time (and because of that doesn't transmit anything on UART1), then it will
   send "still alive"messages to settle the client, that the gateway is still functioning.
*/
class EspNowUartGateway
{
public:
	
	/* Returns the one an only instance of the class.
	*/
    static EspNowUartGateway& ICACHE_FLASH_ATTR getInstance();


    /* pEspNowMsg is a pointer to an ESP-now message (instance of the class EspNowMessage)
       It will be encapsulated in a JSON string and transmitted on UART1 TX.
    */
    void ICACHE_FLASH_ATTR EspNow2uart1(void* pEspNowMsg);


    /* Sends the string to uart1 TX (GPIO2) with the terminating null. The string cannot be longer than 120 chars (according to Espressif SDK), so this function
       is intended to transmit only short strings.
    */
    void ICACHE_FLASH_ATTR toUart1(const char* msg);

private:
    
    // private constructor. Only one instance of the class is allowed.         
	ICACHE_FLASH_ATTR EspNowUartGateway();
		
	// disable copy constructor 
	EspNowUartGateway(const EspNowUartGateway&);
	
	// disable operator=
	EspNowUartGateway& operator=(const EspNowUartGateway&);

	// Measure time since the last ESP-now message
	Timer m_timerStillAlive;
    

	/* Writes the string "Im still alive" to UART1.
	   If the gateway doesn't receive ESP-Now messages for a long time, then we don't send anything to the UART1 for a long time.
	   The receiver side (host computer) doesn't receive anything, and might think that we are broken or mailfunctioning.
	   To avoid this, the gateway can send regurarly "Im still alive" messages, if we don't receive ESP-Now messages for a long time.
	*/
    void ICACHE_FLASH_ATTR sendImStillAlive(void*);

};

}
#endif
