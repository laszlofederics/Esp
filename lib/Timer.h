#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

extern "C"
{
	#include <osapi.h>
}

#include "Signal.h"

namespace Esp8266Base
{

/*! \class Timer
    \brief Simple timer for ESP8266

   This is a simple timer class, which wraps the C stype API functions of Espressif SDK,
   and provides an object oriented interface for timer functionality.
 */
class Timer
{
public:

	/*! The signal is emitted if the timer expires
	*/
	Signal timeOut;


	/*! Starts the timer.
	*/
	void ICACHE_FLASH_ATTR start(int ms, bool bRepeat);


	/*! Stops the timer.
    */
	void ICACHE_FLASH_ATTR stop();


private:

	os_timer_t m_osTimer;
};

}

#endif
