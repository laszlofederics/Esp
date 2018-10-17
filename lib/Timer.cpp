#include "Timer.h"

using namespace Esp8266Base;

void ICACHE_FLASH_ATTR global_timer_callback(void * args)
{
	if (args != NULL)
	{
		Timer* p = (Timer*)args;
		p->timeOut.emit(NULL);
	}
}


void ICACHE_FLASH_ATTR Timer::start(int ms, bool bRepeat)
{
	os_timer_disarm(&m_osTimer);
	os_timer_setfn(&m_osTimer, global_timer_callback, this);
	os_timer_arm(&m_osTimer, ms, bRepeat);
}


void ICACHE_FLASH_ATTR Timer::stop()
{
	os_timer_disarm(&m_osTimer);
}
