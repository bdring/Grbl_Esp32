/*
  WifiConfig.cpp -  wifi functions class

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

#include "../Grbl.h"

#ifdef ENABLE_WIFI

#    include <WiFi.h>
#    include <esp_wifi.h>
#    include <ESPmDNS.h>
#    include <FS.h>
#    include <SPIFFS.h>
#    include <cstring>
#    include "WifiServices.h"

namespace WebUI {
    WiFiConfig wifi_config;

    String WiFiConfig::_hostname          = "";
    bool   WiFiConfig::_events_registered = false;

    WiFiConfig::WiFiConfig() {}

    //just simple helper to convert mac address to string
    char* WiFiConfig::mac2str(uint8_t mac[8]) {
        static char macstr[18];
        if (0 > sprintf(macstr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5])) {
            strcpy(macstr, "00:00:00:00:00:00");
        }
        return macstr;
    }

    const char* WiFiConfig::info() {
        static String result;
        String        tmp;
        result = "[MSG:";

        if ((WiFi.getMode() == WIFI_MODE_STA) || (WiFi.getMode() == WIFI_MODE_APSTA)) {
            result += "Mode=STA:SSID=";
            result += WiFi.SSID();
            result += ":Status=";
            result += (WiFi.status() == WL_CONNECTED) ? "Connected" : "Not connected";
            result += ":IP=";
            result += WiFi.localIP().toString();
            result += ":MAC=";
            tmp = WiFi.macAddress();
            tmp.replace(":", "-");
            result += tmp;
        }

        if ((WiFi.getMode() == WIFI_MODE_AP) || (WiFi.getMode() == WIFI_MODE_APSTA)) {
            if (WiFi.getMode() == WIFI_MODE_APSTA) {
                result += "]\r\n[MSG:";
            }
            result += "Mode=AP:SSDI=";
            wifi_config_t conf;
            esp_wifi_get_config(WIFI_IF_AP, &conf);
            result += (const char*)conf.ap.ssid;
            result += ":IP=";
            result += WiFi.softAPIP().toString();
            result += ":MAC=";
            tmp = WiFi.softAPmacAddress();
            tmp.replace(":", "-");
            result += tmp;
        }
        if (WiFi.getMode() == WIFI_MODE_NULL) {
            result += "No Wifi";
        }
        result += "]\r\n";
        return result.c_str();
    }

    /**
     * Helper to convert  IP string to int
     */

    uint32_t WiFiConfig::IP_int_from_string(String& s) {
        uint32_t  ip_int = 0;
        IPAddress ipaddr;
        if (ipaddr.fromString(s)) {
            ip_int = ipaddr;
        }
        return ip_int;
    }

    /**
     * Helper to convert int to IP string
     */

    String WiFiConfig::IP_string_from_int(uint32_t ip_int) {
        IPAddress ipaddr(ip_int);
        return ipaddr.toString();
    }

    /**
     * Check if Hostname string is valid
     */

    bool WiFiConfig::isHostnameValid(const char* hostname) {
        //limited size
        if (!hostname) {
            return true;
        }
        char c;
        // length is checked automatically by string setting
        //only letter and digit
        for (int i = 0; i < strlen(hostname); i++) {
            c = hostname[i];
            if (!(isdigit(c) || isalpha(c) || c == '-')) {
                return false;
            }
            if (c == ' ') {
                return false;
            }
        }
        return true;
    }

    /**
     * Check if SSID string is valid
     */

    bool WiFiConfig::isSSIDValid(const char* ssid) {
        //limited size
        //char c;
        // length is checked automatically by string setting
        //only printable
        if (!ssid) {
            return true;
        }
        for (int i = 0; i < strlen(ssid); i++) {
            if (!isPrintable(ssid[i])) {
                return false;
            }
        }
        return true;
    }

    /**
     * Check if password string is valid
     */

    bool WiFiConfig::isPasswordValid(const char* password) {
        if (!password) {
            return true;
        }
        if (strlen(password) == 0) {
            return true;  //open network
        }

        // Limited size. Length is checked automatically by string setting

        return true;
    }

    /**
     * Check if IP string is valid
     */
    bool WiFiConfig::isValidIP(const char* string) {
        IPAddress ip;
        return ip.fromString(string);
    }

    /**
     * WiFi events
     * SYSTEM_EVENT_WIFI_READY               < ESP32 WiFi ready
     * SYSTEM_EVENT_SCAN_DONE                < ESP32 finish scanning AP
     * SYSTEM_EVENT_STA_START                < ESP32 station start
     * SYSTEM_EVENT_STA_STOP                 < ESP32 station stop
     * SYSTEM_EVENT_STA_CONNECTED            < ESP32 station connected to AP
     * SYSTEM_EVENT_STA_DISCONNECTED         < ESP32 station disconnected from AP
     * SYSTEM_EVENT_STA_AUTHMODE_CHANGE      < the auth mode of AP connected by ESP32 station changed
     * SYSTEM_EVENT_STA_GOT_IP               < ESP32 station got IP from connected AP
     * SYSTEM_EVENT_STA_LOST_IP              < ESP32 station lost IP and the IP is reset to 0
     * SYSTEM_EVENT_STA_WPS_ER_SUCCESS       < ESP32 station wps succeeds in enrollee mode
     * SYSTEM_EVENT_STA_WPS_ER_FAILED        < ESP32 station wps fails in enrollee mode
     * SYSTEM_EVENT_STA_WPS_ER_TIMEOUT       < ESP32 station wps timeout in enrollee mode
     * SYSTEM_EVENT_STA_WPS_ER_PIN           < ESP32 station wps pin code in enrollee mode
     * SYSTEM_EVENT_AP_START                 < ESP32 soft-AP start
     * SYSTEM_EVENT_AP_STOP                  < ESP32 soft-AP stop
     * SYSTEM_EVENT_AP_STACONNECTED          < a station connected to ESP32 soft-AP
     * SYSTEM_EVENT_AP_STADISCONNECTED       < a station disconnected from ESP32 soft-AP
     * SYSTEM_EVENT_AP_PROBEREQRECVED        < Receive probe request packet in soft-AP interface
     * SYSTEM_EVENT_GOT_IP6                  < ESP32 station or ap or ethernet interface v6IP addr is preferred
     * SYSTEM_EVENT_ETH_START                < ESP32 ethernet start
     * SYSTEM_EVENT_ETH_STOP                 < ESP32 ethernet stop
     * SYSTEM_EVENT_ETH_CONNECTED            < ESP32 ethernet phy link up
     * SYSTEM_EVENT_ETH_DISCONNECTED         < ESP32 ethernet phy link down
     * SYSTEM_EVENT_ETH_GOT_IP               < ESP32 ethernet got IP from connected AP
     * SYSTEM_EVENT_MAX
     */

    void WiFiConfig::WiFiEvent(WiFiEvent_t event) {
        switch (event) {
            case SYSTEM_EVENT_STA_GOT_IP:
                grbl_sendf(CLIENT_ALL, "[MSG:Connected with %s]\r\n", WiFi.localIP().toString().c_str());
                break;
            case SYSTEM_EVENT_STA_DISCONNECTED:
                grbl_send(CLIENT_ALL, "[MSG:Disconnected]\r\n");
                break;
            default:
                break;
        }
    }

    /*
     * Get WiFi signal strength
     */
    int32_t WiFiConfig::getSignal(int32_t RSSI) {
        if (RSSI <= -100) {
            return 0;
        }
        if (RSSI >= -50) {
            return 100;
        }
        return 2 * (RSSI + 100);
    }

    /*
     * Connect client to AP
     */

    bool WiFiConfig::ConnectSTA2AP() {
        String      msg, msg_out;
        uint8_t     count  = 0;
        uint8_t     dot    = 0;
        wl_status_t status = WiFi.status();
        while (status != WL_CONNECTED && count < 40) {
            switch (status) {
                case WL_NO_SSID_AVAIL:
                    msg = "No SSID";
                    break;
                case WL_CONNECT_FAILED:
                    msg = "Connection failed";
                    break;
                case WL_CONNECTED:
                    break;
                default:
                    if ((dot > 3) || (dot == 0)) {
                        dot     = 0;
                        msg_out = "Connecting";
                    }
                    msg_out += ".";
                    msg = msg_out;
                    dot++;
                    break;
            }
            grbl_sendf(CLIENT_ALL, "[MSG:%s]\r\n", msg.c_str());
            COMMANDS::wait(500);
            count++;
            status = WiFi.status();
        }
        return status == WL_CONNECTED;
    }

    /*
     * Start client mode (Station)
     */

    bool WiFiConfig::StartSTA() {
        //stop active service
        wifi_services.end();
        //Sanity check
        if ((WiFi.getMode() == WIFI_STA) || (WiFi.getMode() == WIFI_AP_STA)) {
            WiFi.disconnect();
        }
        if ((WiFi.getMode() == WIFI_AP) || (WiFi.getMode() == WIFI_AP_STA)) {
            WiFi.softAPdisconnect();
        }
        WiFi.enableAP(false);
        WiFi.mode(WIFI_STA);
        //Get parameters for STA
        String h = wifi_hostname->get();
        WiFi.setHostname(h.c_str());
        //SSID
        String SSID = wifi_sta_ssid->get();
        if (SSID.length() == 0) {
            SSID = DEFAULT_STA_SSID;
        }
        //password
        String  password = wifi_sta_password->get();
        int8_t  IP_mode  = wifi_sta_mode->get();
        int32_t IP       = wifi_sta_ip->get();
        int32_t GW       = wifi_sta_gateway->get();
        int32_t MK       = wifi_sta_netmask->get();
        //if not DHCP
        if (IP_mode != DHCP_MODE) {
            IPAddress ip(IP), mask(MK), gateway(GW);
            WiFi.config(ip, gateway, mask);
        }
        if (WiFi.begin(SSID.c_str(), (password.length() > 0) ? password.c_str() : NULL)) {
            grbl_send(CLIENT_ALL, "\n[MSG:Client Started]\r\n");
            grbl_sendf(CLIENT_ALL, "[MSG:Connecting %s]\r\n", SSID.c_str());
            return ConnectSTA2AP();
        } else {
            grbl_send(CLIENT_ALL, "[MSG:Starting client failed]\r\n");
            return false;
        }
    }

    /**
     * Setup and start Access point
     */

    bool WiFiConfig::StartAP() {
        //stop active services
        wifi_services.end();
        //Sanity check
        if ((WiFi.getMode() == WIFI_STA) || (WiFi.getMode() == WIFI_AP_STA)) {
            WiFi.disconnect();
        }
        if ((WiFi.getMode() == WIFI_AP) || (WiFi.getMode() == WIFI_AP_STA)) {
            WiFi.softAPdisconnect();
        }
        WiFi.enableSTA(false);
        WiFi.mode(WIFI_AP);
        //Get parameters for AP
        //SSID
        String SSID = wifi_ap_ssid->get();
        if (SSID.length() == 0) {
            SSID = DEFAULT_AP_SSID;
        }

        String password = wifi_ap_password->get();

        int8_t channel = wifi_ap_channel->get();
        if (channel == 0) {
            channel = DEFAULT_AP_CHANNEL;
        }

        int32_t   IP = wifi_ap_ip->get();
        IPAddress ip(IP);
        IPAddress mask;
        mask.fromString(DEFAULT_AP_MK);
        //Set static IP
        WiFi.softAPConfig(ip, ip, mask);
        //Start AP
        if (WiFi.softAP(SSID.c_str(), (password.length() > 0) ? password.c_str() : NULL, channel)) {
            grbl_sendf(CLIENT_ALL, "\n[MSG:Local access point %s started, %s]\r\n", SSID.c_str(), WiFi.softAPIP().toString().c_str());
            return true;
        } else {
            grbl_send(CLIENT_ALL, "[MSG:Starting AP failed]\r\n");
            return false;
        }
    }

    /**
     * Stop WiFi
     */

    void WiFiConfig::StopWiFi() {
        //Sanity check
        if ((WiFi.getMode() == WIFI_STA) || (WiFi.getMode() == WIFI_AP_STA)) {
            WiFi.disconnect(true);
        }
        if ((WiFi.getMode() == WIFI_AP) || (WiFi.getMode() == WIFI_AP_STA)) {
            WiFi.softAPdisconnect(true);
        }
        wifi_services.end();
        WiFi.enableSTA(false);
        WiFi.enableAP(false);
        WiFi.mode(WIFI_OFF);
        grbl_send(CLIENT_ALL, "\n[MSG:WiFi Off]\r\n");
    }

    /**
     * begin WiFi setup
     */
    void WiFiConfig::begin() {
        //stop active services
        wifi_services.end();
        //setup events
        if (!_events_registered) {
            //cumulative function and no remove so only do once
            WiFi.onEvent(WiFiConfig::WiFiEvent);
            _events_registered = true;
        }
        //Get hostname
        _hostname       = wifi_hostname->get();
        int8_t wifiMode = wifi_radio_mode->get();
        if (wifiMode == ESP_WIFI_AP) {
            StartAP();
            //start services
            wifi_services.begin();
        } else if (wifiMode == ESP_WIFI_STA) {
            if (!StartSTA()) {
                grbl_sendf(CLIENT_ALL, "[MSG:Cannot connect to %s]\r\n", wifi_sta_ssid->get());
                StartAP();
            }
            //start services
            wifi_services.begin();
        } else {
            WiFi.mode(WIFI_OFF);
        }
    }

    /**
     * End WiFi
     */
    void WiFiConfig::end() { StopWiFi(); }

    /**
     * Reset ESP
     */
    void WiFiConfig::reset_settings() {
        bool error = false;
        for (Setting* s = Setting::List; s; s = s->next()) {
            if (s->getDescription()) {
                s->setDefault();
            }
        }
        // TODO commit the changes and check that for errors
        if (error) {
            grbl_send(CLIENT_ALL, "[MSG:WiFi reset error]\r\n");
        }
        grbl_send(CLIENT_ALL, "[MSG:WiFi reset done]\r\n");
    }
    bool WiFiConfig::Is_WiFi_on() { return !(WiFi.getMode() == WIFI_MODE_NULL); }

    /**
     * Handle not critical actions that must be done in sync environement
     */
    void WiFiConfig::handle() {
        //Services
        COMMANDS::wait(0);
        wifi_services.handle();
    }

    WiFiConfig::~WiFiConfig() { end(); }
}
#endif  // ENABLE_WIFI
