/*
  WebSetting.cpp - Settings and Commands for Grbl_ESP32's interface
  to ESP3D_WebUI.  Code snippets extracted from commands.cpp in the
  old WebUI interface code are presented via the Settings class.

  Copyright (c) 2020 Mitch Bradley
  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "grbl.h"

#include <WiFi.H>
#include <FS.h>
#include <SPIFFS.h>
#include <esp_wifi.h>
#include <esp_ota_ops.h>

#ifndef WIFI_OR_BLUETOOTH
void make_web_settings() {}
#else
#include "SettingsClass.h"
#include "GCodePreprocessor.h"
#include "espresponse.h"
#include "web_server.h"
#include "string.h"

StringSetting* wifi_sta_ssid;
StringSetting* wifi_sta_password;

#ifdef ENABLE_WIFI
EnumSetting*   wifi_sta_mode;
IPaddrSetting* wifi_sta_ip;
IPaddrSetting* wifi_sta_gateway;
IPaddrSetting* wifi_sta_netmask;

StringSetting* wifi_ap_ssid;
StringSetting* wifi_ap_password;

IPaddrSetting* wifi_ap_ip;

IntSetting* wifi_ap_channel;

StringSetting* wifi_hostname;
EnumSetting* http_enable;
IntSetting* http_port;
EnumSetting* telnet_enable;
IntSetting* telnet_port;
typedef std::map<const char *, int8_t, cmp_str> enum_opt_t;
enum_opt_t staModeOptions = {
    { "DHCP",   DHCP_MODE , },
    { "Static", STATIC_MODE , },
};
#endif

#ifdef WIFI_OR_BLUETOOTH
EnumSetting* wifi_radio_mode;
enum_opt_t radioOptions = {
    { "None", ESP_RADIO_OFF, },
    { "STA", ESP_WIFI_STA, },
    { "AP", ESP_WIFI_AP, },
    { "BT", ESP_BT, },
};
enum_opt_t radioEnabledOptions = {
    { "NONE", ESP_RADIO_OFF, },
#ifdef ENABLE_WIFI
    { "STA", ESP_WIFI_STA, },
    { "AP", ESP_WIFI_AP, },
#endif
#ifdef ENABLE_BLUETOOTH
    { "BT", ESP_BT, },
#endif
};
#endif

#ifdef ENABLE_BLUETOOTH
StringSetting* bt_name;
#endif

#ifdef ENABLE_AUTHENTICATION
// TODO Settings - need ADMIN_ONLY and if it is called without a parameter it sets the default
StringSetting* user_password;
StringSetting* admin_password;
#endif

#ifdef ENABLE_NOTIFICATIONS
enum_opt_t notificationOptions = {
    { "NONE", 0, },
    { "LINE", 3, },
    { "PUSHOVER", 1, },
    { "EMAIL", 2, },
};
EnumSetting* notification_type;
StringSetting* notification_t1;
StringSetting* notification_t2;
StringSetting* notification_ts;
#endif

enum_opt_t onoffOptions = {
  { "OFF", 0, },
  { "ON", 1, }
};

static ESPResponseStream* espresponse;

char* get_param(char *parameter, const char *key, bool allowSpaces) {
    String paramStr = String((const char *)parameter);
    int pos = paramStr.indexOf(key);
    char *value = parameter + pos + strlen(key);
    if (!allowSpaces) {
        // For no-space values like with T= and P=,
        // the value string starts right after = and
        // ends at the first space.
        char* end = strchr(value, ' ');
        if (end) {
            *end = '\0';
        }
    }
    // For space-allowed values like with V=, the value
    // starts after the = and continues to the string end.
    // Any trailing " pwd=<password>" part has already been
    // removed.
    return value;
}

const char* remove_password(char *parameter) {
    String paramStr = String((const char*)parameter);
    int pos = paramStr.indexOf("pwd=");
    if (pos == -1) {
        return NULL;
    }
    if (pos > 0 && parameter[pos-1] == ' ') {
        parameter[pos-1] = '\0';
    } else {
        parameter[pos] = '\0';
    }
    return parameter + pos + strlen("pwd=");
}

err_t WebCommand::action(char* value, ESPResponseStream* out) {
    char empty = '\0';
    if (!value) {
        value = &empty;
    }
    espresponse = out;
    // TODO Settings - authenticate
    password = remove_password(value);
    #ifdef ENABLE_AUTHENTICATION
        switch (_group) {
        case WEBCMDRU: // Read, needs user or admin password
            TBD
        case WEBCMDWU: // Write, needs user or admin password
        case WEBCMDWA: // Write, needs admin passord
        }
    #endif
    return _action(value);
};

static int webColumn = 0;
// We create a variety of print functions to make the rest
// of the code more compact and readable.
static void webPrint(const char *s)
{
    if (espresponse) {
        espresponse->print(s);
        webColumn += strlen(s);
    }
}
static void webPrintSetColumn(int column) {
    while (webColumn < column) {
        webPrint(" ");
    }
}
static void webPrint(String s)
{
    webPrint(s.c_str());
}
static void webPrint(const char *s1, const char *s2)
{
    webPrint(s1);
    webPrint(s2);
}
static void webPrint(const char *s1, String s2)
{
    webPrint(s1);
    webPrint(s2.c_str());
}
static void webPrint(const char *s1, const char *s2, const char *s3)
{
    webPrint(s1);
    webPrint(s2);
    webPrint(s3);
}
static void webPrintln(const char *s)
{
    webPrint(s);
    webPrint("\r\n");
    webColumn = 0;
}
static void webPrintln(String s)
{
    webPrintln(s.c_str());
}
static void webPrintln(const char *s1, const char *s2)
{
    webPrint(s1);
    webPrintln(s2);
}
static void webPrintln(const char *s, IPAddress ip)
{
    webPrint(s);
    webPrintln(ip.toString().c_str());
}
static void webPrintln(const char *s, String s2)
{
    webPrintln(s, s2.c_str());
}

static char *trim(char *str)
{
    char *end;
    // Trim leading space
    while(isspace((unsigned char)*str)) {
        str++;
    }
    if(*str == 0)  {  // All spaces?
        return str;
    }
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) {
        end--;
    }
    // Write new null terminator character
    end[1] = '\0';
    return str;
}

static void print_mac(const char *s, String mac)
{
    webPrint(s);
    webPrint(" (");
    webPrint(mac);
    webPrintln(")");
}

static err_t showFwInfo(char *parameter) { // ESP800
    webPrint("FW version:" GRBL_VERSION " (" GRBL_VERSION_BUILD ")" " # FW target:grbl-embedded  # FW HW:");
    #ifdef ENABLE_SD_CARD
        webPrint("Direct SD");
    #else
        webPrint("No SD");
    #endif
    webPrint("  # primary sd:/sd # secondary sd:none # authentication:");
#ifdef ENABLE_AUTHENTICATION
    webPrint("yes");
#else
    webPrint("no");
#endif
#if defined (ENABLE_WIFI)
    #if defined (ENABLE_HTTP)
        webPrint(" # webcommunication: Sync: ", String(web_server.port() + 1));
        webPrint(":");
        switch (WiFi.getMode()) {
        case WIFI_MODE_AP:
            webPrint(WiFi.softAPIP().toString());
            break;
        case WIFI_MODE_STA:
            webPrint(WiFi.localIP().toString());
            break;
        case WIFI_MODE_APSTA:
            webPrint(WiFi.softAPIP().toString());
            break;
        default:
            webPrint("0.0.0.0");
            break;
        }
    #endif
        webPrint(" # hostname:", wifi_config.Hostname());
        if (WiFi.getMode() == WIFI_AP) {
            webPrint("(AP mode)");
        }
#endif
        //to save time in decoding `?`
        webPrintln(" # axis:", String(N_AXIS));
    return STATUS_OK;
}

static err_t SPIFFSSize(char *parameter) { // ESP720
    webPrint(parameter);
    webPrint("SPIFFS  Total:", ESPResponseStream::formatBytes(SPIFFS.totalBytes()));
    webPrintln(" Used:", ESPResponseStream::formatBytes(SPIFFS.usedBytes()));
    return STATUS_OK;
}

static err_t formatSpiffs(char *parameter) { // ESP710
    if (strcmp(parameter, "FORMAT") != 0) {
        webPrintln("Parameter must be FORMAT");
        return STATUS_INVALID_VALUE;
    }
    webPrint("Formatting");
    SPIFFS.format();
    webPrintln("...Done");
    return STATUS_OK;
}

static err_t runFile(char *parameter) { // ESP700
    String path = trim(parameter);
    if ((path.length() > 0) && (path[0] != '/')) {
        path = "/" + path;
    }
    if (!SPIFFS.exists(path)) {
        webPrintln("");
        return STATUS_SD_FILE_NOT_FOUND;
    }
    File currentfile = SPIFFS.open(parameter, FILE_READ);
    if (!currentfile) {//if file open success
        return STATUS_SD_FAILED_OPEN_FILE;
    }
    //until no line in file
    while (currentfile.available()) {
        String currentline = currentfile.readStringUntil('\n');
        currentline.replace("\n", "");
        currentline.replace("\r", "");
        if (currentline.length() > 0) {
            // TODO Settings - feed into command interpreter
            // while accumulating error codes
        }
    }
    currentfile.close();
    return STATUS_OK;
}

#ifdef ENABLE_NOTIFICATIONS
static err_t showSetNotification(char *parameter) { // ESP610
    if (*parameter == '\0') {
        webPrint("", notification_type->getStringValue());
        webPrintln(" ", notification_ts->getStringValue());
        return STATUS_OK;
    }
    char *ts = get_param(parameter, "TS=", false);
    char *t2 = get_param(parameter, "T2=", false);
    char *t1 = get_param(parameter, "T1=", false);
    char *ty = get_param(parameter, "type=", false);
    err_t err = notification_type->setStringValue(ty);
    if (!err) {
        err = notification_t1->setStringValue(t1);
    }
    if (!err) {
        err = notification_t2->setStringValue(t2);
    }
    if (!err) {
        err = notification_ts->setStringValue(ts);
    }
    return err;

}

static err_t sendMessage(char *parameter) { // ESP600
    if (*parameter == '\0') {
        webPrintln("Invalid message!");
        return STATUS_INVALID_VALUE;
    }
    if (!notificationsservice.sendMSG("GRBL Notification", parameter)) {
        webPrintln("Cannot send message!");
        return STATUS_MESSAGE_FAILED;
    }
    return STATUS_OK;
}
#endif

#ifdef ENABLE_AUTHENTICATION
static err_t setUserPassword(char *parameter) { // ESP555
    if (*parameter == '\0') {
        user_password->setDefault();
        return STATUS_OK;
    }
    if (user_password->setStringValue(parameter)) {
        webPrintln("Invalid Password");
        return STATUS_INVALID_VALUE;
    }
    return STATUS_OK;
}
#endif

static err_t setSystemMode(char *parameter) { // ESP444
    parameter = trim(parameter);
    if (strcasecmp(parameter, "RESTART") != 0) {
        webPrintln("Incorrect command");
        return STATUS_INVALID_VALUE;
    }
    grbl_send(CLIENT_ALL, "[MSG:Restart ongoing]\r\n");
    COMMANDS::restart_ESP();
    return STATUS_OK;
}

static err_t showSysStats(char *parameter) { // ESP420
    webPrintln("Chip ID: ", String((uint16_t)(ESP.getEfuseMac() >> 32)));
    webPrintln("CPU Frequency: ", String(ESP.getCpuFreqMHz()) + "Mhz");
    webPrintln("CPU Temperature: ", String(temperatureRead(), 1) + "C");
    webPrintln("Free memory: ", ESPResponseStream::formatBytes(ESP.getFreeHeap()));
    webPrintln("SDK: ", ESP.getSdkVersion());
    webPrintln("Flash Size: ", ESPResponseStream::formatBytes(ESP.getFlashChipSize()));

    // Round baudRate to nearest 100 because ESP32 can say e.g. 115201
    webPrintln("Baud rate: ", String((Serial.baudRate()/100) * 100));
    webPrintln("Sleep mode: ", WiFi.getSleep() ? "Modem" : "None");

#ifdef ENABLE_WIFI
    int mode = WiFi.getMode();
    if (mode != WIFI_MODE_NULL) {
        //Is OTA available ?
        size_t flashsize = 0;
        if (esp_ota_get_running_partition()) {
            const esp_partition_t* partition = esp_ota_get_next_update_partition(NULL);
            if (partition)
                flashsize = partition->size;
        }
        webPrintln("Available Size for update: ", ESPResponseStream::formatBytes(flashsize));
        webPrintln("Available Size for SPIFFS: ", ESPResponseStream::formatBytes(SPIFFS.totalBytes()));

#if defined (ENABLE_HTTP)
        webPrintln("Web port: ", String(web_server.port()));
#endif
#if defined (ENABLE_TELNET)
        webPrintln("Data port: ", String(telnet_server.port()));
#endif
        webPrintln("Hostname: ", wifi_config.Hostname());
    }

    webPrint("Current WiFi Mode: ");
    switch (mode) {
    case WIFI_STA:
        print_mac("STA", WiFi.macAddress());

        webPrint("Connected to: ");
        if (WiFi.isConnected()) { //in theory no need but ...
            webPrintln(WiFi.SSID());
            webPrintln("Signal: ", String(wifi_config.getSignal(WiFi.RSSI())) + "%");

            uint8_t PhyMode;
            esp_wifi_get_protocol(ESP_IF_WIFI_STA, &PhyMode);
            const char *modeName;
            switch (PhyMode) {
                case WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N:
                    modeName = "11n";
                    break;
                case WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G:
                    modeName = "11g";
                    break;
                case WIFI_PROTOCOL_11B:
                    modeName = "11b";
                    break;
                default:
                    modeName = "???";
            }
            webPrintln("Phy Mode: ", modeName);

            webPrintln("Channel: ", String(WiFi.channel()));

            tcpip_adapter_dhcp_status_t dhcp_status;
            tcpip_adapter_dhcpc_get_status(TCPIP_ADAPTER_IF_STA, &dhcp_status);
            webPrintln("IP Mode: ", dhcp_status == TCPIP_ADAPTER_DHCP_STARTED ? "DHCP" : "Static");
            webPrintln("IP: ", WiFi.localIP());
            webPrintln("Gateway: ", WiFi.gatewayIP());
            webPrintln("Mask: ", WiFi.subnetMask());
            webPrintln("DNS: ", WiFi.dnsIP());

        } //this is web command so connection => no command
        webPrint("Disabled Mode: ");
        print_mac("AP", WiFi.softAPmacAddress());
        break;
    case WIFI_AP:
        print_mac("AP", WiFi.softAPmacAddress());

        wifi_config_t conf;
        esp_wifi_get_config(ESP_IF_WIFI_AP, &conf);
        webPrintln("SSID: ", (const char*) conf.ap.ssid);
        webPrintln("Visible: ", (conf.ap.ssid_hidden == 0) ? "Yes" : "No");

        const char *mode;
        switch (conf.ap.authmode) {
            case WIFI_AUTH_OPEN:     mode = "None"; break;
            case WIFI_AUTH_WEP:      mode = "WEP";  break;
            case WIFI_AUTH_WPA_PSK:  mode = "WPA";  break;
            case WIFI_AUTH_WPA2_PSK: mode = "WPA2"; break;
            default:                 mode = "WPA/WPA2";
        }

        webPrintln("Authentication: ", mode);
        webPrintln("Max Connections: ", String(conf.ap.max_connection));

        tcpip_adapter_dhcp_status_t dhcp_status;
        tcpip_adapter_dhcps_get_status(TCPIP_ADAPTER_IF_AP, &dhcp_status);
        webPrintln("DHCP Server: ", dhcp_status == TCPIP_ADAPTER_DHCP_STARTED ? "Started" : "Stopped");

        webPrintln("IP: ", WiFi.softAPIP());

        tcpip_adapter_ip_info_t ip_AP;
        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip_AP);
        webPrintln("Gateway: ", IPAddress(ip_AP.gw.addr));
        webPrintln("Mask: ", IPAddress(ip_AP.netmask.addr));

        wifi_sta_list_t station;
        tcpip_adapter_sta_list_t tcpip_sta_list;
        esp_wifi_ap_get_sta_list(&station);
        tcpip_adapter_get_sta_list(&station, &tcpip_sta_list);
        webPrintln("Connected clients: ", String(station.num));

        for (int i = 0; i < station.num; i++) {
            webPrint(wifi_config.mac2str(tcpip_sta_list.sta[i].mac));
            webPrintln(" ", IPAddress(tcpip_sta_list.sta[i].ip.addr));
        }
        webPrint("Disabled Mode: ");
        print_mac("STA", WiFi.macAddress());
        break;
    case WIFI_AP_STA: //we should not be in this state but just in case ....
        webPrintln("Mixed");

        print_mac("STA", WiFi.macAddress());
        print_mac("AP", WiFi.softAPmacAddress());
        break;
    default: //we should not be there if no wifi ....
        webPrintln("Off");
        break;
    }
#endif // ENABLE_WIFI
#ifdef ENABLE_BLUETOOTH
    webPrint("Current BT Mode: ");
    if (bt_config.Is_BT_on()) {
        webPrintln("On");

        webPrint("BT Name: ");
        webPrint(bt_config.BTname());
        webPrint("(");
        webPrint(bt_config.device_address());
        webPrintln(")");

        webPrint("Status: ");
        if (SerialBT.hasClient()) {
            webPrintln("Connected with ", bt_config._btclient);
        } else
            webPrintln("Not connected");
    } else
        webPrintln("Off");
#endif
#ifdef ENABLE_NOTIFICATIONS
    webPrint("Notifications: ");
    webPrint(notificationsservice.started() ? "Enabled" : "Disabled");
    if (notificationsservice.started()) {
        webPrint("(");
        webPrint(notificationsservice.getTypeString());
        webPrint(")");
    }
    webPrintln("");
#endif
    webPrint("FW version: ");
    webPrint(GRBL_VERSION);
    webPrint(" (");
    webPrint(GRBL_VERSION_BUILD);
    webPrint(") (ESP32)");
    webPrintln("");
    return STATUS_OK;
}

#ifdef ENABLE_WIFI
static err_t listAPs(char *parameter) { // ESP410
    JSONencoder* j = new JSONencoder(espresponse->client() != CLIENT_WEBUI);
    j->begin();
    j->begin_array("AP_LIST");
    //j->line();
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
            //j->line();
        }
    }
    WiFi.scanDelete();
    if (WiFi.scanComplete() == -2)
        WiFi.scanNetworks(true);
    j->end_array();
    webPrint(j->end());
    delete j;
    return STATUS_OK;
}
#endif

static err_t setWebSetting(char *parameter) { // ESP401
    // We do not need the "T=" (type) parameter because the
    // Setting objects know their own type
    char *sval = get_param(parameter, "V=", true);
    const char *spos = get_param(parameter, "P=", false);
    if (*spos == '\0') {
        webPrintln("Missing parameter");
        return STATUS_INVALID_VALUE;
    }
    return do_command_or_setting(spos, sval, espresponse);
}

static err_t listSettings(char *parameter) { // ESP400
    JSONencoder* j = new JSONencoder(espresponse->client() != CLIENT_WEBUI);
    j->begin();
    j->begin_array("EEPROM");
    for (Setting *js = Setting::List; js; js = js->next()) {
        if (js->getGroup() == WEBSET) {
            js->addWebui(j);
        }
    }
    j->end_array();
    webPrint(j->end());
    delete j;
    return STATUS_OK;
}

#ifdef ENABLE_SD_CARD
static err_t runSDFile(char *parameter) { // ESP220
    parameter = trim(parameter);
    if (*parameter == '\0') {
        webPrintln("Missing file name!");
        return STATUS_INVALID_VALUE;
    }
    int8_t state = get_sd_state(true);
    if (state != SDCARD_IDLE) {
        if (state == SDCARD_NOT_PRESENT) {
            webPrintln("No SD Card");
            return STATUS_SD_FAILED_MOUNT;
        } else {
            webPrintln("SD Card Busy");
            return STATUS_SD_FAILED_BUSY;
        }
    }
    if (sys.state != STATE_IDLE) {
        webPrintln("Busy");
        return STATUS_IDLE_ERROR;
    }
    if (!openFile(SD, parameter)) {
        report_status_message(STATUS_SD_FAILED_READ, (espresponse) ? espresponse->client() : CLIENT_ALL);
        webPrintln("");
        return STATUS_OK;
    }
    char fileLine[255];
    if (!readFileLine(fileLine, 255)) {
        //No need notification here it is just a macro
        closeFile();
        webPrintln("");
        return STATUS_OK;
    }
    SD_client = (espresponse) ? espresponse->client() : CLIENT_ALL;
    report_status_message(gc_execute_line(fileLine, (espresponse) ? espresponse->client() : CLIENT_ALL), (espresponse) ? espresponse->client() : CLIENT_ALL); // execute the first line
    report_realtime_status((espresponse) ? espresponse->client() : CLIENT_ALL);
    webPrintln("");
    return STATUS_OK;
}

static err_t deleteSDObject(char *parameter) { // ESP215
    parameter = trim(parameter);
    if (*parameter == '\0') {
        webPrintln("Missing file name!");
        return STATUS_INVALID_VALUE;
    }
    int8_t state = get_sd_state(true);
    if (state != SDCARD_IDLE) {
        webPrintln( (state == SDCARD_NOT_PRESENT) ? "No SD card" : "Busy");
        return STATUS_OK;
    }
    String path = parameter;
    if (parameter[0] != '/') {
        path = "/" + path;
    }
    File file2del = SD.open(path.c_str());
    if (!file2del) {
        webPrintln("Cannot stat file!");
        return STATUS_SD_FILE_NOT_FOUND;
    }
    if (file2del.isDirectory()) {
        if (!SD.rmdir((char*)path.c_str())) {
            webPrintln("Cannot delete directory! Is directory empty?");
            return STATUS_SD_FAILED_DEL_DIR;
        }
        webPrintln("Directory deleted.");
    } else {
        if (!SD.remove((char*)path.c_str())) {
            webPrintln("Cannot delete file!");
            return STATUS_SD_FAILED_DEL_FILE;
        }
        webPrintln("File deleted.");
    }
    file2del.close();
    return STATUS_OK;
}

static err_t listSDFiles(char *parameter) { // ESP210
    int8_t state = get_sd_state(true);
    if (state != SDCARD_IDLE) {
        if (state == SDCARD_NOT_PRESENT) {
            webPrintln("No SD Card");
            return STATUS_SD_FAILED_MOUNT;
        } else {
            webPrintln("SD Card Busy");
            return STATUS_SD_FAILED_BUSY;
        }
    }
    webPrintln("");
    listDir(SD, "/", 10, espresponse->client());
    String ssd = "[SD Free:" + ESPResponseStream::formatBytes(SD.totalBytes() - SD.usedBytes());
    ssd += " Used:" + ESPResponseStream::formatBytes(SD.usedBytes());
    ssd += " Total:" + ESPResponseStream::formatBytes(SD.totalBytes());
    ssd += "]";
    webPrintln(ssd);
    return STATUS_OK;
}
#endif

static err_t showSDStatus(char *parameter) {  // ESP200
    const char* resp = "No SD card";
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
    webPrintln(resp);
    return STATUS_OK;
}

static err_t setRadioState(char *parameter) { // ESP115
    parameter = trim(parameter);
    if (*parameter == '\0') {
        // Display the radio state
        bool on = false;
#if defined (ENABLE_WIFI)
        if (WiFi.getMode() != WIFI_MODE_NULL)on = true;
#endif
#if defined (ENABLE_BLUETOOTH)
        if (bt_config.Is_BT_on())on = true;
#endif
        webPrintln(on ? "ON" : "OFF");
        return STATUS_OK;
    }
    int8_t on = -1;
    if (strcasecmp(parameter, "ON") == 0) {
        on = 1;
    } else if (strcasecmp(parameter, "OFF") == 0) {
        on = 0;
    }
    if (on == -1) {
        webPrintln("only ON or OFF mode supported!");
        return STATUS_INVALID_VALUE;
    }
    //Stop everything
#if defined (ENABLE_WIFI)
    if (WiFi.getMode() != WIFI_MODE_NULL) {
        wifi_config.StopWiFi();
    }
#endif
#if defined (ENABLE_BLUETOOTH)
    if (bt_config.Is_BT_on()) {
        bt_config.end();
    }
#endif
    //if On start proper service
    if (!on) {
        webPrintln("[MSG: Radio is Off]");
        return STATUS_OK;
    }
    //On
    switch (wifi_radio_mode->get()) {
    case ESP_WIFI_AP:
    case ESP_WIFI_STA:
        #if !defined (ENABLE_WIFI)
            webPrintln("WiFi is not enabled!");
            return STATUS_WIFI_FAIL_BEGIN;

        #else
            wifi_config.begin();
            return STATUS_OK;
        #endif
    case ESP_BT:
        #if !defined (ENABLE_BLUETOOTH)
            webPrintln("Bluetooth is not enabled!");
            return STATUS_BT_FAIL_BEGIN;
        #else
            bt_config.begin();
            return STATUS_OK;
        #endif
    default:
        webPrintln("[MSG: Radio is Off]");
        return STATUS_OK;
    }
}

#ifdef ENABLE_WIFI
static err_t showIP(char *parameter) { // ESP111
    webPrintln(parameter, WiFi.getMode() == WIFI_STA ? WiFi.localIP() : WiFi.softAPIP());
    return STATUS_OK;
}

static err_t showSetStaParams(char *parameter) { // ESP103
    if (*parameter == '\0') {
        webPrint("IP:", wifi_sta_ip->getStringValue());
        webPrint(" GW:", wifi_sta_gateway->getStringValue());
        webPrintln(" MSK:", wifi_sta_netmask->getStringValue());
        return STATUS_OK;
    }
    char *gateway = get_param(parameter, "GW=", false);
    char *netmask = get_param(parameter, "MSK=", false);
    char *ip = get_param(parameter, "IP=", false);
    err_t err = wifi_sta_ip->setStringValue(ip);
    if (!err) {
        err = wifi_sta_netmask->setStringValue(netmask);
    }
    if (!err) {
        err = wifi_sta_gateway->setStringValue(gateway);
    }
    return err;
}
#endif

static err_t showWebHelp(char *parameter) { // ESP0
    webPrintln("Persistent web settings - $name to show, $name=value to set");
    webPrintln("ESPname FullName         Description");
    webPrintln("------- --------         -----------");
    for (Setting *s = Setting::List; s; s = s->next()) {
        if (s->getGroup() == WEBSET) {
            if (s->getGrblName()) {
                webPrint(" ", s->getGrblName());
            }
            webPrintSetColumn(8);
            webPrint(s->getName());
            webPrintSetColumn(25);
            webPrintln(s->getDescription());
        }
    }
    webPrintln("");
    webPrintln("Other web commands: $name to show, $name=value to set");
    webPrintln("ESPname FullName         Values");
    webPrintln("------- --------         ------");
    for (Command *cp = Command::List; cp; cp = cp->next()) {
        if (cp->getGroup() >= WEBNOAUTH) {
            if (cp->getGrblName()) {
                webPrint(" ", cp->getGrblName());
            }
            webPrintSetColumn(8);
            webPrint(cp->getName());
            if (cp->getDescription()) {
                webPrintSetColumn(25);
                webPrintln(cp->getDescription());
            } else {
                webPrintln("");
            }
        }
    }
    return STATUS_OK;
}

// WEB_COMMON should always be defined.  It is a trick to make the definitions
// line up while allowing VSCode code folding to work correction.
#define WEB_COMMON

void make_web_settings()
{
    // If authentication enabled, display_settings skips or displays <Authentication Required>
    // RU - need user or admin password to read
    // WU - need user or admin password to set
    // WA - need admin password to set
    #ifdef ENABLE_AUTHENTICATION
        // TODO Settings - need ADMIN_ONLY and if it is called without a parameter it sets the default
        admin_password = new StringSetting(WEBUI, NULL, "AdminPwd", DEFAULT_ADMIN_PWD, isLocalPasswordValid);
        user_password = new StringSetting(WEBUI, NULL, "UserPwd", DEFAULT_USER_PWD, isLocalPasswordValid);
    #endif
    #ifdef WEB_COMMON
        new WebCommand(NULL,      WEBCMDRU, "ESP800", "FwInfo",       showFwInfo);
        new WebCommand(NULL,      WEBCMDRU, "ESP720", "SPIFFSSize",   SPIFFSSize);
        new WebCommand("FORMAT",  WEBCMDWA, "ESP710", "FormatSpiffs", formatSpiffs);
        new WebCommand("path",    WEBCMDRU, "ESP700", "RunLocalFile", runFile);
    #endif
    #ifdef ENABLE_NOTIFICATIONS
        new WebCommand("TYPE=NONE|PUSHOVER|EMAIL|LINE T1=token1 T2=token2 TS=settings",
                                  WEBCMDWA, "ESP610", "Notifications",showSetNotification);
        new WebCommand("message", WEBCMDWU, "ESP600", "SendMessage",  sendMessage);
    #endif
    #ifdef ENABLE_AUTHENTICATION
        new WebCommand("password",WEBCMDWA, "ESP555", "SetUserPassword",  setUserPassword);
    #endif
    #ifdef WEB_COMMON
        new WebCommand("RESTART", WEBCMDWA, "ESP444", "SetSystemMode",setSystemMode);
        new WebCommand(NULL,      WEBCMDRU, "ESP420", "SysStats",     showSysStats);
    #endif
    #ifdef ENABLE_WIFI
        new WebCommand(NULL,      WEBCMDRU, "ESP410", "ListAPs",      listAPs);
    #endif
    #ifdef WEB_COMMON
        new WebCommand("P=position T=type V=value",
                                  WEBCMDWA, "ESP401", "SetWebSetting",setWebSetting);
        new WebCommand(NULL,      WEBCMDRU, "ESP400", "ListSettings", listSettings);
    #endif
    #ifdef ENABLE_SD_CARD
        new WebCommand("path",    WEBCMDRU, "ESP220", "RunSDFile",    runSDFile);
        new WebCommand("file_or_directory_path",
                                  WEBCMDWU, "ESP215", "DeleteSDItem", deleteSDObject);
        new WebCommand(NULL,      WEBCMDRU, "ESP210", "ListSDFiles",  listSDFiles);
    #endif
    #ifdef WEB_COMMON
        new WebCommand(NULL,      WEBCMDRU, "ESP200", "SDStatus",     showSDStatus);
        new WebCommand("STA|AP|BT|OFF",
                                  WEBCMDWA, "ESP115", "RadioState",   setRadioState);
    #endif
    #ifdef ENABLE_WIFI
        new WebCommand(NULL,      WEBNOAUTH,"ESP111", "CurrentIP",    showIP);
        new WebCommand("IP=ipaddress MSK=netmask GW=gateway",
                                  WEBCMDWA, "ESP103", "StaParams",    showSetStaParams);
    #endif
    #ifdef WEB_COMMON
        new WebCommand(NULL,      WEBNOAUTH,"ESP0",   "WebHelp",      showWebHelp);
        new WebCommand(NULL,      WEBNOAUTH,"ESP",    "WebHelp",      showWebHelp);
    #endif
    // WebUI Settings
    // Standard WEBUI authentication is user+ to get, admin to set unless otherwise specified
    #ifdef ENABLE_NOTIFICATIONS
        notification_ts   = new StringSetting("Notification Settings",  WEBSET, NULL,     "NotificationTS",     DEFAULT_TOKEN, 0, MAX_NOTIFICATION_SETTING_LENGTH, NULL);
        notification_t2   = new StringSetting("Notification Token 2",   WEBSET, NULL,     "NotificationT2",     DEFAULT_TOKEN, MIN_NOTIFICATION_TOKEN_LENGTH, MAX_NOTIFICATION_TOKEN_LENGTH, NULL);
        notification_t1   = new StringSetting("Notification Token 1",   WEBSET, NULL,     "NotificationT1",     DEFAULT_TOKEN , MIN_NOTIFICATION_TOKEN_LENGTH, MAX_NOTIFICATION_TOKEN_LENGTH, NULL);
        notification_type = new EnumSetting("Notification type",        WEBSET, NULL,     "NotificationType",   DEFAULT_NOTIFICATION_TYPE, &notificationOptions);
    #endif
    #ifdef ENABLE_AUTHENTICATION
        user_password     = new StringSetting("User password",          WEBSET, NULL,     "UserPassword", &isLocalPasswordValid);
        admin_password    = new StringSetting("Admin password",         WEBSET, NULL,     "AdminPassword", &isLocalPasswordValid);
    #endif
    #ifdef ENABLE_BLUETOOTH
        bt_name           = new StringSetting("Bluetooth name",         WEBSET, "ESP140", "BTName",       DEFAULT_BT_NAME, MIN_BTNAME_LENGTH, MAX_BTNAME_LENGTH, (bool (*)(char*))BTConfig::isBTnameValid);
    #endif

    #ifdef WIFI_OR_BLUETOOTH
        // user+ to get, admin to set
        wifi_radio_mode   = new EnumSetting("Radio mode",               WEBSET, "ESP110", "RadioMode",    DEFAULT_RADIO_MODE, &radioEnabledOptions);
    #endif

    #ifdef ENABLE_WIFI
        telnet_port       = new IntSetting("Telnet Port",               WEBSET, "ESP131", "TelnetPort",   DEFAULT_TELNETSERVER_PORT, MIN_TELNET_PORT, MAX_TELNET_PORT, NULL);
        telnet_enable     = new EnumSetting("Telnet Enable",            WEBSET, "ESP130", "TelnetOn",     DEFAULT_TELNET_STATE, &onoffOptions);
        http_port         = new IntSetting("HTTP Port",                 WEBSET, "ESP121", "HttpPort",     DEFAULT_WEBSERVER_PORT, MIN_HTTP_PORT, MAX_HTTP_PORT, NULL);
        http_enable       = new EnumSetting("HTTP Enable",              WEBSET, "ESP120", "HttpOn",       DEFAULT_HTTP_STATE, &onoffOptions);
        wifi_hostname     = new StringSetting("Hostname",               WEBSET, "ESP112", "MyHostname",   DEFAULT_HOSTNAME, MIN_HOSTNAME_LENGTH, MAX_HOSTNAME_LENGTH, (bool (*)(char*))WiFiConfig::isHostnameValid);
        wifi_ap_channel   = new IntSetting("AP Channel",                WEBSET, "ESP108", "ApChannel",    DEFAULT_AP_CHANNEL, MIN_CHANNEL, MAX_CHANNEL, NULL);
        wifi_ap_ip        = new IPaddrSetting("AP Static IP",           WEBSET, "ESP107", "ApIP",         DEFAULT_AP_IP, NULL);
        // no get, admin to set
        wifi_ap_password  = new StringSetting("AP Password",            WEBSET, "ESP106", "ApPassword",   DEFAULT_AP_PWD,  MIN_PASSWORD_LENGTH, MAX_PASSWORD_LENGTH, (bool (*)(char*))WiFiConfig::isPasswordValid);
        wifi_ap_ssid      = new StringSetting("AP SSID",                WEBSET, "ESP105", "ApSSID",       DEFAULT_AP_SSID, MIN_SSID_LENGTH, MAX_SSID_LENGTH, (bool (*)(char*))WiFiConfig::isSSIDValid);
        wifi_sta_netmask  = new IPaddrSetting("Station Static Mask",    WEBSET, NULL,     "StaMask",      DEFAULT_STA_MK, NULL);
        wifi_sta_gateway  = new IPaddrSetting("Station Static Gateway", WEBSET, NULL,     "StaGateway",   DEFAULT_STA_GW, NULL);
        wifi_sta_ip       = new IPaddrSetting("Station Static IP",      WEBSET, NULL,     "StaIP",        DEFAULT_STA_IP, NULL);
        wifi_sta_mode     = new EnumSetting("Station IP Mode",          WEBSET, "ESP102", "StaIPMode",    DEFAULT_STA_IP_MODE, &staModeOptions);
        // no get, admin to set
        wifi_sta_password = new StringSetting("Station Password",       WEBSET, "ESP101", "StaPassword",  DEFAULT_STA_PWD,  MIN_PASSWORD_LENGTH, MAX_PASSWORD_LENGTH, (bool (*)(char*))WiFiConfig::isPasswordValid);
        wifi_sta_ssid     = new StringSetting("Station SSID",           WEBSET, "ESP100", "StaSSID",      DEFAULT_STA_SSID, MIN_SSID_LENGTH, MAX_SSID_LENGTH, (bool (*)(char*))WiFiConfig::isSSIDValid);
    #endif
}
#endif
