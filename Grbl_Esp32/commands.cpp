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

bool COMMANDS::execute_internal_command(int cmd, String cmd_params, level_authenticate_type auth_level,  ESPResponseStream*  espresponse) {
    bool response = true;
    return response;
}


String COMMANDS::get_param(String& cmd_params, const char* id, bool withspace) {
    static String parameter;
    String sid = id;
    int start;
    int end = -1;
    if (cmd_params.indexOf("pwd=") == 0)cmd_params = " " + cmd_params;
    parameter = "";
    //if no id it means it is first part of cmd
    if (strlen(id) == 0)
        start = 0;
    //else find id position
    else
        start = cmd_params.indexOf(id);
    //if no id found and not first part leave
    if (start == -1)
        return parameter;
    //password and SSID can have space so handle it
    //if no space expected use space as delimiter
    if (!withspace)
        end = cmd_params.indexOf(" ", start);
#ifdef ENABLE_AUTHENTICATION
    //if space expected only one parameter but additional password may be present
    else if (sid != " pwd=")
        end = cmd_params.indexOf(" pwd=", start);
#endif
    //if no end found - take all
    if (end == -1)
        end = cmd_params.length();
    //extract parameter
    parameter = cmd_params.substring(start + strlen(id), end);
    //be sure no extra space
    parameter.trim();
    return parameter;
}

#ifdef ENABLE_AUTHENTICATION

bool COMMANDS::isLocalPasswordValid(const char* password) {
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

//check admin password
bool COMMANDS::isadmin(String& cmd_params) {
    String adminpassword;
    String sadminPassword;
    Preferences prefs;
    prefs.begin(NAMESPACE, true);
    String defV = DEFAULT_ADMIN_PWD;
    sadminPassword = prefs.getString(ADMIN_PWD_ENTRY, defV);
    prefs.end();
    adminpassword = get_param(cmd_params, "pwd=", true);
    if (!sadminPassword.equals(adminpassword))
        return false;
    else
        return true;
}
//check user password - admin password is also valid
bool COMMANDS::isuser(String& cmd_params) {
    String userpassword;
    String suserPassword;
    Preferences prefs;
    prefs.begin(NAMESPACE, true);
    String defV = DEFAULT_USER_PWD;
    suserPassword = prefs.getString(USER_PWD_ENTRY, defV);
    prefs.end();
    userpassword = get_param(cmd_params, "pwd=", true);
    //it is not user password
    if (!suserPassword.equals(userpassword)) {
        //check admin password
        return isadmin(cmd_params);
    } else
        return true;
}
#endif

//check is valid [ESPXXX] command
//return XXX as cmd and command as cmd_params
bool COMMANDS::check_command(const char* line, int* cmd, String& cmd_params) {
    String buffer = line;
    bool result = false;
    int ESPpos = buffer.indexOf("[ESP");
    if (ESPpos > -1) {
        //is there the second part?
        int ESPpos2 = buffer.indexOf("]", ESPpos);
        if (ESPpos2 > -1) {
            //Split in command and parameters
            String cmd_part1 = buffer.substring(ESPpos + 4, ESPpos2);
            String cmd_part2 = "";
            //is there space for parameters?
            if (ESPpos2 < buffer.length())
                cmd_part2 = buffer.substring(ESPpos2 + 1);
            //if command is a valid number then execute command
            if (cmd_part1.toInt() >= 0) {
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
