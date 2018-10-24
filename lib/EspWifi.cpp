/* ************************************************************************** */
/* *************     Configuration settings                ****************** */
/* ************************************************************************** */

#define ENABLE_DEBUG

#define ESP_NOW_WIFI_CHANNEL 1

/* Define a locally administered MAC address of the ESP-Now gateway. This address will be compiled in the gateway(as MAC address) and in the nodes too. The nodes can
   send messages only to the gateway.
   To find a locally  administered MAC address: https://en.wikipedia.org/wiki/MAC_address#Universal_vs._local
*/
unsigned char ESP_NOW_GATEWAY_MAC[] = {0x06, 0x00, 0x00, 0x00, 0x00, 0x00};

/* *************     End configuration settings           ******************* */


#include "EspWifi.h"

extern "C" {
  #include <user_interface.h>
  #include <espnow.h>
  #include <osapi.h>
  #include <mem.h>
  #include <string.h>
  #include "ets_decl.h"
}

using namespace Esp8266Base;

#ifdef ENABLE_DEBUG
  #define debug ets_uart_printf
#else
  #define debug
#endif

#define print ets_uart_printf



ICACHE_FLASH_ATTR EspWifi::EspNowMessage::EspNowMessage(uint8_t *mac, char *data)
{
	memcpy(m_mac, mac, 6);
	strcpy(m_data, data);
}


const uint8_t* ICACHE_FLASH_ATTR EspWifi::EspNowMessage::from() const
{
	return m_mac;
}


const char* ICACHE_FLASH_ATTR EspWifi::EspNowMessage::data() const
{
	return m_data;
}



ICACHE_FLASH_ATTR EspWifi::UdpMessage::UdpMessage(char *data)
{
	strcpy(m_data, data);
}


const char* ICACHE_FLASH_ATTR EspWifi::UdpMessage::data() const
{
	return m_data;
}



EspWifi& EspWifi::getInstance(Mode nMode /*= Default */)
{
	// create and return the one instance of the class
	static EspWifi sWifi(nMode);
	return sWifi;
}

/* **************************************************************************************
   **********                                                                  **********
   **********           Global helper functions to call the callbacks          **********
   **********                                                                  **********   
   **************************************************************************************
*/

void ICACHE_FLASH_ATTR espNowSendCallback(uint8_t *mac_addr, uint8_t status)
{
  debug(">>> espNowSendCallback("MACSTR",%d)\n", MAC2STR(mac_addr), status);

  if (0 == status)
  {
	  EspWifi::getInstance().espNowMessageSent.emit(NULL);
  }
  else
  {
	  EspWifi::getInstance().espNowMessageSendFailed.emit(NULL);
  }

  int iTotalTime = system_get_time();
  print("Total time to send an ESP-now message: %d ms\n", iTotalTime/1000);


  debug("<<< espNowSendCallback()\n");
}


void ICACHE_FLASH_ATTR espNowRecvCallback(uint8_t *mac, uint8_t *data, uint8_t len)
{
  debug(">>> espNowRecvCallback("MACSTR", %s, %d)\n", MAC2STR(mac), data, len);

  EspWifi::getInstance().espNowMessageReceived.emit(new EspWifi::EspNowMessage(mac, (char*)data));

  debug("<<< espNowRecvCallback()\n");
}


void ICACHE_FLASH_ATTR wifiEventHandler(System_Event_t *evt)
{
	switch (evt->event) {
    case EVENT_STAMODE_CONNECTED:
        debug(">>> EspWifi::wifiEventHandler(EVENT_STAMODE_CONNECTED, ssid: %s, channel: %d)\n", evt->event_info.connected.ssid, evt->event_info.connected.channel);
        break;
    
    case EVENT_STAMODE_DISCONNECTED:
        debug(">>> EspWifi::wifiEventHandler(EVENT_STAMODE_DISCONNECTED) disconnect from ssid %s, reason %d\n", evt->event_info.disconnected.ssid,  evt->event_info.disconnected.reason);
        EspWifi::getInstance().disconnectedFromAP.emit(NULL);
        break;
    
    case EVENT_STAMODE_AUTHMODE_CHANGE:
        debug(">>> wifiEventHandler(EVENT_STAMODE_AUTHMODE_CHANGE) mode: %d -> %d\n", evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode);
        EspWifi::getInstance().disconnectedFromAP.emit(NULL);
        break;
    
    case EVENT_STAMODE_GOT_IP:
        debug(">>> EspWifi::wifiEventHandler(EVENT_STAMODE_GOT_IP, IP: " IPSTR ", Mask: " IPSTR ", Gateway: " IPSTR ")\n", IP2STR(&evt->event_info.got_ip.ip), IP2STR(&evt->event_info.got_ip.mask), IP2STR(&evt->event_info.got_ip.gw));
        EspWifi::getInstance().connectedToAP.emit(NULL);
        break;
    
    case EVENT_SOFTAPMODE_STACONNECTED:
        debug(">>> EspWifi::wifiEventHandler(EVENT_SOFTAPMODE_STACONNECTED) station: " MACSTR "join, AID = %d\n", MAC2STR(evt->event_info.sta_connected.mac), evt->event_info.sta_connected.aid);
        break;
    
    case EVENT_SOFTAPMODE_STADISCONNECTED:
        debug(">>> EspWifi::wifiEventHandler(EVENT_SOFTAPMODE_STADISCONNECTED) station: " MACSTR "leave, AID = %d\n",  MAC2STR(evt->event_info.sta_disconnected.mac), evt->event_info.sta_disconnected.aid);
        break;
    
    case EVENT_STAMODE_DHCP_TIMEOUT:
        debug(">>> EspWifi::wifiEventHandler(EVENT_STAMODE_DHCP_TIMEOUT)\n");
        EspWifi::getInstance().disconnectedFromAP.emit(NULL);
        break;
    
    case EVENT_SOFTAPMODE_PROBEREQRECVED:
        debug(">>> EspWifi::wifiEventHandler(EVENT_SOFTAPMODE_PROBEREQRECVED)\n");
        break;
    
    default:
        debug(">>> EspWifi::wifiEventHandler(0x%08x)\n", evt->event);
        EspWifi::getInstance().disconnectedFromAP.emit(NULL);
        break;
    }
    
    debug("<<< EspWifi::wifiEventHandler()\n");
}



void ICACHE_FLASH_ATTR wpsCallback(int status)
{
  bool bRet = false;
  switch (status) {
  
  case WPS_CB_ST_SUCCESS:
    debug(">>> wpsCallback(WPS_CB_ST_SUCCESS)\n");

    bRet = wifi_wps_disable();
    debug("    wifi_wps_disable() returns %s\n", bRet ? "true":"false");

    bRet = wifi_station_set_auto_connect(1);
	debug("    wifi_station_set_auto_connect(1) returns %s\n", bRet ? "true":"false");

	bRet = wifi_station_set_reconnect_policy(true);
	debug("    wifi_station_set_reconnect_policy(true) returns %s\n", bRet ? "true":"false");

	bRet = wifi_station_dhcpc_start();
	debug("    wifi_station_dhcpc_start() returns %s\n", bRet ? "true":"false");
     
    bRet = wifi_station_connect();
    debug("    wifi_station_connect() returns %s\n", bRet ? "true":"false");
    
    struct station_config sc;
    bRet = wifi_station_get_config_default(&sc);
    debug("    wifi_station_get_config_default() returns %s, SSID: %s, PASS: %s\n", bRet ? "true":"false", sc.ssid, sc.password);
    if (bRet)
    {
      debug("WPS config succeeded. SSID: %s, PASS: %s\n", sc.ssid, sc.password);
    }
    break;
     
  case WPS_CB_ST_FAILED:
  case WPS_CB_ST_TIMEOUT:
    debug(">>> wpsCallback(%d)\n", status);
    
    EspWifi::getInstance().wpsConfigFailed.emit(NULL);
      
    bRet = wifi_wps_disable();
    debug("    wifi_wps_disable() returns %s\n", bRet ? "true":"false");
      
    bRet = wifi_station_connect();
    debug("    wifi_station_connect() returns %s\n", bRet ? "true":"false");
      
    debug("WPS config failed\n");
    break;
    
  default:
    debug(">>> wpsCallback(%d)\n", status);
    
    EspWifi::getInstance().wpsConfigFailed.emit(NULL);
        
    bRet = wifi_wps_disable();
    debug("    wifi_wps_disable() returns %s\n", bRet ? "true":"false");
    
    bRet = wifi_station_connect();
    debug("    wifi_station_connect() returns %s\n", bRet ? "true":"false");
    
    debug("WPS config failed\n");
    break;
   }

   debug("<<< wpsCallback(%d)\n", status);
}



EspWifi::EspWifi(EspWifi::Mode nMode) : m_nMode(Off)
{
	debug(">>> EspWifi::EspWifi(%d)\n", static_cast<int>(nMode));

	bool bRet = false;
	int iRet  =-1;

	switch (nMode)
	{
	case Off:

		bRet = wifi_station_set_auto_connect(0);
		debug("    wifi_station_set_auto_connect(0) returns %s\n", bRet ? "true":"false");

		bRet = wifi_station_set_reconnect_policy(false);
		debug("    wifi_station_set_reconnect_policy(false) returns %s\n", bRet ? "true":"false");

		bRet = wifi_station_dhcpc_stop();
		debug("    wifi_station_dhcpc_stop() returns %s\n", bRet ? "true":"false");

		bRet = wifi_station_disconnect();
        debug("    wifi_station_disconnect() returns %s\n", bRet ? "true":"false");

        m_nMode = Off;

        break;

	case ConnectToAP:

		wifi_set_event_handler_cb((wifi_event_handler_cb_t)wifiEventHandler);

		break;

	case SendEspNow:

		  bRet = wifi_station_disconnect();
		  debug("    wifi_station_disconnect() returns %s\n", bRet ? "true":"false");

		  bRet =  wifi_set_opmode(STATION_MODE);
		  debug("    wifi_set_opmode(STATION_MODE) returns %s\n", bRet ? "true":"false");

		  bRet = wifi_station_set_auto_connect(0);
		  debug("    wifi_station_set_auto_connect(0) returns %s\n", bRet ? "true":"false");

		  iRet = esp_now_init();
		  debug("    esp_now_init() returns %d\n", iRet);

		  if (0 == iRet)
		  {
			  iRet = esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
			  debug("    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER) returns %d\n", iRet);
		  }

		  if (0 == iRet)
		  {
			  iRet = esp_now_add_peer(ESP_NOW_GATEWAY_MAC, ESP_NOW_ROLE_SLAVE, ESP_NOW_WIFI_CHANNEL, NULL, 0);
			  debug("    esp_now_add_peer(ESP_NOW_GATEWAY_MAC, ESP_NOW_ROLE_SLAVE) returns %d\n", iRet);
		  }

		  if (0 == iRet)
		  {
			  esp_now_register_send_cb((esp_now_send_cb_t)espNowSendCallback);
			  m_nMode = SendEspNow;
		  }

		break;

	case ReceiveEspNow:

		bRet = wifi_station_disconnect();
		debug("    wifi_station_disconnect() returns %s\n", bRet ? "true":"false");

		bRet = wifi_set_opmode(SOFTAP_MODE);
		debug("    wifi_set_opmode(SOFTAP_MODE) returns %s\n", bRet ? "true":"false");

		bRet = wifi_set_macaddr(SOFTAP_IF, ESP_NOW_GATEWAY_MAC);
		debug("    wifi_set_macaddr(SOFTAP_IF, ESP_NOW_GATEWAY_MAC) returns %s\n", bRet ? "true":"false");

		iRet = esp_now_init();
		debug("    esp_now_init() returns %d\n", iRet);

		if (0 == iRet)
		{
			iRet = esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
			debug("    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE) returns %d\n", iRet);
		}

		if (0 == iRet)
		{
			iRet = esp_now_register_recv_cb((esp_now_recv_cb_t)espNowRecvCallback);
			m_nMode = ReceiveEspNow;
		}

		break;

	case Default:

		break;
	}

	debug("<<< EspWifi::EspWifi()\n");
}


void ICACHE_FLASH_ATTR EspWifi::printInfo()
{
  char str[30];
  uint8 macaddr[6];
  bool bSucceeded = wifi_get_macaddr(STATION_IF, macaddr);
  if (bSucceeded)
  {
      print("    wifi_get_macaddr(STATION_IF, " MACSTR ") returns %s\n", MAC2STR(macaddr), bSucceeded ? "true":"false");
  }
  else
  {
      print("    wifi_get_macaddr(STATION_IF, ERROR) returns %s\n", bSucceeded ? "true":"false");
  }
  
  bSucceeded = wifi_get_macaddr(SOFTAP_IF, macaddr);
  if (bSucceeded)
  {
      print("    wifi_get_macaddr(SOFTAP_IF, " MACSTR ") returns %s\n", MAC2STR(macaddr), bSucceeded ? "true":"false");
  }
  else
  {
      print("    wifi_get_macaddr(SOFTAP_IF, ERROR) returns %s\n", bSucceeded ? "true":"false");
  }

  int iCurrentOpMode = wifi_get_opmode_default();
  print("    wifi_get_opmode_default() returns %d\n", iCurrentOpMode);
  
  int iAutoConnect = wifi_station_get_auto_connect();
  print("    wifi_station_get_auto_connect() returns %d\n", iAutoConnect);

  
  struct station_config sc;
  bool bRet = wifi_station_get_config_default(&sc);
  print("    wifi_station_get_config_default() returns %s\n", bRet ? "true":"false");
  if (bRet)
  {
    print("       SSID: %s\n", sc.ssid);
    print("       PASS: %s\n", sc.password);
  }
}


int ICACHE_FLASH_ATTR EspWifi::espNowSend(const char* data, int length)
{
    debug(">>> EspWifi::espNowSend(%s, %d)\n", data, length);
    
    int iRet = esp_now_send(ESP_NOW_GATEWAY_MAC, (unsigned char*)data, length);

    debug("<<< EspWifi::espNowSend() returns %d\n", iRet);
    
    return iRet;
}


bool ICACHE_FLASH_ATTR EspWifi::factoryReset() const
{
  debug("%p >>> EspWifi::factoryReset()\n", this);
  
  bool bRet = wifi_set_opmode(STATION_MODE);
  debug("    wifi_set_opmode(STATION_MODE) returns %s\n", bRet ? "true":"false");
  
  struct station_config stationConf;
  stationConf.bssid_set = 0; 
  os_memset(&stationConf.ssid, 0, 32);
  os_memset(&stationConf.password, 0, 64);
  os_memcpy(&stationConf.ssid, "", os_strlen(""));
  os_memcpy(&stationConf.password, "", os_strlen(""));
  bRet = wifi_station_set_config(&stationConf);
  debug("    wifi_station_set_config() returns %s\n", bRet ? "true":"false");
  
  bRet = wifi_station_set_auto_connect(1);
  debug("    wifi_station_set_auto_connect(1) returns %s\n", bRet ? "true":"false");
  
  bRet = wifi_station_dhcpc_start();
  debug("    wifi_station_dhcpc_start() returns %s\n", bRet ? "true":"false");
  
  debug("%p <<< EspWifi::factoryReset()\n", this);
}



void ICACHE_FLASH_ATTR EspWifi::startWpsConfig()
{
  debug("%p >>> EspWifi::startWpsConfig()\n", this);

  if (STATION_MODE != wifi_get_opmode())
  {
    bool bRet = wifi_set_opmode(STATION_MODE);
    debug("    wifi_set_opmode(STATION_MODE) returns %s\n", bRet ? "true":"false");
  }
  
  bool bRet = wifi_wps_disable();
  debug("    wifi_wps_disable() returns %s\n", bRet ? "true":"false");
  
  if (bRet)
  {
    bRet = wifi_wps_enable(WPS_TYPE_PBC);
    debug("    wifi_wps_enable(WPS_TYPE_PBC) returns %s\n", bRet ? "true":"false");
  }
  
  if (bRet)
  {
    bRet = wifi_set_wps_cb((wps_st_cb_t)wpsCallback);
    debug("    wifi_set_wps_cb() returns %s\n", bRet ? "true":"false");
  }
  else
  {
    bool bRet = wifi_wps_disable();
    debug("    wifi_wps_disable() returns %s\n", bRet ? "true":"false");
  }
  
  if (bRet)
  {
    bRet = wifi_wps_start();
    debug("    wifi_wps_start() returns %s\n", bRet ? "true":"false");
  }
  else
  {
    bool bRet = wifi_wps_disable();
    debug("    wifi_wps_disable() returns %s\n", bRet ? "true":"false");
  }
  
  if (bRet)
  {
	  EspWifi::getInstance().wpsConfigStarted.emit(NULL);
  }
  else
  {
	  EspWifi::getInstance().wpsConfigFailed.emit(NULL);
  }
    
  debug("%p <<< EspWifi::startWpsConfig()\n", this);
}


void ICACHE_FLASH_ATTR user_udp_recv(void *arg, char *pusrdata, unsigned short length)
{
	EspWifi::getInstance().udpMessageReceived.emit(new EspWifi::UdpMessage(pusrdata));
}


void ICACHE_FLASH_ATTR EspWifi::listenForUdpMessages(bool bListen)
{
  if (bListen)
  {
    m_espconnUDP.type = ESPCONN_UDP;
    m_espconnUDP.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
    m_espconnUDP.proto.udp->local_port = 1025;
    espconn_regist_recvcb(&m_espconnUDP, (espconn_recv_callback)user_udp_recv);
    espconn_create(&m_espconnUDP);
  }
  else
  {
    espconn_delete(&m_espconnUDP);
  }
}

