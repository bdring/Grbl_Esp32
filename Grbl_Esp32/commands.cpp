/*
  commands.cpp - GRBL_ESP command class

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
#include "config.h"
#include "commands.h"
#include "espresponse.h"
#include <Preferences.h>
#include "report.h"
#ifdef ENABLE_SD_CARD
#include "grbl_sd.h"
#endif
#ifdef ENABLE_BLUETOOTH
#include "BTconfig.h"
#endif
#ifdef ENABLE_WIFI
#include "wificonfig.h"
#if defined (ENABLE_HTTP)
#include "web_server.h"
#endif
#ifdef ENABLE_TELNET
#include "telnet_server.h"
#endif
#endif
#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <esp_wifi.h>

#ifdef __cplusplus
extern "C" {
#endif
esp_err_t esp_task_wdt_reset();
#ifdef __cplusplus
}
#endif

bool COMMANDS::restart_ESP_module = false;

/*
 * delay is to avoid with asyncwebserver and may need to wait sometimes
 */
void COMMANDS::wait(uint32_t milliseconds){
    uint32_t timeout = millis();
    esp_task_wdt_reset(); //for a wait 0;
    //wait feeding WDT
    while ( (millis() - timeout) < milliseconds) {
       esp_task_wdt_reset();
    }
}

bool COMMANDS::execute_internal_command (int cmd, String cmd_params, level_authenticate_type auth_level,  ESPResponseStream  *espresponse)
{
    bool response = true;
    level_authenticate_type auth_type = auth_level;
    if (!espresponse) return false;
#ifdef ENABLE_AUTHENTICATION
    
    if (isadmin(cmd_params)) {
        auth_type = LEVEL_ADMIN;
    }
    if (isuser (cmd_params) && (auth_type != LEVEL_ADMIN) ) {
        auth_type = LEVEL_USER;
    }
#endif
    //manage parameters
    String parameter;
    switch (cmd) {
#ifdef ENABLE_WIFI
        //STA SSID
        //[ESP100]<SSID>[pwd=<admin password>]
        case 100:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            parameter = get_param (cmd_params, "", true);
            if (!WiFiConfig::isSSIDValid (parameter.c_str() ) ) {
                if(espresponse)espresponse->println ("Error: Incorrect SSID!");
                response = false;
                return false;
            } else {
                Preferences prefs;
                prefs.begin(NAMESPACE, false);
                if (prefs.putString(STA_SSID_ENTRY, parameter) == 0){
                    response = false;
                    if(espresponse)espresponse->println ("Error: Set failed!");
                    } else if(espresponse)espresponse->println ("ok");
                prefs.end();
                }
            }
            break;
        //STA Password
        //[ESP101]<Password>[pwd=<admin password>]
        case 101:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            parameter = get_param (cmd_params, "", true);
            if (!WiFiConfig::isPasswordValid (parameter.c_str() ) ) {
                if(espresponse)espresponse->println ("Error: Incorrect password!");
                response = false;
                return false;
            } else {
                Preferences prefs;
                prefs.begin(NAMESPACE, false);
                if (prefs.putString(STA_PWD_ENTRY, parameter) == 0){
                    response = false;
                    if(espresponse)espresponse->println ("Error: Set failed!");
                    } else if(espresponse)espresponse->println ("ok");
                prefs.end();
                }
            }
            break;
        //Change STA IP mode (DHCP/STATIC) 
        //[ESP102]<mode>pwd=<admin password>
        case 102:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            parameter = get_param (cmd_params, "", true);
            parameter.toUpperCase();
            if (!((parameter == "STATIC") || (parameter == "DHCP"))) {
                if(espresponse)espresponse->println ("Error: only STATIC or DHCP mode supported!");
                response = false;
                return false;
            } else {
                Preferences prefs;
                prefs.begin(NAMESPACE, false);
                int8_t bbuf = (parameter == "DHCP")?DHCP_MODE:STATIC_MODE;
                if (prefs.putChar(STA_IP_MODE_ENTRY, bbuf) == 0){
                    response = false;
                    if(espresponse)espresponse->println ("Error: Set failed!");
                    } else if(espresponse)espresponse->println ("ok");
                prefs.end();
                }
            }
            break;
        //Change STA IP/Mask/GW 
        //[ESP103]IP=<IP> MSK=<IP> GW=<IP> pwd=<admin password>
        case 103:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            String IP = get_param (cmd_params, "IP=", true);
            String GW = get_param (cmd_params, "GW=", true);
            String MSK = get_param (cmd_params, "MSK=", true);
            if ( !WiFiConfig::isValidIP(IP.c_str())) {
                if(espresponse)espresponse->println ("Error: Incorrect IP!");
                response = false;
                return false;
            }
            if ( !WiFiConfig::isValidIP(GW.c_str())) {
                if(espresponse)espresponse->println ("Error: Incorrect Gateway!");
                response = false;
                return false;
            }
            if ( !WiFiConfig::isValidIP(MSK.c_str())) {
                if(espresponse)espresponse->println ("Error: Incorrect Mask!");
                response = false;
                return false;
            }  
            Preferences prefs;
            prefs.begin(NAMESPACE, false);
            if ((prefs.putInt(STA_IP_ENTRY, wifi_config.IP_int_from_string(IP)) == 0) || 
            (prefs.putInt(STA_IP_ENTRY, wifi_config.IP_int_from_string(GW)) == 0) ||
            (prefs.putInt(STA_IP_ENTRY, wifi_config.IP_int_from_string(MSK)) == 0)){
                response = false;
                if(espresponse)espresponse->println ("Error: Set failed!");
                } else if(espresponse)espresponse->println ("ok");
            prefs.end();
            }
            break;

        //Change AP SSID 
        //[ESP105]<SSID>pwd=<admin password>
        case 105:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            parameter = get_param (cmd_params, "", true);
            if (!WiFiConfig::isSSIDValid (parameter.c_str() ) ) {
                if(espresponse)espresponse->println ("Error: Incorrect SSID!");
                response = false;
                return false;
            }
            Preferences prefs;
            prefs.begin(NAMESPACE, false);
            if (prefs.putString(AP_SSID_ENTRY, parameter) == 0){
                response = false;
                if(espresponse)espresponse->println ("Error: Set failed!");
                } else if(espresponse)espresponse->println ("ok");
            prefs.end();
                
            }
            break;
        //Change AP Password 
        //[ESP106]<Password>pwd=<admin password>
        case 106:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            parameter = get_param (cmd_params, "", true);
            if (!WiFiConfig::isPasswordValid (parameter.c_str() ) ) {
                if(espresponse)espresponse->println ("Error: Incorrect password!");
                response = false;
                return false;
            } 
            Preferences prefs;
            prefs.begin(NAMESPACE, false);
            if (prefs.putString(AP_PWD_ENTRY, parameter) == 0){
                response = false;
                if(espresponse)espresponse->println ("Error: Set failed!");
                } else if(espresponse)espresponse->println ("ok");
            prefs.end();
                
            }
            break;
        //Change AP IP 
        //[ESP107]<IP>pwd=<admin password>
        case 107:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            parameter = get_param (cmd_params, "", true);
            if ( !WiFiConfig::isValidIP(parameter.c_str())) {
                if(espresponse)espresponse->println ("Error: Incorrect IP!");
                response = false;
                return false;
            }
            Preferences prefs;
            prefs.begin(NAMESPACE, false);
            if (prefs.putInt(AP_IP_ENTRY, wifi_config.IP_int_from_string(parameter)) == 0){
                response = false;
                if(espresponse)espresponse->println ("Error: Set failed!");
                } else if(espresponse)espresponse->println ("ok");
            prefs.end();
            }
            break;
        //Change AP channel 
        //[ESP108]<channel>pwd=<admin password>
        case 108:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            parameter = get_param (cmd_params, "", true);
            int8_t bbuf = parameter.toInt();
            if ((bbuf > MAX_CHANNEL) || (bbuf < MIN_CHANNEL)) {
                if(espresponse)espresponse->println ("Error: Incorrect channel!");
                response = false;
                return false;
            }
            Preferences prefs;
            prefs.begin(NAMESPACE, false);
            
            if (prefs.putChar(AP_CHANNEL_ENTRY, bbuf) == 0){
                response = false;
                if(espresponse)espresponse->println ("Error: Set failed!");
                } else if(espresponse)espresponse->println ("ok");
            prefs.end();
                
            }
            break;
#endif
#if defined( ENABLE_WIFI) ||  defined( ENABLE_BLUETOOTH)
        //Set radio state at boot which can be BT, STA, AP, OFF
        //[ESP110]<state>pwd=<admin password>
        case 110:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            parameter = get_param (cmd_params, "", true);
            parameter.toUpperCase();
            if (!(
#if defined( ENABLE_BLUETOOTH)
            (parameter == "BT") ||
#endif      
#if defined( ENABLE_WIFI) 
            (parameter == "STA") || (parameter == "AP") || 
#endif
            (parameter == "OFF"))) {
                
                if(espresponse)espresponse->println ("Error: only "
#ifdef ENABLE_BLUETOOTH
                "BT or "
#endif
#ifdef ENABLE_WIFI
                "STA or AP or "
#endif
                "OFF mode supported!");
                response = false;
                return false;
            } else {
                Preferences prefs;
                prefs.begin(NAMESPACE, false);
                int8_t bbuf = ESP_RADIO_OFF;
#ifdef ENABLE_WIFI
                if(parameter == "STA")bbuf = ESP_WIFI_STA;
                if(parameter == "AP")bbuf = ESP_WIFI_AP;
#endif
#ifdef ENABLE_BLUETOOTH
                if(parameter == "BT")bbuf = ESP_BT;
#endif
                if (prefs.putChar(ESP_RADIO_MODE, bbuf) == 0){
                    response = false;
                    if(espresponse)espresponse->println ("Error: Set failed!");
                    } else if(espresponse)espresponse->println ("ok");
                prefs.end();
                }
            }
            break;
#endif
#ifdef ENABLE_WIFI
        //Get current IP
        //[ESP111]<header answer>
        case 111: 
            {
            if (!espresponse) return false;
            String currentIP = cmd_params;
            if (WiFi.getMode() == WIFI_STA) {
                currentIP += WiFi.localIP().toString();
            } else {
                currentIP += WiFi.softAPIP().toString();
                }
            espresponse->println (currentIP.c_str());
            }
            break;

        //Get/Set hostname
        //[ESP112]<Hostname> pwd=<admin password>
        case 112: {
#ifdef ENABLE_AUTHENTICATION
        if (auth_type == LEVEL_GUEST) {
             espresponse->println ("Error: Wrong authentication!");
            return false;
        } 
#endif
        parameter = get_param (cmd_params, "", true);
        if ((parameter.length() == 0) && !espresponse) return false;
#ifdef ENABLE_AUTHENTICATION
        if ((auth_type != LEVEL_ADMIN) && (parameter.length() > 0)) {
             espresponse->println ("Error: Wrong authentication!");
             return false;
        } 
#endif
            //Get hostname
            if (parameter.length() == 0) {
                String shost = cmd_params;
                shost=wifi_config.Hostname();
                espresponse->println (shost.c_str());
            } else { //set host name
                if (!wifi_config.isHostnameValid (parameter.c_str() ) ) {
                if(espresponse)espresponse->println ("Error: Incorrect hostname!");
                response = false;
                } else {
                Preferences prefs;
                prefs.begin(NAMESPACE, false);
                if (prefs.putString(HOSTNAME_ENTRY, parameter) == 0){
                    response = false;
                    if(espresponse)espresponse->println ("Error: Set failed!");
                    } else if(espresponse)espresponse->println ("ok");
                prefs.end();
                }
            }
        }
        break;
        
        //Set immediate wifi state which can be ON, OFF
        //[ESP115]<state>pwd=<admin password>
        case 115:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            parameter = get_param (cmd_params, "", true);
            parameter.toUpperCase();
            if (!((parameter == "ON") || (parameter == "OFF"))) {
                if(espresponse)espresponse->println ("Error: only ON or OFF mode supported!");
                return false;
            } else {
                if (parameter == "OFF") {
                   wifi_config.StopWiFi();
                } else { //On
                    wifi_config.begin();
                }
            }
            }
            break;
            
        //Set HTTP state which can be ON, OFF
        //[ESP120]<state>pwd=<admin password>
        case 120:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            parameter = get_param (cmd_params, "", true);
            parameter.toUpperCase();
            if (!((parameter == "ON") || (parameter == "OFF"))) {
                if(espresponse)espresponse->println ("Error: only ON or OFF mode supported!");
                response = false;
                return false;
            } else {
                Preferences prefs;
                prefs.begin(NAMESPACE, false);
                int8_t bbuf = (parameter == "ON")?1:0;
                if (prefs.putChar(HTTP_ENABLE_ENTRY, bbuf) == 0){
                    response = false;
                    if(espresponse)espresponse->println ("Error: Set failed!");
                    } else if(espresponse)espresponse->println ("ok");
                prefs.end();
                }
            }
        break;
        //Set HTTP port 
        //[ESP121]<port>pwd=<admin password>
        case 121:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            parameter = get_param (cmd_params, "", true);
            int ibuf = parameter.toInt();
            if ((ibuf > MAX_HTTP_PORT) || (ibuf < MIN_HTTP_PORT)) {
                if(espresponse)espresponse->println ("Error: Incorrect port!");
                response = false;
                return false;
            }
            Preferences prefs;
            prefs.begin(NAMESPACE, false);
            
            if (prefs.putUShort(HTTP_PORT_ENTRY, ibuf) == 0){
                response = false;
                if(espresponse)espresponse->println ("Error: Set failed!");
                } else if(espresponse)espresponse->println ("ok");
            prefs.end();
                
            }
            break;
        //Set Telnet state which can be ON, OFF
        //[ESP130]<state>pwd=<admin password>
        case 130:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            parameter = get_param (cmd_params, "", true);
            parameter.toUpperCase();
            if (!((parameter == "ON") || (parameter == "OFF"))) {
                if(espresponse)espresponse->println ("Error: only ON or OFF mode supported!");
                response = false;
                return false;
            } else {
                Preferences prefs;
                prefs.begin(NAMESPACE, false);
                int8_t bbuf = (parameter == "ON")?1:0;
                if (prefs.putChar(TELNET_ENABLE_ENTRY, bbuf) == 0){
                    response = false;
                    if(espresponse)espresponse->println ("Error: Set failed!");
                    } else if(espresponse)espresponse->println ("ok");
                prefs.end();
                }
            }
        break;
        //Set Telnet port 
        //[ESP131]<port>pwd=<admin password>
        case 131:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                 espresponse->println ("Error: Wrong authentication!");
                 return false;
            } 
#endif
            parameter = get_param (cmd_params, "", true);
            int ibuf = parameter.toInt();
            if ((ibuf > MAX_TELNET_PORT) || (ibuf < MIN_TELNET_PORT)) {
                if(espresponse)espresponse->println ("Error: Incorrect port!");
                response = false;
                return false;
            }
            Preferences prefs;
            prefs.begin(NAMESPACE, false);
            
            if (prefs.putUShort(TELNET_PORT_ENTRY, ibuf) == 0){
                response = false;
                if(espresponse)espresponse->println ("Error: Set failed!");
                } else if(espresponse)espresponse->println ("ok");
            prefs.end();
                
            }
            break;
#endif

#ifdef ENABLE_BLUETOOTH
        //Get/Set btname
        //[ESP140]< Bluetooth name> pwd=<admin password>
        case 140: {
#ifdef ENABLE_AUTHENTICATION
        if (auth_type == LEVEL_GUEST) {
             espresponse->println ("Error: Wrong authentication!");
            return false;
        } 
#endif
        parameter = get_param (cmd_params, "", true);
        if ((parameter.length() == 0) && !espresponse) return false;
#ifdef ENABLE_AUTHENTICATION
        if ((auth_type != LEVEL_ADMIN) && (parameter.length() > 0)) {
             espresponse->println ("Error: Wrong authentication!");
             return false;
        } 
#endif
            //Get btname
            if (parameter.length() == 0) {
                String shost = cmd_params;
                shost=bt_config.BTname();
                espresponse->println (shost.c_str());
            } else { //set BT name
                if (!bt_config.isBTnameValid (parameter.c_str() ) ) {
                if(espresponse)espresponse->println ("Error: Incorrect name!");
                response = false;
                } else {
                Preferences prefs;
                prefs.begin(NAMESPACE, false);
                if (prefs.putString(BT_NAME_ENTRY, parameter) == 0){
                    response = false;
                    if(espresponse)espresponse->println ("Error: Set failed!");
                    } else if(espresponse)espresponse->println ("ok");
                prefs.end();
                }
            }
        }
        break;
#endif
        //Get SD Card Status
        //[ESP200]
        case 200:
            {
            if (!espresponse) return false;
            String resp = "No SD card";
#ifdef ENABLE_SD_CARD

            int8_t state = get_sd_state(true);
            if (state == SDCARD_IDLE)resp="SD card detected";
            else if (state == SDCARD_NOT_PRESENT)resp="No SD card";
            else resp="Busy";
#endif
            espresponse->println (resp.c_str());
            }
            break;
        //Get full ESP32  settings content
        //[ESP400]
        case 400:
            { 
            String v;
            String defV;
            Preferences prefs;
            if (!espresponse) return false;
#ifdef ENABLE_AUTHENTICATION
            if (auth_type == LEVEL_GUEST) {
                espresponse->println ("Error: Wrong authentication!");
                return false;
            }
#endif
            int8_t vi;
            espresponse->print("{\"EEPROM\":[");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
            prefs.begin(NAMESPACE, true);
#ifdef ENABLE_WIFI
            //1 - Hostname
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (HOSTNAME_ENTRY);
            espresponse->print ("\",\"T\":\"S\",\"V\":\"");
            espresponse->print (wifi_config.Hostname().c_str());
            espresponse->print ("\",\"H\":\"Hostname\" ,\"S\":\"");
            espresponse->print (String(MAX_HOSTNAME_LENGTH).c_str());
            espresponse->print ("\", \"M\":\"");
            espresponse->print (String(MIN_HOSTNAME_LENGTH).c_str());
            espresponse->print ("\"}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
#ifdef ENABLE_HTTP
            //2 - http protocol mode
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (HTTP_ENABLE_ENTRY);
            espresponse->print ("\",\"T\":\"B\",\"V\":\"");
            vi = prefs.getChar(HTTP_ENABLE_ENTRY, 1);
            espresponse->print (String(vi).c_str());
            espresponse->print ("\",\"H\":\"HTTP protocol\",\"O\":[{\"Enabled\":\"1\"},{\"Disabled\":\"0\"}]}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
            //3 - http port
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (HTTP_PORT_ENTRY);
            espresponse->print ("\",\"T\":\"I\",\"V\":\"");
            espresponse->print (String(web_server.port()).c_str());
            espresponse->print ("\",\"H\":\"HTTP Port\",\"S\":\"");
            espresponse->print (String(MAX_HTTP_PORT).c_str());
            espresponse->print ("\",\"M\":\"");
            espresponse->print (String(MIN_HTTP_PORT).c_str());
            espresponse->print ("\"}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
#endif
            
#ifdef ENABLE_TELNET
            //4 - telnet protocol mode
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (TELNET_ENABLE_ENTRY);
            espresponse->print ("\",\"T\":\"B\",\"V\":\"");
            vi = prefs.getChar(TELNET_ENABLE_ENTRY, 0);
            espresponse->print (String(vi).c_str());
            espresponse->print ("\",\"H\":\"Telnet protocol\",\"O\":[{\"Enabled\":\"1\"},{\"Disabled\":\"0\"}]}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
            //5 - telnet Port
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (TELNET_PORT_ENTRY);
            espresponse->print ("\",\"T\":\"I\",\"V\":\"");
            espresponse->print (String(telnet_server.port()).c_str());
            espresponse->print ("\",\"H\":\"Telnet Port\",\"S\":\"");
            espresponse->print (String(MAX_TELNET_PORT).c_str());
            espresponse->print ("\",\"M\":\"");
            espresponse->print (String(MIN_TELNET_PORT).c_str());
            espresponse->print ("\"}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
#endif          
            //6 - radio mode
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (ESP_RADIO_MODE);
            espresponse->print ("\",\"T\":\"B\",\"V\":\"");
            vi = prefs.getChar(ESP_RADIO_MODE, ESP_RADIO_OFF);
            espresponse->print (String(vi).c_str());
            espresponse->print ("\",\"H\":\"Radio mode\",\"O\":[{\"None\":\"0\"}");
#ifdef ENABLE_WIFI
            espresponse->print (",{\"STA\":\"1\"},{\"AP\":\"2\"}");
#endif
#ifdef ENABLE_BLUETOOTH
            espresponse->print (",{\"BT\":\"3\"}");
#endif
            espresponse->print ("]}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");

            //7 - STA SSID
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (STA_SSID_ENTRY);
            espresponse->print ("\",\"T\":\"S\",\"V\":\"");
            defV = DEFAULT_STA_SSID;
            espresponse->print (prefs.getString(STA_SSID_ENTRY, defV).c_str());
            espresponse->print ("\",\"S\":\"");
            espresponse->print (String(MAX_SSID_LENGTH).c_str());
            espresponse->print ("\",\"H\":\"Station SSID\",\"M\":\"");
            espresponse->print (String(MIN_SSID_LENGTH).c_str());
            espresponse->print ("\"}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");

            //8 - STA password
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (STA_PWD_ENTRY);
            espresponse->print ("\",\"T\":\"S\",\"V\":\"");
            espresponse->print (HIDDEN_PASSWORD);
            espresponse->print ("\",\"S\":\"");
            espresponse->print (String(MAX_PASSWORD_LENGTH).c_str());
            espresponse->print ("\",\"H\":\"Station Password\",\"M\":\"");
            espresponse->print (String(MIN_PASSWORD_LENGTH).c_str());
            espresponse->print ("\"}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
            
            // 9 - STA IP mode
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (STA_IP_MODE_ENTRY);
            espresponse->print ("\",\"T\":\"B\",\"V\":\"");
            espresponse->print (String(prefs.getChar(STA_IP_MODE_ENTRY, DHCP_MODE)).c_str());
            espresponse->print ("\",\"H\":\"Station IP Mode\",\"O\":[{\"DHCP\":\"0\"},{\"Static\":\"1\"}]}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");

            //10-STA static IP
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (STA_IP_ENTRY);
            espresponse->print ("\",\"T\":\"A\",\"V\":\"");
            espresponse->print (wifi_config.IP_string_from_int(prefs.getInt(STA_IP_ENTRY, 0)).c_str());
            espresponse->print ("\",\"H\":\"Station Static IP\"}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");

            //11-STA static Gateway
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (STA_GW_ENTRY);
            espresponse->print ("\",\"T\":\"A\",\"V\":\"");
            espresponse->print (wifi_config.IP_string_from_int(prefs.getInt(STA_GW_ENTRY, 0)).c_str());
            espresponse->print ("\",\"H\":\"Station Static Gateway\"}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");

            //12-STA static Mask
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (STA_MK_ENTRY);
            espresponse->print ("\",\"T\":\"A\",\"V\":\"");
            espresponse->print (wifi_config.IP_string_from_int(prefs.getInt(STA_MK_ENTRY, 0)).c_str());
            espresponse->print ("\",\"H\":\"Station Static Mask\"}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
            
            //13 - AP SSID
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (AP_SSID_ENTRY);
            espresponse->print ("\",\"T\":\"S\",\"V\":\"");
            defV = DEFAULT_AP_SSID;
            espresponse->print (prefs.getString(AP_SSID_ENTRY, defV).c_str());
            espresponse->print ("\",\"S\":\"");
            espresponse->print (String(MAX_SSID_LENGTH).c_str());
            espresponse->print ("\",\"H\":\"AP SSID\",\"M\":\"");
            espresponse->print (String(MIN_SSID_LENGTH).c_str());
            espresponse->print ("\"}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");

            //14 - AP password
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (AP_PWD_ENTRY);
            espresponse->print ("\",\"T\":\"S\",\"V\":\"");
            espresponse->print (HIDDEN_PASSWORD);
            espresponse->print ("\",\"S\":\"");
            espresponse->print (String(MAX_PASSWORD_LENGTH).c_str());
            espresponse->print ("\",\"H\":\"AP Password\",\"M\":\"");
            espresponse->print (String(MIN_PASSWORD_LENGTH).c_str());
            espresponse->print ("\"}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
            
            //15 - AP static IP
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (AP_IP_ENTRY);
            espresponse->print ("\",\"T\":\"A\",\"V\":\"");
            defV = DEFAULT_AP_IP;
            espresponse->print (wifi_config.IP_string_from_int(prefs.getInt(AP_IP_ENTRY, wifi_config.IP_int_from_string(defV))).c_str());
            espresponse->print ("\",\"H\":\"AP Static IP\"}");
            espresponse->print (",");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
            
            //16 - AP Channel
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (AP_CHANNEL_ENTRY);
            espresponse->print ("\",\"T\":\"B\",\"V\":\"");
            espresponse->print (String(prefs.getChar(AP_CHANNEL_ENTRY, DEFAULT_AP_CHANNEL)).c_str());
            espresponse->print ("\",\"H\":\"AP Channel\",\"O\":[");
            for (int i = MIN_CHANNEL; i <= MAX_CHANNEL ; i++) {
                espresponse->print ("{\"");
                espresponse->print (String(i).c_str());
                espresponse->print ("\":\"");
                espresponse->print (String(i).c_str());
                espresponse->print ("\"}");
                if (i < MAX_CHANNEL) {
                    espresponse->print (",");
                }
            }
            espresponse->print ("]}");
#endif
            espresponse->print ("]}");
            if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
            prefs.end();
            }
            break;

        //Set EEPROM setting
        //[ESP401]P=<position> T=<type> V=<value> pwd=<user/admin password>
        case 401:    
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                if (espresponse)espresponse->println ("Error: Wrong authentication!");
                return false;
            }
#endif
            //check validity of parameters
            String spos = get_param (cmd_params, "P=", false);
            String styp = get_param (cmd_params, "T=", false);
            String sval = get_param (cmd_params, "V=", true);
            spos.trim();
            sval.trim();
            if (spos.length() == 0) {
                response = false;
            }
            if (! (styp == "B" || styp == "S" || styp == "A" || styp == "I" || styp == "F") ) {
                response = false;
            }
            if (sval.length() == 0) {
                response = false;
            }

            if (response) {
                Preferences prefs;
                prefs.begin(NAMESPACE, false);
                //Byte value
                if ((styp == "B")  ||  (styp == "F")){
                    int8_t bbuf = sval.toInt();
                    if (prefs.putChar(spos.c_str(), bbuf) ==0 ) {
                        response = false;
                    } else {
#if defined (ENABLE_WIFI)
                        //dynamique refresh is better than restart the board
                        if (spos == ESP_RADIO_MODE){
                            //TODO
                        }
                        if (spos == AP_CHANNEL_ENTRY) {
                            //TODO
                        }
#if defined (ENABLE_HTTP)
                        if (spos == HTTP_ENABLE_ENTRY) {
                            //TODO
                        }
#endif
#if defined (ENABLE_TELNET)
                        if (spos == TELNET_ENABLE_ENTRY) {
                            //TODO
                        }
#endif
#endif
                    }
                }
                //Integer value
                if (styp == "I") {
                    int16_t ibuf = sval.toInt();
                    if (prefs.putUShort(spos.c_str(), ibuf) == 0) {
                        response = false;
                    } else {
#if defined (ENABLE_WIFI)
#if defined (ENABLE_HTTP)
                        if (spos == HTTP_PORT_ENTRY){
                            //TODO
                        }
#endif
#if defined (ENABLE_TELNET)
                        if (spos == TELNET_PORT_ENTRY){
                            //TODO
                            //Serial.println(ibuf);
                        }
#endif
#endif
                    }
                    
                }
                //String value
                if (styp == "S") {
                   if (prefs.putString(spos.c_str(), sval) == 0) {
                        response = false;
                    } else {
#if defined (ENABLE_WIFI)
                        if (spos == HOSTNAME_ENTRY){
                            //TODO
                        }
                        if (spos == STA_SSID_ENTRY){
                            //TODO
                        }
                        if (spos == STA_PWD_ENTRY){
                            //TODO
                        }
                        if (spos == AP_SSID_ENTRY){
                            //TODO
                        }
                        if (spos == AP_PWD_ENTRY){
                            //TODO
                        }
#endif
                    }
                    
                }
#if defined (ENABLE_WIFI)
                //IP address
                if (styp == "A") {
                    if (prefs.putInt(spos.c_str(), wifi_config.IP_int_from_string(sval)) == 0) {
                        response = false;
                    } else {

                        if (spos == STA_IP_ENTRY){
                            //TODO
                        }
                        if (spos == STA_GW_ENTRY){
                            //TODO
                        }
                        if (spos == STA_MK_ENTRY){
                            //TODO
                        }
                        if (spos == AP_IP_ENTRY){
                            //TODO
                        }
                    }
                }
#endif
                prefs.end();
            }
            if (!response) {
                if (espresponse) espresponse->println ("Error: Incorrect Command");
            } else {
                 if (espresponse) espresponse->println ("ok");
            }

            }
            break;
#if defined (ENABLE_WIFI)
    //Get available AP list (limited to 30)
    //output is JSON 
    //[ESP410]
    case 410: {
        if (!espresponse)return false;
#ifdef ENABLE_AUTHENTICATION
            if (auth_type == LEVEL_GUEST) {
                espresponse->println ("Error: Wrong authentication!");
                return false;
            }
#endif
        espresponse->print("{\"AP_LIST\":[");
        if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
        int n = WiFi.scanComplete();
        if (n == -2) {
            WiFi.scanNetworks (true);
        } else if (n) {	
            for (int i = 0; i < n; ++i) {
                if (i > 0) {
                    espresponse->print (",");
                    if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
                }
                espresponse->print ("{\"SSID\":\"");
                espresponse->print (WiFi.SSID (i).c_str());
                espresponse->print ("\",\"SIGNAL\":\"");
                espresponse->print (String(wifi_config.getSignal (WiFi.RSSI (i) )).c_str());
                espresponse->print ("\",\"IS_PROTECTED\":\"");
                
                if (WiFi.encryptionType (i) == WIFI_AUTH_OPEN) {
                    espresponse->print ("0");
                } else {
                    espresponse->print ("1");
                }
                espresponse->print ("\"}");
                }
            }
            WiFi.scanDelete();      
            if (WiFi.scanComplete() == -2) {
                WiFi.scanNetworks (true);
            }
       espresponse->print ("]}");
       if(espresponse->client() != CLIENT_WEBUI)espresponse->println("");
    }
    break;
#endif
        //Get ESP current status 
       case 420:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type == LEVEL_GUEST) {
                espresponse->println ("Error: Wrong authentication!");
                return false;
            }
#endif
            if (!espresponse)return false;
            espresponse->print ("Chip ID: ");
            espresponse->print (String ( (uint16_t) (ESP.getEfuseMac() >> 32) ).c_str());
            espresponse->println("");
            espresponse->print ("CPU Frequency: ");
            espresponse->print (String (ESP.getCpuFreqMHz() ).c_str());
            espresponse->print ("Mhz");
            espresponse->println("");
            espresponse->print ("CPU Temperature: ");
            espresponse->print (String (temperatureRead(), 1).c_str());
            if(espresponse->client() == CLIENT_WEBUI)espresponse->print ("&deg;");
            espresponse->print ("C");
            espresponse->println("");
            espresponse->print ("Free memory: ");
            espresponse->print (ESPResponseStream::formatBytes (ESP.getFreeHeap()).c_str());
            espresponse->println("");
            espresponse->print ("SDK: ");
            espresponse->print (ESP.getSdkVersion());
            espresponse->println("");
            espresponse->print ("Flash Size: ");
            espresponse->print (ESPResponseStream::formatBytes (ESP.getFlashChipSize()).c_str());
            espresponse->println("");
#if defined (ENABLE_WIFI)
            if (WiFi.getMode() != WIFI_MODE_NULL){
                espresponse->print ("Available Size for update: ");
                //Not OTA on 2Mb board per spec
                if (ESP.getFlashChipSize() > 0x20000) {
                    espresponse->print (ESPResponseStream::ESPResponseStream::formatBytes (0x140000).c_str());
                } else {
                    espresponse->print (ESPResponseStream::formatBytes (0x0).c_str());
                }
                espresponse->println("");
            }
            if (WiFi.getMode() != WIFI_MODE_NULL){
                espresponse->print ("Available Size for SPIFFS: ");
                espresponse->print (ESPResponseStream::formatBytes (SPIFFS.totalBytes()).c_str());
                espresponse->println("");
            }
#endif
            espresponse->print ("Baud rate: ");
            long br = Serial.baudRate();
            //workaround for ESP32
            if (br == 115201) {
                br = 115200;
            }
            if (br == 230423) {
                br = 230400;
            }
            espresponse->print (String(br).c_str());
            espresponse->println("");
            espresponse->print ("Sleep mode: ");
            if (WiFi.getSleep())espresponse->print ("Modem");
            else espresponse->print ("None");
            espresponse->println("");
#if defined (ENABLE_WIFI) 
#if defined (ENABLE_HTTP)
            if (WiFi.getMode() != WIFI_MODE_NULL){
                espresponse->print ("Web port: ");
                espresponse->print (String(web_server.port()).c_str());
                espresponse->println("");
            }
#endif
#if defined (ENABLE_TELNET)
            if (WiFi.getMode() != WIFI_MODE_NULL){
                espresponse->print ("Data port: ");
                espresponse->print (String(telnet_server.port()).c_str());
                espresponse->println("");
            }
#endif
            if (WiFi.getMode() != WIFI_MODE_NULL){
                espresponse->print ("Hostname: ");
                espresponse->print ( wifi_config.Hostname().c_str());
                espresponse->println("");
            }
            espresponse->print ("Current WiFi Mode: ");
            if (WiFi.getMode() == WIFI_STA) {
                 espresponse->print ("STA (");
                 espresponse->print ( WiFi.macAddress().c_str());
                 espresponse->print (")");
                 espresponse->println("");
                 espresponse->print ("Connected to: ");
                 if (WiFi.isConnected()){ //in theory no need but ...  
                     espresponse->print (WiFi.SSID().c_str());
                     espresponse->println("");
                     espresponse->print ("Signal: ");
                     espresponse->print ( String(wifi_config.getSignal (WiFi.RSSI())).c_str());
                     espresponse->print ("%");
                     espresponse->println("");
                     uint8_t PhyMode;
                     esp_wifi_get_protocol (ESP_IF_WIFI_STA, &PhyMode);
                     espresponse->print ("Phy Mode: ");
                     if (PhyMode == (WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N)) espresponse->print ("11n");
                     else if (PhyMode == (WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G)) espresponse->print ("11g");
                     else if (PhyMode == (WIFI_PROTOCOL_11B )) espresponse->print ("11b");
                     else espresponse->print ("???");
                     espresponse->println("");
                     espresponse->print ("Channel: ");
                     espresponse->print (String (WiFi.channel()).c_str());
                     espresponse->println("");
                     espresponse->print ("IP Mode: ");
                     tcpip_adapter_dhcp_status_t dhcp_status;
                     tcpip_adapter_dhcpc_get_status (TCPIP_ADAPTER_IF_STA, &dhcp_status);
                     if (dhcp_status == TCPIP_ADAPTER_DHCP_STARTED)espresponse->print ("DHCP");
                     else espresponse->print ("Static");
                     espresponse->println("");
                     espresponse->print ("IP: ");
                     espresponse->print (WiFi.localIP().toString().c_str());
                     espresponse->println("");
                     espresponse->print ("Gateway: ");
                     espresponse->print (WiFi.gatewayIP().toString().c_str());
                     espresponse->println("");
                     espresponse->print ("Mask: ");
                     espresponse->print (WiFi.subnetMask().toString().c_str());
                     espresponse->println("");
                     espresponse->print ("DNS: ");
                     espresponse->print (WiFi.dnsIP().toString().c_str());
                     espresponse->println("");
                 } //this is web command so connection => no command 
                 espresponse->print ("Disabled Mode: ");
                 espresponse->print ("AP (");
                 espresponse->print (WiFi.softAPmacAddress().c_str());
                 espresponse->print (")");
                 espresponse->println("");
            } else if (WiFi.getMode() == WIFI_AP) {
                 espresponse->print ("AP (");
                 espresponse->print (WiFi.softAPmacAddress().c_str());
                 espresponse->print (")");
                 espresponse->println("");
                 wifi_config_t conf;
                 esp_wifi_get_config (ESP_IF_WIFI_AP, &conf);
                 espresponse->print ("SSID: ");
                 espresponse->print ((const char*) conf.ap.ssid);
                 espresponse->println("");
                 espresponse->print ("Visible: ");
                 espresponse->print ( (conf.ap.ssid_hidden == 0) ? "Yes" : "No");
                 espresponse->println("");
                 espresponse->print ("Authentication: ");
                 if (conf.ap.authmode == WIFI_AUTH_OPEN) {
                    espresponse->print ("None");
                 } else if (conf.ap.authmode == WIFI_AUTH_WEP) {
                    espresponse->print ("WEP");
                 } else if (conf.ap.authmode == WIFI_AUTH_WPA_PSK) {
                    espresponse->print ("WPA");
                 } else if (conf.ap.authmode == WIFI_AUTH_WPA2_PSK) {
                    espresponse->print ("WPA2");
                 } else {
                    espresponse->print ("WPA/WPA2");
                 }
                espresponse->println("");
                espresponse->print ("Max Connections: ");
                espresponse->print (String(conf.ap.max_connection).c_str());
                espresponse->println("");
                espresponse->print ("DHCP Server: ");
                tcpip_adapter_dhcp_status_t dhcp_status;
                tcpip_adapter_dhcps_get_status (TCPIP_ADAPTER_IF_AP, &dhcp_status);
                if (dhcp_status == TCPIP_ADAPTER_DHCP_STARTED)espresponse->print ("Started");
                else espresponse->print ("Stopped");
                espresponse->println("");
                espresponse->print ("IP: ");
                espresponse->print (WiFi.softAPIP().toString().c_str());
                espresponse->println("");
                tcpip_adapter_ip_info_t ip_AP;
                tcpip_adapter_get_ip_info (TCPIP_ADAPTER_IF_AP, &ip_AP);
                espresponse->print ("Gateway: ");
                espresponse->print (IPAddress (ip_AP.gw.addr).toString().c_str());
                espresponse->println("");
                espresponse->print ("Mask: ");
                espresponse->print (IPAddress (ip_AP.netmask.addr).toString().c_str());
                espresponse->println("");
                espresponse->print ("Connected clients: ");
                wifi_sta_list_t station;
                tcpip_adapter_sta_list_t tcpip_sta_list;
                esp_wifi_ap_get_sta_list (&station);
                tcpip_adapter_get_sta_list (&station, &tcpip_sta_list);
                espresponse->print (String(station.num).c_str());
                espresponse->println("");
                for (int i = 0; i < station.num; i++) {
                    espresponse->print (wifi_config.mac2str(tcpip_sta_list.sta[i].mac));
                    espresponse->print (" ");
                    espresponse->print ( IPAddress (tcpip_sta_list.sta[i].ip.addr).toString().c_str());
                    espresponse->println("");
                    }
                espresponse->print ("Disabled Mode: ");
                espresponse->print ("STA (");
                espresponse->print (WiFi.macAddress().c_str());
                espresponse->print (")");
                espresponse->println("");
            } else if (WiFi.getMode() == WIFI_AP_STA) //we should not be in this state but just in case ....
            {
               espresponse->print ("Mixed");
               espresponse->println("");
               espresponse->print ("STA (");
               espresponse->print (WiFi.macAddress().c_str());
               espresponse->print (")");
               espresponse->println("");
               espresponse->print ("AP (");
               espresponse->print (WiFi.softAPmacAddress().c_str());
               espresponse->print (")");
               espresponse->println("");
        
            } else { //we should not be there if no wifi ....
               espresponse->print ("Off");
               espresponse->println("");
            }
#endif
#ifdef ENABLE_BLUETOOTH
             espresponse->print ("Current BT Mode: ");
             if (bt_config.Is_BT_on()){
               espresponse->println("On");
               espresponse->print ("BT Name: ");
               espresponse->print (bt_config.BTname().c_str());
               espresponse->print ("(");
               espresponse->print (bt_config.device_address());
               espresponse->println(")");
               espresponse->print ("Status: ");
               if(SerialBT.hasClient()) espresponse->print ("Connected");
               else espresponse->print ("Not connected");
            } else{
                espresponse->print ("Off");
            }
            espresponse->println("");
#endif
            //TODO to complete
            espresponse->print ("FW version: ");
            espresponse->print (GRBL_VERSION);
            espresponse->print (" (");
            espresponse->print (GRBL_VERSION_BUILD);
            espresponse->print (") (ESP32)");
            espresponse->println("");
            }
            break;
         //Set ESP mode
         //cmd is RESTART
         //[ESP444]<cmd>
      case 444:
            parameter = get_param(cmd_params,"", true);
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                if (espresponse)espresponse->println ("Error: Wrong authentication!");
                return false;
            }
#endif
            { 
                if (parameter=="RESTART") {
                 grbl_send(CLIENT_ALL,"[MSG:Restart ongoing]\r\n");
                 COMMANDS::restart_ESP();
                } else response = false;
            }
            if (!response) {
                 if (espresponse)espresponse->println ("Error: Incorrect Command");
            } else {
                 if (espresponse)espresponse->println ("ok");
            }
            break;
#ifdef ENABLE_AUTHENTICATION
    //Change / Reset user password
    //[ESP555]<password>
    case 555: {
        if (auth_type == LEVEL_ADMIN) {
            parameter = get_param (cmd_params, "", true);
            if (parameter.length() == 0) {
                Preferences prefs;
                parameter = DEFAULT_USER_PWD;
                prefs.begin(NAMESPACE, false);
                if (prefs.putString(USER_PWD_ENTRY, parameter) != parameter.length()){
                    response = false;
                    if (espresponse)espresponse->println ("error");
                } else if (espresponse)espresponse->println ("ok");
                prefs.end();
                
            } else {
                if (isLocalPasswordValid (parameter.c_str() ) ) {
                    Preferences prefs;
                    prefs.begin(NAMESPACE, false);
                    if (prefs.putString(USER_PWD_ENTRY, parameter) != parameter.length()) {
                        response = false;
                        if (espresponse)espresponse->println ("error");
                    } else if (espresponse)espresponse->println ("ok");
                    prefs.end();
                } else {
                    if (espresponse)espresponse->println ("error");
                    response = false;
                }
            }
        } else {
            if (espresponse)espresponse->println ("error");
            response = false;
        }
        break;
    }
#endif
    //[ESP700]<filename> pwd=<user/admin password>
    case 700: { //read local file
       
#ifdef ENABLE_AUTHENTICATION
            if (auth_type == LEVEL_GUEST) {
                if (espresponse)espresponse->println ("Error: Wrong authentication!");
                return false;
            }
#endif 
        parameter = get_param (cmd_params, "", true);
        if ( (parameter.length() > 0) && (parameter[0] != '/') ) {
            parameter = "/" + parameter;
        }
        if (!SPIFFS.exists(parameter)){
            if (espresponse)espresponse->println ("Error:No such file!");
            response = false;
        } else {
            File currentfile = SPIFFS.open (parameter, FILE_READ);
            if (currentfile) {//if file open success
                //until no line in file
                while (currentfile.available()) {
                    String currentline = currentfile.readStringUntil('\n');
                    currentline.replace("\n","");
                    currentline.replace("\r","");
                    if (currentline.length() > 0) {
                        int ESPpos = currentline.indexOf ("[ESP");
                        if (ESPpos > -1) {
                            //is there the second part?
                            int ESPpos2 = currentline.indexOf ("]", ESPpos);
                            if (ESPpos2 > -1) {
                                //Split in command and parameters
                                String cmd_part1 = currentline.substring (ESPpos + 4, ESPpos2);
                                String cmd_part2 = "";
                                //is there space for parameters?
                                if (ESPpos2 < currentline.length() ) {
                                    cmd_part2 = currentline.substring (ESPpos2 + 1);
                                }
                                //if command is a valid number then execute command
                                if(cmd_part1.toInt()!=0) {
                                    if (!execute_internal_command(cmd_part1.toInt(),cmd_part2, auth_type, espresponse)) response = false;
                                }
                                //if not is not a valid [ESPXXX] command ignore it
                            }
                        } else {
                            //preprocess line
                            String processedline = "";
                            char c;
                            uint8_t line_flags = 0;
                            for (uint16_t index=0; index < currentline.length(); index++){
                                c = currentline[index];
                                if (c == '\r' || c == ' ' || c == '\n') {  
                                // ignore these whitespace items
                                    }
                                else if (c == '(') {
                                    line_flags |= LINE_FLAG_COMMENT_PARENTHESES;
                                }
                                else if (c == ')') {
                                // End of '()' comment. Resume line allowed.
                                if (line_flags & LINE_FLAG_COMMENT_PARENTHESES) { line_flags &= ~(LINE_FLAG_COMMENT_PARENTHESES); }
                                }
                                else if (c == ';') {
                                // NOTE: ';' comment to EOL is a LinuxCNC definition. Not NIST.
                                    if (!(line_flags & LINE_FLAG_COMMENT_PARENTHESES)) // semi colon inside parentheses do not mean anything						
                                        line_flags |= LINE_FLAG_COMMENT_SEMICOLON;
                                }
                                
                                else { // add characters to the line
                                    if (!line_flags) {
                                        c = toupper(c); // make upper case
                                        processedline += c;
                                        }
                                    }
                                }
                            if (processedline.length() > 0)gc_execute_line((char *)processedline.c_str(), CLIENT_WEBUI);
                            wait (1);
                        }
                    wait (1);
                    }
                }
                currentfile.close();
                 if (espresponse)espresponse->println ("ok");
            } else {
                 if (espresponse)espresponse->println ("error");
                response = false;
            }
        }
        break;
    }
    //Format SPIFFS
    //[ESP710]FORMAT pwd=<admin password>
    case 710:
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                if (espresponse)espresponse->println ("Error: Wrong authentication!");
                return false;
            }
#endif
        parameter = get_param (cmd_params, "", true);
        {
            if (parameter == "FORMAT") {
                 if (espresponse)espresponse->print ("Formating");
                SPIFFS.format();
                 if (espresponse)espresponse->println ("...Done");
            } else {
                 if (espresponse)espresponse->println ("error");
                response = false;
            }
        }
        break;
    //SPIFFS total size and used size
    //[ESP720]<header answer>
    case 720:
    if (!espresponse)return false;
#ifdef ENABLE_AUTHENTICATION
            if (auth_type == LEVEL_GUEST) {
                espresponse->println ("Error: Wrong authentication!");
                return false;
            }
#endif
        parameter = get_param (cmd_params, "", true);
        espresponse->print (parameter.c_str());
        espresponse->print ("SPIFFS  Total:");
        espresponse->print (ESPResponseStream::formatBytes (SPIFFS.totalBytes() ).c_str());
        espresponse->print (" Used:");
        espresponse->println (ESPResponseStream::formatBytes (SPIFFS.usedBytes() ).c_str());
        break;
        //get fw version / fw target / hostname / authentication
        //[ESP800]
        case 800: 
            {
            if (!espresponse)return false;
            String resp;
            resp = "FW version:";
            resp += GRBL_VERSION;
            resp += " # FW target:grbl-embedded  # FW HW:";
            #ifdef ENABLE_SD_CARD
            resp += "Direct SD";
            #else
            resp += "No SD";
            #endif
            resp += "  # primary sd:/sd # secondary sd:none # authentication:";
            #ifdef ENABLE_AUTHENTICATION
            resp += "yes";
            #else
            resp += "no";
            #endif
            #if defined (ENABLE_WIFI)
            #if defined (ENABLE_HTTP)
            resp += " # webcommunication: Sync: ";
            resp += String(web_server.port() + 1);
            #endif
            resp += "# hostname:";
            resp += wifi_config.Hostname();
            if (WiFi.getMode() == WIFI_AP)resp += "(AP mode)";
            #endif
            if (espresponse)espresponse->println (resp.c_str());
        }   
            break;
        default:
             if (espresponse)espresponse->println ("Error: Incorrect Command");
            response = false;
            break;
    }
    return response;
}


String COMMANDS::get_param(String & cmd_params, const char * id, bool withspace)
{
    static String parameter;
    String sid = id;
    int start;
    int end = -1;
    if (cmd_params.indexOf("pwd=") == 0)cmd_params = " " + cmd_params;
    parameter = "";
    //if no id it means it is first part of cmd
    if (strlen (id) == 0) {
        start = 0;
    }
    //else find id position
    else {
        start = cmd_params.indexOf (id);
    }
    //if no id found and not first part leave
    if (start == -1 ) {
        return parameter;
    }
    //password and SSID can have space so handle it
    //if no space expected use space as delimiter
    if (!withspace) {
        end = cmd_params.indexOf (" ", start);
    }
#ifdef ENABLE_AUTHENTICATION
    //if space expected only one parameter but additional password may be present
    else if (sid != " pwd=") {
        end = cmd_params.indexOf (" pwd=", start);
    }
#endif
    //if no end found - take all
    if (end == -1) {
        end = cmd_params.length();
    }
    //extract parameter
    parameter = cmd_params.substring (start + strlen (id), end);
    //be sure no extra space
    parameter.trim();
    return parameter;
}

#ifdef ENABLE_AUTHENTICATION

bool COMMANDS::isLocalPasswordValid (const char * password)
{
    char c;
    //limited size
    if ( (strlen (password) > MAX_LOCAL_PASSWORD_LENGTH) ||  (strlen (password) < MIN_LOCAL_PASSWORD_LENGTH) ) {
        return false;
    }
    //no space allowed
    for (int i = 0; i < strlen (password); i++) {
        c = password[i];
        if (c == ' ') {
            return false;
        }
    }
    return true;
}

//check admin password
bool COMMANDS::isadmin (String & cmd_params)
{   String adminpassword;
    String sadminPassword;
    Preferences prefs;
    prefs.begin(NAMESPACE, true);
    String defV = DEFAULT_ADMIN_PWD;
    sadminPassword = prefs.getString(ADMIN_PWD_ENTRY, defV);
    prefs.end();
    adminpassword = get_param (cmd_params, "pwd=", true);
    if (!sadminPassword.equals (adminpassword) ) {
        return false;
    } else {
        return true;
    }
}
//check user password - admin password is also valid
bool COMMANDS::isuser (String & cmd_params)
{
    String userpassword;
    String suserPassword;
    Preferences prefs;
    prefs.begin(NAMESPACE, true);
    String defV = DEFAULT_USER_PWD;
    suserPassword = prefs.getString(USER_PWD_ENTRY, defV);
    prefs.end();
    userpassword = get_param (cmd_params, "pwd=", true);
    //it is not user password
    if (!suserPassword.equals (userpassword) ) {
        //check admin password
        return isadmin (cmd_params);
    } else {
        return true;
    }
}
#endif

//check is valid [ESPXXX] command
//return XXX as cmd and command as cmd_params
bool COMMANDS::check_command (const char * line, int * cmd, String & cmd_params)
{
    String buffer = line;
    bool result = false;
    int ESPpos = buffer.indexOf ("[ESP");
    if (ESPpos > -1) {
        //is there the second part?
        int ESPpos2 = buffer.indexOf ("]", ESPpos);
        if (ESPpos2 > -1) {
            //Split in command and parameters
            String cmd_part1 = buffer.substring (ESPpos + 4, ESPpos2);
            String cmd_part2 = "";
            //is there space for parameters?
            if (ESPpos2 < buffer.length() ) {
                cmd_part2 = buffer.substring (ESPpos2 + 1);
            }
            //if command is a valid number then execute command
            if (cmd_part1.toInt() != 0) {
                 *cmd = cmd_part1.toInt();
                 cmd_params = cmd_part2;
                 result = true;
            }
            //if not is not a valid [ESPXXX] command
        }
    }
    return result;
}

/**
 * Restart ESP
 */
void COMMANDS::restart_ESP(){
    restart_ESP_module=true;
}

/**
 * Handle not critical actions that must be done in sync environement
 */
void COMMANDS::handle() {
    //in case of restart requested
    if (restart_ESP_module) {
        ESP.restart();
        while (1) {};
    }
}
