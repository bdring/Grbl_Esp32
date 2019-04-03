/*
  wificonfig.h -  wifi functions class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

//Preferences entries
#define HOSTNAME_ENTRY "ESP_HOSTNAME"
#define STA_SSID_ENTRY "STA_SSID"
#define STA_PWD_ENTRY "STA_PWD"
#define STA_IP_ENTRY "STA_IP"
#define STA_GW_ENTRY "STA_GW"
#define STA_MK_ENTRY "STA_MK"
#define AP_SSID_ENTRY "AP_SSID"
#define AP_PWD_ENTRY "AP_PWD"
#define AP_IP_ENTRY "AP_IP"
#define AP_CHANNEL_ENTRY "AP_CHANNEL"
#define HTTP_ENABLE_ENTRY "HTTP_ON"
#define HTTP_PORT_ENTRY "HTTP_PORT"
#define TELNET_ENABLE_ENTRY "TELNET_ON"
#define TELNET_PORT_ENTRY "TELNET_PORT"
#define STA_IP_MODE_ENTRY "STA_IP_MODE"
#define NOTIFICATION_TYPE "NOTIF_TYPE"
#define NOTIFICATION_T1 "NOTIF_T1"
#define NOTIFICATION_T2 "NOTIF_T2"
#define NOTIFICATION_TS "NOTIF_TS"

//Notifications
#define ESP_PUSHOVER_NOTIFICATION	1
#define ESP_EMAIL_NOTIFICATION		2
#define ESP_LINE_NOTIFICATION		3

#define DHCP_MODE   0
#define STATIC_MODE   1

//Switch 
#define ESP_SAVE_ONLY 0
#define ESP_APPLY_NOW 1

//defaults values
#define DEFAULT_HOSTNAME "grblesp"
#define DEFAULT_STA_SSID "GRBL_ESP"
#define DEFAULT_STA_PWD "12345678"
#define DEFAULT_STA_IP "0.0.0.0"
#define DEFAULT_STA_GW "0.0.0.0"
#define DEFAULT_STA_MK "0.0.0.0"
#define DEFAULT_AP_SSID "GRBL_ESP"
#define DEFAULT_AP_PWD "12345678"
#define DEFAULT_AP_IP "192.168.0.1"
#define DEFAULT_AP_MK "255.255.255.0"
#define DEFAULT_AP_CHANNEL 1
#define DEFAULT_WEBSERVER_PORT 80
#define DEFAULT_HTTP_STATE 1
#define DEFAULT_TELNETSERVER_PORT 23
#define DEFAULT_TELNET_STATE 1
#define DEFAULT_STA_IP_MODE DHCP_MODE
#define HIDDEN_PASSWORD "********"
#define DEFAULT_TOKEN ""
#define DEFAULT_NOTIFICATION_TYPE 0

//boundaries
#define MAX_SSID_LENGTH         32
#define MIN_SSID_LENGTH         1
#define MAX_PASSWORD_LENGTH     64
//min size of password is 0 or upper than 8 char
//so let set min is 8
#define MIN_PASSWORD_LENGTH     8
#define MAX_HOSTNAME_LENGTH     32
#define MIN_HOSTNAME_LENGTH     1
#define MAX_HTTP_PORT			65001
#define MIN_HTTP_PORT			1
#define MAX_TELNET_PORT			65001
#define MIN_TELNET_PORT			1
#define MIN_CHANNEL			1
#define MAX_CHANNEL			14
#define MIN_NOTIFICATION_TOKEN_LENGTH	0
#define MAX_NOTIFICATION_TOKEN_LENGTH	63
#define MAX_NOTIFICATION_SETTING_LENGTH	127

#ifndef _WIFI_CONFIG_H
#define _WIFI_CONFIG_H
#include "WiFi.h"

class WiFiConfig {
public:
    WiFiConfig();
    ~WiFiConfig();
    static const char *info();
    static bool isValidIP(const char * string);
    static bool isPasswordValid (const char * password);
    static bool isSSIDValid (const char * ssid);
    static bool isHostnameValid (const char * hostname);
    static uint32_t IP_int_from_string(String & s);
    static String IP_string_from_int(uint32_t ip_int);
    static String Hostname(){return _hostname;}
    static char * mac2str (uint8_t mac [8]);
    static bool StartAP();
    static bool StartSTA();
    static void StopWiFi();
    static int32_t getSignal (int32_t RSSI);
    static void begin();
    static void end();
    static void handle();
    static void reset_settings();
    static bool Is_WiFi_on();
private :
    static bool ConnectSTA2AP();
    static void WiFiEvent(WiFiEvent_t event);
    static String _hostname;
    static bool _events_registered;
};

extern WiFiConfig wifi_config;

#endif
