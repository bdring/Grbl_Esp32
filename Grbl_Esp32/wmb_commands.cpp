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

namespace COMMANDS {

static ESPResponseStream *output;
static level_authenticate_type auth_type;

static bool restart_ESP_module = false;

// We create a variety of print functions to make the rest
// of the code more compact and readable.
static void print(const char *s)
{
    if (output) {
        output->print(s);
    }
}
static void print(string s)
{
    print(s.c_str);
}
static void println(const char *s)
{
    if (output) {
        output->println(s);
    }
}
static void println(string s)
{
    println(s.c_str);
}
static void println(const char *s, const char *s2)
{
    print(s);
    println(s2);
}
static void println(const char *s, String s2)
{
    println(s, s2.c_str);
}
static void print_mac(const char *s, string mac)
{
    print(s);
    print(" (");
    print(mac);
    println(")");
}

// Handle errors as follows:
// Initially call set_error(NULL).  Then, if an error occurs,
// call set_error(msg).  Only the first such error will be saved.
// If errmsg is NULL at the end, everything is okay.
static const char *errmsg = NULL;
static set_error(const char *msg)
{
    if (!msg || !errmsg) {
        errmsg = msg;
    }
}

static int check_auth(bool fail)
{
    if (!auth_enabled) {
        return false;
    }
    if (fail) {
        set_error("wrong authentication!");
    }
    return fail;
}

#define ADMIN_ONLY if (check_auth(auth_type != LEVEL_ADMIN)) { \
        set_error("must be an administrator"); return; }

#ifdef ENABLE_WIFI
// Can be executed as GUEST
void wifi_current_ip(string parameter) // 111
{
    String currentIP = cmd_params;
    if (WiFi.getMode() == WIFI_STA)
        currentIP += WiFi.localIP().toString();
    else
        currentIP += WiFi.softAPIP().toString();
    println(currentIP);
}
#endif

#if defined(ENABLE_WIFI) ||  defined(ENABLE_BLUETOOTH)
static void wifi_radio_enable(string parameter) // 115
{
    ADMIN_ONLY;
    if (parameter.length() == 0) {
        bool on = false;
#if defined (ENABLE_WIFI)
        if (WiFi.getMode() != WIFI_MODE_NULL)on = true;
#endif
#if defined (ENABLE_BLUETOOTH)
        if (bt_config.Is_BT_on())on = true;
#endif
        print_option(onoff_options, on);
    } else {
        int8_t on = option_value(parameter.toUpperCase().c_str());
        if (on == -1) {
            set_error("only ON or OFF mode supported!");
            return;
        }
        //Stop everything
#if defined (ENABLE_WIFI)
        if (WiFi.getMode() != WIFI_MODE_NULL)wifi_config.StopWiFi();
#endif
#if defined (ENABLE_BLUETOOTH)
        if (bt_config.Is_BT_on())bt_config.end();
#endif
        //if On start proper service
        if (on) { //On
            int8_t wifiMode = wifi_radio_mode.get();
            if ((wifiMode == ESP_WIFI_AP) || (wifiMode == ESP_WIFI_STA)) {
#if defined (ENABLE_WIFI)
                wifi_config.begin();
#else
                set_error("WiFi is not enabled!");
                return;
#endif
            }
            if (wifiMode == ESP_BT) {
#if defined (ENABLE_BLUETOOTH)
                bt_config.begin();
#else
                set_error("Bluetooth is not enabled!");
                return;
#endif
            }
            println("[MSG: Radio is Off]");
            return;
        }
        println("[MSG: Radio is Off]");
    }
}
#endif

static void sd_status(string parameter) // 200
{
    String resp = "No SD card";
#ifdef ENABLE_SD_CARD
    switch (get_sd_state(true)) {
        case SDCARD_IDLE:
            resp = "SD card detected";
            break;
        case SDCARD_NOT_PRESENT:
            resp = "No SD card";
            break;
        default:
            resp = "Busy";
    }
#endif
    println(resp);
}

#ifdef ENABLE_SD_CARD
static void sd_content(string parameter) // 200
{
    int8_t state = get_sd_state(true);
    if (state  ==  SDCARD_IDLE) {
        listDir(SD, "/", 10, output->client());
        String ssd = "[SD Free:" + ESPResponseStream::formatBytes(SD.totalBytes() - SD.usedBytes());
        ssd += " Used:" + ESPResponseStream::formatBytes(SD.usedBytes());
        ssd += " Total:" + ESPResponseStream::formatBytes(SD.totalBytes());
        ssd += "]";
        println("");
        println(ssd);
    } else
        println((state == SDCARD_NOT_PRESENT) ? "No SD card" : "Busy");
    }
}

static void sd_delete(string parameter) // 215
{
    if (parameter.length() == 0) {
        set_error("Missing file name!");
        return;
    }
    int8_t state = get_sd_state(true);
    parameter.trim();
    if (parameter[0] != '/')
        parameter = "/" + parameter;
    if (state  ==  SDCARD_IDLE) {
        File file2del = SD.open(parameter.c_str());
        if (!file2del) {
            set_error("Cannot stat file!");
            return;
        }
        if (file2del.isDirectory()) {
            if (!SD.rmdir((char*)parameter.c_str())) {
                set_error("Cannot delete directory! Is directory empty?");
                return;
            }
            println("Directory deleted.");
        } else {
            if (!SD.remove((char*)parameter.c_str())) {
                set_error("Cannot delete file!");
                return;
            }
            println("File deleted.");
        }
        file2del.close();
    }
    println((state == SDCARD_NOT_PRESENT) ? "No SD card" : "Busy");
}

static void sd_run(string parameter) // 220
{
    if (parameter.length() == 0) {
        set_error("Missing file name!");
        return;
    }
    int8_t state = get_sd_state(true);
    if (state  !=  SDCARD_IDLE) {
        println((state == SDCARD_NOT_PRESENT) ? "No SD card" : "Busy");
        return;
    }
    if (sys.state != STATE_IDLE) {
        println("Busy");
        return;
    }
    if (!openFile(SD, parameter.c_str())) {
        report_status_message(STATUS_SD_FAILED_READ, (output) ? output->client() : CLIENT_ALL);
        println("");
        return;
    }
    char fileLine[255];
    SD_client = (output) ? output->client() : CLIENT_ALL;
    if (!readFileLine(fileLine)) {
        //No need notification here it is just a macro
        closeFile();
        println("");
        return;
    }
    report_status_message(gc_execute_line(fileLine, (output) ? output->client() : CLIENT_ALL), (output) ? output->client() : CLIENT_ALL); // execute the first line
    report_realtime_status((output) ? output->client() : CLIENT_ALL);
    println("");
}
#endif

static Setting_t *display_list[] =
{
#ifdef ENABLE_WIFI
    wifi_hostname,
#ifdef ENABLE_HTTP
    http_enable,
    // XXX the JSON form uses Enabled/Disabled instead of ON OFF
#endif
#ifdef ENABLE_TELNET
    telnet_enable,
    // XXX the JSON form uses Enabled/Disabled instead of ON OFF
#endif
    wifi_radio_mode,
    wifi_sta_ssid,
    // XXX need min and max length in prefs strings
    wifi_sta_password,
    wifi_sta_mode,
    wifi_sta_ip,
    wifi_sta_gateway,
    wifi_sta_netmask,
    wifi_ap_ssid,
    wifi_ap_password,
    wifi_ap_ip,
    wifi_ap_channel, // XXX this is really an INT
#ifdef ENABLE_NOTIFICATIONS
    notification_type,
    notification_t1,
    notification_t2,
    notification_ts,
#endif //ENABLE_NOTIFICATIONS
#endif
    NULL,
};
#endif

static void esp3d_display() // 400
{
    JSONencoder j = new JSONencoder(output->client() != CLIENT_WEBUI);
    j->begin();
    j->begin_array("EEPROM");
    for (Setting_t *js = json_display_list; *js; js++) {
        js->addWebui(j);
    }
    j->end_array();
    println(j->end());
    delete j;
}

static void esp3d_set(string parameter) // 401
{
    // We do not need the "T=" (type) parameter because the
    // Setting objects know their own type
    const char *spos = get_param(parameter, "P=", false).cstr;
    const char *sval = get_param(parameter, "V=", true).cstr;
    if (*spos == '\0')
        return;
    do_setting(spos, sval);
}

#if defined (ENABLE_WIFI)
static void wifi_ap_list(string parameter) // 410
{
    JSONencoder j = new JSONencoder(output->client() != CLIENT_WEBUI);
    j->begin();
    j->begin_array("AP_LIST");
    j->line();
    int n = WiFi.scanComplete();
    if (n == -2)
        WiFi.scanNetworks(true);
    else if (n) {
        for (int i = 0; i < n; ++i) {
            j->begin_object();
            j->member("SSID", WiFi.SSID(i));
            j->member("SIGNAL", wifi_config.getSignal(WiFi.RSSI(i)));
            j->member("IS_PROTECTED", WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
            //            j->member("IS_PROTECTED", WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "0" : "1");
            j->end_object();
            j->line();
        }
    }
    WiFi.scanDelete();
    if (WiFi.scanComplete() == -2)
        WiFi.scanNetworks(true);
    j->end_array();
    print(j->end());
    delete j;
}
#endif

static void esp32_status(string parameter) // 420
{
    int mode = WiFi.getMode()
    if (!output)return false;
    println("Chip ID: ", String((uint16_t)(ESP.getEfuseMac() >> 32)));

    println("CPU Frequency: ", String(ESP.getCpuFreqMHz()) + "Mhz");

    print("CPU Temperature: ");
    print(String(temperatureRead(), 1));
    // if (output->client() == CLIENT_WEBUI)print("&deg;");
    print("C");
    println("");

    println("Free memory: ", ESPResponseStream::formatBytes(ESP.getFreeHeap()));

    println("SDK: ", ESP.getSdkVersion());

    println("Flash Size: ", ESPResponseStream::formatBytes(ESP.getFlashChipSize()));

#if defined (ENABLE_WIFI)
    if (mode != WIFI_MODE_NULL) {
        //Is OTA available ?
        size_t flashsize = 0;
        if (esp_ota_get_running_partition()) {
            const esp_partition_t* partition = esp_ota_get_next_update_partition(NULL);
            if (partition)
                flashsize = partition->size;
        }
        println("Available Size for update: ", ESPResponseStream::formatBytes(flashsize));
    }
    if (mode != WIFI_MODE_NULL) {
        println("Available Size for SPIFFS: ", ESPResponseStream::formatBytes(SPIFFS.totalBytes()));
    }
#endif
    long br = Serial.baudRate();
    //workaround for ESP32
    if (br == 115201)
        br = 115200;
    if (br == 230423)
        br = 230400;
    println("Baud rate: ", String(br));

    println("Sleep mode: ", WiFi.getSleep() ? "Modem" : "None");

#if defined (ENABLE_WIFI)
#if defined (ENABLE_HTTP)
    if (mode != WIFI_MODE_NULL) {
        println("Web port: ", String(web_server.port()));
    }
#endif

#if defined (ENABLE_TELNET)
    if (mode != WIFI_MODE_NULL) {
        println("Data port: ", String(telnet_server.port()));
    }
#endif

    if (mode != WIFI_MODE_NULL) {
        println("Hostname: ", wifi_config.Hostname());
    }


    print("Current WiFi Mode: ");
    if (mode == WIFI_STA) {
        print_mac("STA", WiFi.macAddress());

        print("Connected to: ");
        if (WiFi.isConnected()) { //in theory no need but ...
            print(WiFi.SSID());
            println("");

            print("Signal: ", String(wifi_config.getSignal(WiFi.RSSI())));
            print("%");
            println("");

            uint8_t PhyMode;
            esp_wifi_get_protocol(ESP_IF_WIFI_STA, &PhyMode);
            print("Phy Mode: ");
            if (PhyMode == (WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N)) print("11n");
            else if (PhyMode == (WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G)) print("11g");
            else if (PhyMode == (WIFI_PROTOCOL_11B)) print("11b");
            else print("???");
            println("");

            println("Channel: ", String(WiFi.channel()));

            tcpip_adapter_dhcp_status_t dhcp_status;
            tcpip_adapter_dhcpc_get_status(TCPIP_ADAPTER_IF_STA, &dhcp_status);
            println("IP Mode: ", dhcp_status == TCPIP_ADAPTER_DHCP_STARTED ? "DHCP" : "Static");
            println("IP: ", WiFi.localIP().toString());
            println("Gateway: ", WiFi.gatewayIP().toString());
            println("Mask: ", WiFi.subnetMask().toString());
            println("DNS: ", WiFi.dnsIP().toString());

        } //this is web command so connection => no command
        print("Disabled Mode: ");
        print_mac("AP", WiFi.softAPmacAddress());
    } else if (mode == WIFI_AP) {
        print_mac("AP", WiFi.softAPmacAddress());

        wifi_config_t conf;
        esp_wifi_get_config(ESP_IF_WIFI_AP, &conf);
        println("SSID: ", (const char*) conf.ap.ssid);
        println("Visible: ", (conf.ap.ssid_hidden == 0) ? "Yes" : "No");

        const char *mode;
        switch (conf.ap.authmode) {
            case WIFI_AUTH_OPEN:     mode = "None"; break;
            case WIFI_AUTH_WEP:      mode = "WEP";  break;
            case WIFI_AUTH_WPA_PSK:  mode = "WPA";  break;
            case WIFI_AUTH_WPA2_PSK: mode = "WPA2"; break;
            default:                 mode = "WPA/WPA2";
        }

        println("Authentication: ", mode);
        println("Max Connections: ", String(conf.ap.max_connection));

        tcpip_adapter_dhcp_status_t dhcp_status;
        tcpip_adapter_dhcps_get_status(TCPIP_ADAPTER_IF_AP, &dhcp_status);
        println("DHCP Server: ", dhcp_status == TCPIP_ADAPTER_DHCP_STARTED ? "Started" : "Stopped");

        println("IP: ", WiFi.softAPIP().toString());

        tcpip_adapter_ip_info_t ip_AP;
        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip_AP);
        println("Gateway: ", IPAddress(ip_AP.gw.addr).toString());
        print("Mask: ", IPAddress(ip_AP.netmask.addr).toString());


        wifi_sta_list_t station;
        tcpip_adapter_sta_list_t tcpip_sta_list;
        esp_wifi_ap_get_sta_list(&station);
        tcpip_adapter_get_sta_list(&station, &tcpip_sta_list);
        println("Connected clients: ", String(station.num));

        for (int i = 0; i < station.num; i++) {
            print(wifi_config.mac2str(tcpip_sta_list.sta[i].mac));
            print(" ");
            print(IPAddress(tcpip_sta_list.sta[i].ip.addr).toString());
            println("");
        }
        print("Disabled Mode: ");
        print_mac("STA", WiFi.macAddress());
    } else if (mode == WIFI_AP_STA) { //we should not be in this state but just in case ....
        println("Mixed");

        print_mac("STA", WiFi.macAddress());
        print_mac("AP", WiFi.softAPmacAddress());
    } else { //we should not be there if no wifi ....
        println("Off");
    }
#endif
#ifdef ENABLE_BLUETOOTH
    print("Current BT Mode: ");
    if (bt_config.Is_BT_on()) {
        println("On");

        print("BT Name: ");
        print(bt_config.BTname());
        print("(");
        print(bt_config.device_address());
        println(")");

        print("Status: ");
        if (SerialBT.hasClient()) {
            println("Connected with ", bt_config._btclient);
        } else
            println("Not connected");
    } else
        println("Off");
#endif
#ifdef ENABLE_NOTIFICATIONS
    print("Notifications: ");
    print(notificationsservice.started() ? "Enabled" : "Disabled");
    if (notificationsservice.started()) {
        print("(");
        print(notificationsservice.getTypeString());
        print(")");
    }
    println("");
#endif
    //TODO to complete
    print("FW version: ");
    print(GRBL_VERSION);
    print(" (");
    print(GRBL_VERSION_BUILD);
    print(") (ESP32)");
    println("");
}

static void esp32_restart(string parameter) // 444
{
    ADMIN_ONLY;
    {
        if (parameter != "RESTART") {
            return;
        }
        grbl_send(CLIENT_ALL, "[MSG:Restart ongoing]\r\n");
        restart_ESP();
    }
    println(response ? "ok" : "Error: Incorrect Command");
}

#ifdef ENABLE_NOTIFICATIONS
void esp3d_message(string parameter) // 600
{
    if (parameter.length() == 0) {
        set_error("Invalid message!");
        return;
    }
    if (!notificationsservice.sendMSG("GRBL Notification", parameter.c_str())) {
        set_error("Cannot send message!");
        return;
    }
}

void esp3d_notification(string parameter) // 610
{
    //Get prints type and settings only, not the protected T1/T2
    if (parameter.length() == 0) {
        print(notification_type.getStringValue());
        println(" ", notification_ts.getStringValue());
        return;
    }
    //Set values depending on which keywords are present
    String value;
    value = get_param(parameter, "type=", false);
    if (value.length() != 0) {
        if (notification_type.setStringValue(value)) {
            set_error("set failed!");
            return;
        }
    }
    value = get_param(parameter, "T1=", false);
    if (value.length() != 0 && notification_t1.setStringValue(value)) {
        set_error("token 1!");
        return;
    }
    value = get_param(parameter, "T2=", false);
    if (value.length() != 0 && notification_t1.setStringValue(value)) {
        set_error("token 2!");
        return;
    }
    value = get_param(parameter, "TS=", false);
    if (value.length() != 0 && notification_ts.setStringValue(value)) {
        set_error("settings!");
        return;
    }
     //update settings
    notificationsservice.begin();
}
#endif //ENABLE_NOTIFICATIONS

void fs_macro(string parameter) // 700
{
    if ((parameter.length() > 0) && (parameter[0] != '/'))
        parameter = "/" + parameter;
    if (!SPIFFS.exists(parameter)) {
        set_error("no such file!");
        return;
    }
    File currentfile = SPIFFS.open(parameter, FILE_READ);
    if (!currentfile) { //if file open success
        set_error("file open failed");
        return;
    }
    //until no line in file
    while (currentfile.available()) {
        String currentline = currentfile.readStringUntil('\n');
        currentline.replace("\n", "");
        currentline.replace("\r", "");
        if (currentline.length() > 0) {
            int ESPpos = currentline.indexOf("[ESP");
            if (ESPpos > -1) {
                //is there the second part?
                int ESPpos2 = currentline.indexOf("]", ESPpos);
                if (ESPpos2 > -1) {
                    //Split in command and parameters
                    String cmd_part1 = currentline.substring(ESPpos + 4, ESPpos2);
                    String cmd_part2 = "";
                    //is there space for parameters?
                    if (ESPpos2 < currentline.length())
                        cmd_part2 = currentline.substring(ESPpos2 + 1);
                    //if command is a valid number then execute command
                    if (cmd_part1.toInt() >= 0) {
                        execute_internal_command(cmd_part1.toInt(), cmd_part2);
                    }
                    //if command is not a valid [ESPXXX] command ignore it
                }
            } else {
                //preprocess line
                String processedline = "";
                char c;
                uint8_t line_flags = 0;
                for (uint16_t index = 0; index < currentline.length(); index++) {
                    c = currentline[index];
                    if (c == '\r' || c == ' ' || c == '\n') {
                        // ignore these whitespace items
                    } else if (c == '(')
                        line_flags |= LINE_FLAG_COMMENT_PARENTHESES;
                    else if (c == ')') {
                        // End of '()' comment. Resume line allowed.
                        if (line_flags & LINE_FLAG_COMMENT_PARENTHESES)  line_flags &= ~(LINE_FLAG_COMMENT_PARENTHESES);
                    } else if (c == ';') {
                        // NOTE: ';' comment to EOL is a LinuxCNC definition. Not NIST.
                        if (!(line_flags & LINE_FLAG_COMMENT_PARENTHESES)) // semi colon inside parentheses do not mean anything
                            line_flags |= LINE_FLAG_COMMENT_SEMICOLON;
                    } else { // add characters to the line
                        if (!line_flags) {
                            c = toupper(c); // make upper case
                            processedline += c;
                        }
                    }
                }
                if (processedline.length() > 0)gc_execute_line((char*)processedline.c_str(), CLIENT_WEBUI);
                wait(1);
            }
            wait(1);
        }
    }
    currentfile.close();
}

void fs_format(string parameter) // 710
{
    if (parameter != "FORMAT") {
        set_error("error");
        return;
    }
    print("Formatting");
    SPIFFS.format();
    println("...Done");
}

void fs_size(string parameter) // 720
{
    print(parameter);
    print("SPIFFS  Total:");
    print(ESPResponseStream::formatBytes(SPIFFS.totalBytes()));
    println(" Used:", ESPResponseStream::formatBytes(SPIFFS.usedBytes()));
}

// This one can be executed as GUEST
void fw_info(string parameter) // 800
{
    String resp;
    resp = "FW version:";
    resp += GRBL_VERSION ;
    resp += " (";
    resp += GRBL_VERSION_BUILD;
    resp += ")";
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
    resp += ":";
    switch(mode) {
        case WIFI_MODE_AP:
        resp += WiFi.softAPIP().toString();
        break;
        case WIFI_MODE_STA:
        resp += WiFi.localIP().toString();
        break;
        case WIFI_MODE_APSTA:
        resp += WiFi.softAPIP().toString();
        break;
        default:

        resp += "0.0.0.0";
    }
#endif
    resp += " # hostname:";
    resp += wifi_config.Hostname();
    if (WiFi.getMode() == WIFI_AP)resp += "(AP mode)";
#endif
    //to save time in decoding `?`
    resp += " # axis:";
    resp += String(N_AXIS);
    println(resp);
}

typedef struct {
    int number;
    void (*function)();
  const char *doc;
} wifi_cmd_t;

// The only functions that can be executed as GUEST are
// wifi_current_ip and fw_info

wifi_cmd_t wifi_cmds[] = {
#ifdef ENABLE_WIFI
    100, wifi_sta_ssid, "(SSID) - display/set STA SSID",
    101, wifi_sta_password, "(Password) - set STA password",
    102, wifi_sta_mode, "(Mode) - display/set STA IP mode (DHCP/STATIC)",
    103, wifi_sta_ip_gw_mk,   "(IP=xxxx MSK=xxxx GW=xxxx) - display/set STA IP/Mask/GW",
    105, wifi_ap_ssid,  "(SSID) - display/set AP SSID",
    106, wifi_ap_password, "(Password) - set AP password",
    107, wifi_ap_ip, "(IP) - display/set AP IP",
    108, wifi_ap_channel, "(Channel) - display/set AP channel",
#endif
#if defined( ENABLE_WIFI) ||  defined( ENABLE_BLUETOOTH)
    110, wifi_radio_mode, "(State) - display/set radio state which can be STA, AP, BT, OFF",
#endif
#ifdef ENABLE_WIFI
    111, wifi_current_ip, "display current IP",
    112, wifi_hostname, "(Hostname) - display/set Hostname",
#endif
#if defined (ENABLE_WIFI) || defined (ENABLE_BLUETOOTH)
    115, wifi_radio_enable, "(State) - display/set immediate radio state which can be ON, OFF",
#endif
#ifdef ENABLE_WIFI
    120, http_enable, "(State) - display/set HTTP state which can be ON, OFF",
    121, http_port, "(Port) - display/set HTTP port ",
    130, telnet_enable, "(State) - display/set Telnet state which can be ON, OFF",
    131, telnet_port, "(Port) - display/set Telnet port ",
#endif
#ifdef ENABLE_BLUETOOTH
    140, bt_name, "(Bluetooth name) - display/set Bluetooth name",
#endif
    200, sd_status, " - display SD Card Status",
#ifdef ENABLE_SD_CARD
    210, sd_content, " - display SD Card content",
    215, sd_delete, "(file/dir name) - delete SD Card file / directory",
    220, sd_run, "(file name) - run file from SD",
#endif
    400, esp3d_display, " - display ESP3D settings in JSON",
    401, esp3d_set, "P=(position) T=(type) V=(value) - Set specific setting",
#if defined (ENABLE_WIFI)
    410, wifi_ap_list, " - display available AP list (limited to 30) in JSON",
#endif
    420, esp3d_status, " - display ESP3D current status",
    444, esp32_restart, "RESTART - Restart ESP",
#ifdef ENABLE_AUTHENTICATION
    555, esp32_password, "(Password) - set user password",
#endif //ENABLE_AUTHENTICATION
#ifdef ENABLE_NOTIFICATIONS
    600, esp3d_message, "(message) - send message",
    610, esp3d_notification, "type=(NONE/PUSHOVER/EMAIL/LINE) T1=(token1) T2=(token2) TS=(Settings) - display/set Notification settings",
#endif //ENABLE_NOTIFICATIONS
    700, fs_macro, "(file name) - run macro file from ESP Filesystem",
    710, fs_format, "FORMAT - Format ESP Filesystem",
    720, fs_size, "display total size and used size of ESP Filesystem",
    800, fw_info, " - display FW Informations",
  -1, "",
};
/*
 * delay is to avoid with asyncwebserver and may need to wait sometimes
 */
void wait(uint32_t milliseconds) {
    uint32_t timeout = millis();
    esp_task_wdt_reset(); //for a wait 0;
    //wait feeding WDT
    while ((millis() - timeout) < milliseconds)
        esp_task_wdt_reset();
}

static bool execute_internal_command(int cmd, String cmd_params) {
#ifdef ENABLE_AUTHENTICATION
    if (isadmin(cmd_params))
        auth_type = LEVEL_ADMIN;
    if (isuser(cmd_params) && (auth_type != LEVEL_ADMIN))
        auth_type = LEVEL_USER;
#endif
    String parameter;
    error = NULL;
    wifi_cmd_t *p;
    if (cmd == 0) {
        println("[List of ESP3D commands]");
        println("[ESP] - display this help");
        for (p = wifi_cmds; p->number != -1; p++) {
            println("[ESP%d]%s", p->number, p->doc);
        }
    } else {
        string rawParameter = get_param(cmd_params, "", true);
        if (cmd != cmd != 800 && cmd != 111) {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type == LEVEL_GUEST) {
                set_error("must not be a guest");
            } else if (auth_type != LEVEL_ADMIN && rawParameter.length() != 0) {
                set_error("must be an administrator");
            }
        }
#endif

        if (!error) {
            for (p = wifi_cmds; p->number != -1; p++) {
                if (p->number == cmd) {
                    p->function(rawParameter);
                    break;
                }
            }
            if (p->number == -1) {
                set_error("Incorrect Command");
            }
        }
    }
}

bool execute_internal_command(int cmd, String cmd_params, level_authenticate_type auth_level,  ESPResponseStream* outputStream) {
    if (!outputStream)
        return false;
    set_error(NULL);
    output = outputStream;
    auth_type = auth_level;
    execute_internal_command(cmd, cmd_params);
    println(errmsg ? errmsg : "ok");
    return !errmsg;
}

String get_param(String& cmd_params, const char* id, bool withspace) {
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
bool isLocalPasswordValid(const char* password) {
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
bool isadmin(String& cmd_params) {
    String sadminPassword = get_prefs_string(ADMIN_PWD_ENTRY, DEFAULT_ADMIN_PWD);
    String adminpassword = get_param(cmd_params, "pwd=", true);
    return sadminPassword.equals(adminpassword);
}

//check user password - admin password is also valid
bool isuser(String& cmd_params) {
    String suserPassword = get_prefs_string(USER_PWD_ENTRY, DEFAULT_USER_PWD);
    String userpassword = get_param(cmd_params, "pwd=", true);
    // true if either user or admin password
    return suserPassword.equals(userpassword) ? true : isadmin(cmd_params);
}
#endif

//check is valid [ESPXXX] command
//return XXX as cmd and command as cmd_params
bool check_command(const char* line, int* cmd, String& cmd_params) {
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
void restart_ESP() {
    restart_ESP_module = true;
}

/**
 * Handle not critical actions that must be done in sync environement
 */
void handle() {
    wait(0);
    //in case of restart requested
    if (restart_ESP_module) {
        ESP.restart();
        while (1)  ;
    }
}
}
