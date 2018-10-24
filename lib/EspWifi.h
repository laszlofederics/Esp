#ifndef ESP_WIFI_H_INCLUDED
#define ESP_WIFI_H_INCLUDED

extern "C" {
  #include "user_interface.h"
  #include "espconn.h"
}

#include "Signal.h"

namespace Esp8266Base
{

/*! \class EspWifi
    \brief Provide basic WiFi functionality of the ESP8266 in a very simple and convenient way
	
	Usage:
	
	- connecting to an access point (AP)
	  - with this class it is only possible with WPS to connect to an AP
	  - call the function startWpsConfig() and activate WPS on your access point (by pressing its button)
	  - wait for the signal, which shows if WPS succeeded or failed (wpsConfigFailed or connectedToAP)
	  - the WiFi credentials are stored in flash (by Espressif SDK), and if the ESP boots next time, it will automatically connect to the
	    same AP (don't need to call any functions). If the ESP can connect (or can't connect), the appropriate signals are emitted.
	  - if this class is used to connect to an AP, then we always use station mode and auto connect = 1.
	  .
	- sending and receiving ESP-Now messages
	  - with this class it is only possible to send messages to just one ESP. This one ESP is intended to be the ESP-Now gateway.
	  - the MAC address of the ESP-Now gateway is a hard-coded locally administered MAC address. (https://en.wikipedia.org/wiki/MAC_address#Address_details)
	    So this address is fixed in the nodes and in the gateway in compile time.
	  - its disadvantage is the lack of flexibility; its advantage is less complexity, and easy deployment   
	  .
	.
*/
class EspWifi
{
public:

	/*! \class EspNowMessage
	    \brief Stores the content(payload) and the sender(MAC address) of a received ESP-now message. It only works for ASCII string messages
	*/
	class EspNowMessage
	{
	public:

		/*! Creates an ESP-now message
		*/
		ICACHE_FLASH_ATTR EspNowMessage(uint8_t *mac, char *data);

		/*! Returns the MAC address of the sender
		*/
		const uint8_t* ICACHE_FLASH_ATTR from() const;

		/*! Returns the content of the message
		*/
		const char* ICACHE_FLASH_ATTR data() const;

	private:
		uint8_t m_mac[6];
		char m_data[255];
	};
	
	/*! \class UdpMessage
		\brief Stores the content(payload) of a received UDP message.

		It only works for ASCII string messages, and the maximal size for the message is 255 bytes.
	*/
	class UdpMessage
	{
	public:

		/*! Creates an UDP message
		*/
		ICACHE_FLASH_ATTR UdpMessage(char *data);

		/*! Returns the content of the message
		*/
		const char* ICACHE_FLASH_ATTR data() const;

	private:
		char m_data[256];
	};
	

	/*! \enum Mode
	    \brief Operating modes for EspWifi
	*/
	enum Mode
	{
		Off,
		ConnectToAP,
		SendEspNow,
		ReceiveEspNow,
		Default
	};

	/* Returns the one an only instance of the class. The instance of the class will be initialized according to nMode. The parameter nMode is
	   only relevant for the first call of getInstance(). the first call of getInstance() (-> creating the instance of EspWifi) must be in user_init.
	*/
	static EspWifi& ICACHE_FLASH_ATTR getInstance(Mode nMode = Default);

	
	/*! Sets the ESP in station mode, clears the SSID and password, enables auto connect and DHCP.
	*/
    bool ICACHE_FLASH_ATTR factoryReset() const;

    
	/*! Starts the WPS configuration. If the WPS has been started, then the callback function onWpsConfigStarted() is called. If the WPS has been finished with error,
        then the callback function onWpsConfigFailed() is called. If WPS configuration was successful, then the callback function onConnectedToAP() is called, and the ESP
		is connected to the access point. The SSID and password is saved, so that if the ESP next time boots, it will connect automatically to the same AP.
	*/
    void ICACHE_FLASH_ATTR startWpsConfig();

	
    void ICACHE_FLASH_ATTR listenForUdpMessages(bool bListen);
  
    //! The ESP has been disconnected from the access point
    Signal disconnectedFromAP;
    
    //! The ESP has been connected to the access point
    Signal connectedToAP;
    
    //! WPS has been started
    Signal wpsConfigStarted;
    
    //! WPS has been failed
    Signal wpsConfigFailed;
    
    //! An ESP-now message has been sent successfully
    Signal espNowMessageSent;
    
    //! Sendig the ESP-now message has been failed
    Signal espNowMessageSendFailed;
    
    //! An ESP-now message has been received
    Signal espNowMessageReceived;
    
    //! An UDP message has been received 
    Signal udpMessageReceived;
  
	/*! Initialises the ESP for sending ESP-Now messages. The messages will be sent to the ESP chip with the MAC address macReceiver.
	    The return code is 0 on success, otherwise an error code.
	*/
	int ICACHE_FLASH_ATTR espNowSend(const char* data, int length);
    
    void ICACHE_FLASH_ATTR printInfo();
  
        
private:

    // private constructor. Only one instance of the class is allowed.         
    ICACHE_FLASH_ATTR EspWifi(Mode nMode);
        
    // disable copy constructor 
    EspWifi(const EspWifi&);
	
	// disable operator=
    EspWifi& operator=(const EspWifi&);

    espconn m_espconnUDP;
    Mode m_nMode;
    
    friend void ICACHE_FLASH_ATTR espNowSendCallback(uint8_t *mac_addr, uint8_t status);
	friend void ICACHE_FLASH_ATTR espNowRecvCallback(uint8_t *mac, uint8_t *data, uint8_t len);
	friend void ICACHE_FLASH_ATTR wifiEventHandler(System_Event_t *evt);
  
};

}

#endif
