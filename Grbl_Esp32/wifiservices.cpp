/*
  wifiservices.cpp -  wifi services functions class

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

#ifdef ARDUINO_ARCH_ESP32

#include "config.h"

#ifdef ENABLE_WIFI

#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include "report.h"
#include "wificonfig.h"
#include "wifiservices.h"
#ifdef ENABLE_MDNS
#include <ESPmDNS.h>
#endif
#ifdef ENABLE_OTA
#include <ArduinoOTA.h>
#endif
#ifdef ENABLE_HTTP
#include "web_server.h"
#endif
#ifdef ENABLE_TELNET
#include "telnet_server.h"
#endif
#ifdef ENABLE_NOTIFICATIONS
#include "notifications_service.h"
#endif
#include "commands.h"

WiFiServices wifi_services;

WiFiServices::WiFiServices(){
}
WiFiServices::~WiFiServices(){
    end();
}

bool WiFiServices::begin(){
    bool no_error = true;
    //Sanity check
    if(WiFi.getMode() == WIFI_OFF) return false;
    String h;
    Preferences prefs;
    //Get hostname
    String defV = DEFAULT_HOSTNAME;
    prefs.begin(NAMESPACE, true);
    h = prefs.getString(HOSTNAME_ENTRY, defV);
    prefs.end();
    //Start SPIFFS
    SPIFFS.begin(true);

#ifdef ENABLE_OTA
    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else {// U_SPIFFS
        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        type = "filesystem";
        SPIFFS.end();
        }
      grbl_sendf(CLIENT_ALL,"[MSG:Start OTA updating %s]\r\n", type.c_str());
    })
    .onEnd([]() {
      grbl_sendf(CLIENT_ALL,"[MSG:End OTA]\r\n");
      
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      grbl_sendf(CLIENT_ALL,"[MSG:OTA Progress: %u%%]\r\n", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      grbl_sendf(CLIENT_ALL,"[MSG:OTA Error(%u):]\r\n", error);
      if (error == OTA_AUTH_ERROR) grbl_send(CLIENT_ALL,"[MSG:Auth Failed]\r\n");
      else if (error == OTA_BEGIN_ERROR) grbl_send(CLIENT_ALL,"[MSG:Begin Failed]\r\n");
      else if (error == OTA_CONNECT_ERROR) grbl_send(CLIENT_ALL,"[MSG:Connect Failed]\r\n");
      else if (error == OTA_RECEIVE_ERROR) grbl_send(CLIENT_ALL,"[MSG:Receive Failed]\r\n");
      else if (error == OTA_END_ERROR) grbl_send(CLIENT_ALL,"[MSG:End Failed]\r\n");
    });
  ArduinoOTA.begin();
#endif
#ifdef ENABLE_MDNS
     //no need in AP mode
     if(WiFi.getMode() == WIFI_STA){
        //start mDns
        if (!MDNS.begin(h.c_str())) {
            grbl_send(CLIENT_ALL,"[MSG:Cannot start mDNS]\r\n");
            no_error = false;
        } else {
            grbl_sendf(CLIENT_ALL,"[MSG:Start mDNS with hostname:http://%s.local/]\r\n",h.c_str());
        }
    }
#endif
#ifdef ENABLE_HTTP
    web_server.begin();
#endif
#ifdef ENABLE_TELNET
    telnet_server.begin();
#endif
#ifdef ENABLE_NOTIFICATIONS
	notificationsservice.begin();
#endif
    //be sure we are not is mixed mode in setup
    WiFi.scanNetworks (true);
    return no_error;
}
void WiFiServices::end(){
#ifdef ENABLE_NOTIFICATIONS
	notificationsservice.end();
#endif
#ifdef ENABLE_TELNET
    telnet_server.end();
#endif

#ifdef ENABLE_HTTP
    web_server.end();
#endif
    //stop OTA
#ifdef ENABLE_OTA
    ArduinoOTA.end();
#endif
    //Stop SPIFFS
    SPIFFS.end();
    
#ifdef ENABLE_MDNS
    //Stop mDNS
    MDNS.end();
#endif 
}

void WiFiServices::handle(){
    COMMANDS::wait(0);
    //to avoid mixed mode due to scan network
    if (WiFi.getMode() == WIFI_AP_STA) {
        if (WiFi.scanComplete() != WIFI_SCAN_RUNNING) {
            WiFi.enableSTA (false);
        }
    }
#ifdef ENABLE_OTA
    ArduinoOTA.handle();
#endif
#ifdef ENABLE_HTTP
    web_server.handle();
#endif
#ifdef ENABLE_TELNET
    telnet_server.handle();
#endif
}

#endif // ENABLE_WIFI

#endif // ARDUINO_ARCH_ESP32
