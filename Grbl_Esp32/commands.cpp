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
#ifdef ENABLE_NOTIFICATIONS
    #include "notifications_service.h"
#endif
#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <esp_wifi.h>
#include <esp_ota_ops.h>

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
void COMMANDS::wait(uint32_t milliseconds) {
    uint32_t timeout = millis();
    esp_task_wdt_reset(); //for a wait 0;
    //wait feeding WDT
    while ((millis() - timeout) < milliseconds)
        esp_task_wdt_reset();
}

#ifdef ENABLE_AUTHENTICATION

bool COMMANDS::isLocalPasswordValid(char* password) {
    char c;
    //limited size
    if ((strlen(password) > MAX_LOCAL_PASSWORD_LENGTH) || (strlen(password) < MIN_LOCAL_PASSWORD_LENGTH))
        return false;
    //no space allowed
    for (int i = 0; i < strlen(password); i++) {
        c = password[i];
        if (c == ' ')
            return false;
    }
    return true;
}
#endif

/**
 * Restart ESP
 */
void COMMANDS::restart_ESP() {
    restart_ESP_module = true;
}

/**
 * Handle not critical actions that must be done in sync environement
 */
void COMMANDS::handle() {
    COMMANDS::wait(0);
    //in case of restart requested
    if (restart_ESP_module) {
        ESP.restart();
        while (1)  ;
    }
}
