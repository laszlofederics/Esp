/* ************************************************************************** */
/* *************     Configuration settings                ****************** */
/* ************************************************************************** */

#define ENABLE_DEBUG

/* *************     End configuration settings           ******************* */


#include <osapi.h>

#include "Signal.h"

using namespace Esp8266Base;

extern "C" {
  #include "user_interface.h"
  extern int ets_uart_printf(const char *fmt, ...);
}

#ifdef ENABLE_DEBUG
  #define debug ets_uart_printf
#else
  #define debug
#endif

#define printError ets_uart_printf


Signal::SignalSlotConnection Signal::s_listConnections[MAX_NR_OF_SIGNAL_SLOT_CONNECTIONS];
bool Signal::s_bTaskInvokeSlotIsStarted = false;


Signal::Signal()
{
	if (false == s_bTaskInvokeSlotIsStarted)
	{
		s_bTaskInvokeSlotIsStarted = startInvokeTask();
	}
}

Signal::Signal(const char* strSignalName)
{
	debug("%p Signal::Signal(%s)\n", this, strSignalName);

	if (false == s_bTaskInvokeSlotIsStarted)
	{
		s_bTaskInvokeSlotIsStarted = startInvokeTask();
	}
}



void Signal::emit(void* param)
{
	debug("%p >>> emit(), time: %d\n", this, system_get_time());

	for (int i = 0; i < MAX_NR_OF_SIGNAL_SLOT_CONNECTIONS; ++i)
	{
		if (s_listConnections[i].m_Signal == this)
		{
			if (s_listConnections[i].m_Type == DirectConnection)
			{
				// call the slot directly
				s_listConnections[i].m_Slot(param);
			}
			else
			{
				// the slot will be invoked later
				invokeSlotQueued(s_listConnections[i].m_Slot, param);
			}
		}
	}

	debug("%p <<< emit()\n", this);
}




struct InvokeData
{
	InvokeData() : bUsed(false) {}
	bool bUsed;
	FastDelegate1<void*> functionSlot;
    void* pParameter;
};

#define INVOKE_SLOT 1928

InvokeData g_arrayInvokeData[MAX_NR_OF_QUEUED_SIGNALS];
os_event_t g_eventQueue[MAX_NR_OF_QUEUED_SIGNALS];

void taskInvokeSlot(os_event_t *e)
{
	debug(">>> taskInvokeSlot(), time: %d\n", system_get_time());

	if (INVOKE_SLOT == e->sig)
	{
		int i = e->par;
		if (0 <= i && i < MAX_NR_OF_QUEUED_SIGNALS && g_arrayInvokeData[i].bUsed)
		{
			g_arrayInvokeData[i].functionSlot(g_arrayInvokeData[i].pParameter);
			g_arrayInvokeData[i].bUsed = false;
		}
		else
		{
			printError("ERROR: The task taskInvokeSlot() is called with an event with wrong data.\n");
		}
	}
	else
	{
		printError("ERROR: The task taskInvokeSlot() is called with wrong event. Others are posting events with the same priority?\n");
	}

	debug("<<< taskInvokeSlot()\n");
}

bool Signal::startInvokeTask()
{
	debug(">>> startInvokeTask()\n");
	bool bRet = system_os_task(taskInvokeSlot, PRIORITY_OF_PROCESSING_QUEUED_SIGNALS, g_eventQueue, MAX_NR_OF_QUEUED_SIGNALS);
	debug("<<< startInvokeTask() returns %s\n", bRet ? "true":"false");
	return bRet;
}

bool Signal::invokeSlotQueued(FastDelegate1<void*> functionSlot, void* pParameter)
{
	debug(">>> invokeSlotQueued(), time: %d\n", system_get_time());

	bool bRet = false;
	int i = 0;
	while (i < MAX_NR_OF_QUEUED_SIGNALS && g_arrayInvokeData[i].bUsed)
	{
		++i;
	}

	if (0 <= i && i < MAX_NR_OF_QUEUED_SIGNALS)
	{
		g_arrayInvokeData[i].bUsed = true;
		g_arrayInvokeData[i].functionSlot = functionSlot;
		g_arrayInvokeData[i].pParameter = pParameter;

		bRet = system_os_post(PRIORITY_OF_PROCESSING_QUEUED_SIGNALS, INVOKE_SLOT, i);

		if (false == bRet)
		{
		   printError("ERROR: invokeSlotQueued couldn't post event. Event queue too small?\n");
		}
	}
	else
	{
		printError("ERROR: invokeSlotQueued couldn't post event. Invoke data array too small?\n");
	}

	debug("<<< invokeSlotQueued() returns %s\n", bRet ? "true":"false");

	return bRet;
}
