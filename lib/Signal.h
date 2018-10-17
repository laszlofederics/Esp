#ifndef ESP8266_SIGNAL_H_INCLUDED
#define ESP8266_SIGNAL_H_INCLUDED

/* ************************************************************************** */
/* *************     Configuration settings                ****************** */
/* ************************************************************************** */

// maximum number of simultaneous signal-slot connections
#define MAX_NR_OF_SIGNAL_SLOT_CONNECTIONS 100

// maximum number of queued signals
#define MAX_NR_OF_QUEUED_SIGNALS 10

// priority of the task, which is processing the queued signal (0 is lowest priority)
#define PRIORITY_OF_PROCESSING_QUEUED_SIGNALS USER_TASK_PRIO_0
//#define PRIORITY_OF_PROCESSING_QUEUED_SIGNALS USER_TASK_PRIO_1
//#define PRIORITY_OF_PROCESSING_QUEUED_SIGNALS USER_TASK_PRIO_2

/* *************     End configuration settings           ******************* */


/* This file is from https://github.com/dreamcat4/FastDelegate
   I had to remove checking _MSC_VER, otherwise the ESP8266 compiler fails.
*/
extern "C"
{
  #include <osapi.h>
}

#include "FastDelegate.h"

namespace Esp8266Base
{

using namespace fastdelegate;


/* \class Signal
   \brief Simple implementation of the signal slot pattern (based on FastDelegate)
   
   This class provides the signal-slot functionality for the ESP8266 platform.
   Features:
   - what you would except as signal-slot functionality: emitting a signal will result in calling the connected slot(s)
   - two types of signal-slot connections: 
     - DirectConnection: emitting the signal will call the slots as a direct function call from emit
     - QueuedConnection: calling the slot is decoupled from emitting the signal. The slot won't be called directly from emit. (This feature is important
       for the ESP8266 platform to let run the "background SW", which maintains WiFi connectivity; otherwise the watchdog might reset the chip)
     .
   - no dynamic heap management to store the signal-slot connections, or to store the queued signals
   .
   Restrictions:
   - each slot must have an input parameter void*, and can't have return value
   - the number of simultaneous signal-slot connections is limited in MAX_NR_OF_SIGNAL_SLOT_CONNECTIONS
   - the number of queued signals is limited in MAX_NR_OF_QUEUED_SIGNALS
   - the implementation is not interrupt-proof, so don't use the class directly from interrupt handler
   .
*/

class Signal
{
public:
	
	enum ConnectionType
	{
		//! The slot will be directly called from emit()
		DirectConnection,
		
		//! The slot won't be called from emit(). The slot will be called later, scheduled by the chip's scheduler
		QueuedConnection
	};
	
	// Delegate a function which takes void* and returns void
	typedef FastDelegate1<void *> VoidFunction;

	
	/*! Default constructor.
	*/
	Signal();

	
	/*! Constructor which prints the this pointer and the name of the signal. It makes debugging easier, if you which this-pointer belongs to which signal.
    */
	Signal(const char* strSignalName);

	
	/*! Connects the signal to the receiverFunction slot of the object receiverObject
	    The function returns an integer between 0 and MAX_NR_OF_SIGNAL_SLOT_CONNECTIONS if succeeded, otherwise returns -1.
	    The error code -1 means that MAX_NR_OF_SIGNAL_SLOT_CONNECTIONS is too small for the application.
	    You cann connect one signal to the same receiverObject/receiverFunction multiple time -> then the slot will be invoked multiple times too.
	 */
	template < class X, class Y >
    int connect(Y *receiverObject, void (X::* receiverFunction)(void*), ConnectionType connectionType)
	{
		int iConnectionIndex = 0;
		while (iConnectionIndex < MAX_NR_OF_SIGNAL_SLOT_CONNECTIONS && s_listConnections[iConnectionIndex].m_Signal != 0)
		{
			++iConnectionIndex;
		}

		if (0 <= iConnectionIndex && iConnectionIndex < MAX_NR_OF_SIGNAL_SLOT_CONNECTIONS)
		{
			s_listConnections[iConnectionIndex].m_Signal = this;
			s_listConnections[iConnectionIndex].m_Slot.bind(receiverObject, receiverFunction);
			s_listConnections[iConnectionIndex].m_Type = connectionType;
		}
		else
		{
			iConnectionIndex = -1;
		}
		return iConnectionIndex;
	}

	
	/*! Disconnects the signal from the receiverFunction slot of the object receiverObject.
	    If the signal was connected multiple times to receiverObject/receiverFunction, then all connections are removed.
	    The return value is the number of disconnected signal-slots.
	*/
	template < class X, class Y >
	int disconnect(Y *receiverObject, void (X::* receiverFunction)(void*))
	{
		int iNrOfDisconnects = 0;
		VoidFunction fcnt; fcnt.bind(receiverObject, receiverFunction);
		for (int i = 0; i < MAX_NR_OF_SIGNAL_SLOT_CONNECTIONS; ++i)
		{
			if (s_listConnections[i].m_Signal == this &&
			    s_listConnections[i].m_Slot == fcnt)
			{
				s_listConnections[i].m_Signal = 0;
				++iNrOfDisconnects;
			}
		}
		return iNrOfDisconnects;
	}

	/*! Emits the signal with the parameter pParameter
	*/
	void emit(void* pParameter);

private:
	
	// disable copy constructor
	Signal(const Signal&);
	
	// disable operator=
	Signal& operator=(const Signal&);
	

	struct SignalSlotConnection
	{
		SignalSlotConnection() : m_Signal(0) {  }
		Signal* m_Signal;
		VoidFunction m_Slot;
		ConnectionType m_Type;
	};


	// List of all signal-slot connections
	static SignalSlotConnection s_listConnections[MAX_NR_OF_SIGNAL_SLOT_CONNECTIONS];

	// Is the task which handles queued signals already started?
	static bool s_bTaskInvokeSlotIsStarted;

	// Starts an internal task, which is processing the queued signals
	bool startInvokeTask();

	// Sends an event to taskInvokeSlot to call a slot
	bool invokeSlotQueued(FastDelegate1<void*> functionSlot, void* pParameter);


};

}

#endif
