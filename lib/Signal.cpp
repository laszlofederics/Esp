/* ************************************************************************** */
/* *************     Configuration settings                ****************** */
/* ************************************************************************** */

//#define ENABLE_DEBUG

/* *************     End configuration settings           ******************* */


#include "Signal.h"

using namespace Esp8266Base;

extern "C" {
  #include <user_interface.h>
  #include <osapi.h>
  #include <mem.h>
}

#include "debug.h"

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
	debug("%p >>> emit()\n", this);

	bool bOnlyDirectConnections = true;
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
				bOnlyDirectConnections = false;
			}
		}
	}

	if (bOnlyDirectConnections)
	{
		// if there were only direct connections, then we can release the memory of param now
		if (NULL != param)
		{
			os_free(param);
		}
	}
	else
	{
		// if there were Queued connections too, then param can be freed after the last slot has been called
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


bool isParameterQueuedForOtherSlots(int i, void* param)
{
  bool bUsed = false;
  for (int j = 0; j < MAX_NR_OF_QUEUED_SIGNALS && !bUsed; ++j)
  {
	  if (i != j && g_arrayInvokeData[j].pParameter == param)
	  {
		  bUsed = true;
	  }
  }
  return bUsed;
}


void taskInvokeSlot(os_event_t *e)
{
	debug(">>> taskInvokeSlot()\n");

	if (INVOKE_SLOT == e->sig)
	{
		int i = e->par;
		if (0 <= i && i < MAX_NR_OF_QUEUED_SIGNALS && g_arrayInvokeData[i].bUsed)
		{
			g_arrayInvokeData[i].functionSlot(g_arrayInvokeData[i].pParameter);
			g_arrayInvokeData[i].bUsed = false;
			if (NULL != g_arrayInvokeData[i].pParameter)
			{
				if (isParameterQueuedForOtherSlots(i, g_arrayInvokeData[i].pParameter))
				{
					g_arrayInvokeData[i].pParameter = NULL;
				}
				else
				{
					os_free(g_arrayInvokeData[i].pParameter);
					g_arrayInvokeData[i].pParameter = NULL;
				}
			}
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
	debug(">>> Signal::startInvokeTask()\n");
	bool bRet = system_os_task(taskInvokeSlot, PRIORITY_OF_PROCESSING_QUEUED_SIGNALS, g_eventQueue, MAX_NR_OF_QUEUED_SIGNALS);
	debug("<<< Signal::startInvokeTask() returns %s\n", bRet ? "true":"false");
	return bRet;
}

bool Signal::invokeSlotQueued(FastDelegate1<void*> functionSlot, void* pParameter)
{
	debug(">>> Signal::invokeSlotQueued()\n");

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
		   printError("ERROR: Signal::invokeSlotQueued couldn't post event. Event queue too small?\n");
		}
	}
	else
	{
		printError("ERROR: Signal::invokeSlotQueued couldn't post event. Invoke data array too small?\n");
	}

	debug("<<< Signal::invokeSlotQueued() returns %s\n", bRet ? "true":"false");

	return bRet;
}
