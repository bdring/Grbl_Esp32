#include "grbl.h"
#include "SettingsClass.h"
#include "espresponse.h"
#include "web_server.h"
#include "string.h"
#include <FS.h>
#include <SPIFFS.h>
#include <esp_wifi.h>
#include <esp_ota_ops.h>

StringSetting* wifi_sta_ssid;
StringSetting* wifi_sta_password;
// XXX hack StringSetting class to return a ***** password if checker is isPasswordValid

#ifdef ENABLE_WIFI
EnumSetting*   wifi_sta_mode;
IPaddrSetting* wifi_sta_ip;
IPaddrSetting* wifi_sta_gateway;
IPaddrSetting* wifi_sta_netmask;

//XXX for compatibility, implement wifi_sta_ip_gw_mk;

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

#if defined( ENABLE_WIFI) ||  defined( ENABLE_BLUETOOTH)
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
// XXX need ADMIN_ONLY and if it is called without a parameter it sets the default
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

err_t WebCommandSetting::processWebCommand(ESPResponseStream* response, char* parameter) {
    espresponse = response;
    // TODO authenticate
    password = remove_password(parameter);
    #ifdef ENABLE_AUTHENTICATION
        switch (_group) {
        case WEBCMDRU: // Read, needs user or admin password
            TBD
        case WEBCMDWU: // Write, needs user or admin password
        case WEBCMDWA: // Write, needs admin passord
        }
    #endif
    const char *err = _action(parameter);
    if (err) {
        grbl_sendf(CLIENT_ALL, "%s\n", err);
        return STATUS_INVALID_STATEMENT;
    }
    return STATUS_OK;
};

// We create a variety of print functions to make the rest
// of the code more compact and readable.
static void webPrint(const char *s)
{
    if (espresponse) {
        espresponse->print(s);
    }
}
static void webPrint(string s)
{
    webPrint(s.c_str);
}
static void webPrint(String s)
{
    webPrint(s.c_str);
}
static void webPrint(const char *s1, const char *s2)
{
    webPrint(s1);
    webPrint(s2);
}
static void webPrint(const char *s1, String s2)
{
    webPrint(s1);
    webPrint(s2.c_str);
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
}
static void webPrintln(String s)
{
    webPrintln(s.c_str);
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
    webPrintln(s, s2.c_str);
}
static void print_mac(const char *s, String mac)
{
    webPrint(s);
    webPrint(" (");
    webPrint(mac);
    webPrintln(")");
}

static const char* SPIFFSSize(char *parameter) { // ESP720
    webPrint(parameter);
    webPrint("SPIFFS  Total:", ESPResponseStream::formatBytes(SPIFFS.totalBytes()));
    webPrintln(" Used:", ESPResponseStream::formatBytes(SPIFFS.usedBytes()));
    return STATUS_OK;
}

char *trim(char *str)
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

static const char* runFile(char *parameter) { // ESP700
    String path = trim(parameter);
    if ((path.length() > 0) && (path[0] != '/')) {
        path = "/" + path;
    }
    if (!SPIFFS.exists(path)) {
        webPrintln("");
        return "No such file!";
    }
    File currentfile = SPIFFS.open(parameter, FILE_READ);
    if (!currentfile) {//if file open success
        return "File open failed";
    }
    //until no line in file
    while (currentfile.available()) {
        String currentline = currentfile.readStringUntil('\n');
        currentline.replace("\n", "");
        currentline.replace("\r", "");
        if (currentline.length() > 0) {
            // TODO feed into command interpreter
            // while accumulating error codes
        }
    }
    currentfile.close();
    return NULL;
}
static const char* getSysStats(char *parameter) { // ESP420
    webPrintln("Chip ID: ", String((uint16_t)(ESP.getEfuseMac() >> 32)));
    webPrintln("CPU Frequency: ", String(ESP.getCpuFreqMHz()) + "Mhz");
    webPrintln("CPU Temperature: ", String(temperatureRead(), 1) + "C");
    webPrintln("Free memory: ", ESPResponseStream::formatBytes(ESP.getFreeHeap()));
    webPrintln("SDK: ", ESP.getSdkVersion());
    webPrintln("Flash Size: ", ESPResponseStream::formatBytes(ESP.getFlashChipSize()));

    // Round baudRate to nearest 100 because ESP32 can say e.g. 115201
    webPrintln("Baud rate: ", String((Serial.baudRate()/100) * 100));
    webPrintln("Sleep mode: ", WiFi.getSleep() ? "Modem" : "None");

    int mode = WiFi.getMode();
    if (mode != WIFI_MODE_NULL) {
#if defined (ENABLE_WIFI)
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
    //TODO to complete
    webPrint("FW version: ");
    webPrint(GRBL_VERSION);
    webPrint(" (");
    webPrint(GRBL_VERSION_BUILD);
    webPrint(") (ESP32)");
    webPrintln("");
    return NULL;
}
static const char* getAPList(char *parameter) { // ESP410
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
    return NULL;
}
static const char* getSettings(char *parameter) { // ESP400
    JSONencoder* j = new JSONencoder(espresponse->client() != CLIENT_WEBUI);
    j->begin();
    j->begin_array("EEPROM");
    for (Setting *js = SettingsList; js; js->next()) {
        if (js->getGroup == WEBUI) {
            js->addWebui(j);
        }
    }
    j->end_array();
    webPrint(j->end());
    delete j;
    return NULL;
}
static const char* printSDFile(char *parameter) { // ESP220
    parameter = trim(parameter);
    if (*parameter == '\0') {
        return "Missing file name!";
    }
    int8_t state = get_sd_state(true);
    if (state  !=  SDCARD_IDLE) {
        return (state == SDCARD_NOT_PRESENT) ? "No SD card" : "Busy";
    }
    if (sys.state != STATE_IDLE) {
        return "Busy";
    }
    if (!openFile(SD, parameter)) {
        report_status_message(STATUS_SD_FAILED_READ, (espresponse) ? espresponse->client() : CLIENT_ALL);
        webPrintln("");
        return;
    }
    char fileLine[255];
    SD_client = (espresponse) ? espresponse->client() : CLIENT_ALL;
    if (!readFileLine(fileLine)) {
        //No need notification here it is just a macro
        closeFile();
        webPrintln("");
        return;
    }
    report_status_message(gc_execute_line(fileLine, (espresponse) ? espresponse->client() : CLIENT_ALL), (espresponse) ? espresponse->client() : CLIENT_ALL); // execute the first line
    report_realtime_status((espresponse) ? espresponse->client() : CLIENT_ALL);
    webPrintln("");
    return NULL;
}
static const char* getSDData(char *parameter) { // ESP210
    int8_t state = get_sd_state(true);
    if (state != SDCARD_IDLE) {
        return (state == SDCARD_NOT_PRESENT) ? "No SD card" : "Busy";
    }
    webPrintln("");
    listDir(SD, "/", 10, espresponse->client());
    String ssd = "[SD Free:" + ESPResponseStream::formatBytes(SD.totalBytes() - SD.usedBytes());
    ssd += " Used:" + ESPResponseStream::formatBytes(SD.usedBytes());
    ssd += " Total:" + ESPResponseStream::formatBytes(SD.totalBytes());
    ssd += "]";
    webPrintln(ssd);
    return NULL;
}
static const char* getSDStatus(char *parameter) {  // ESP200
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
    return NULL;
}
static const char* deleteSDObject(char *parameter) { // ESP215
    parameter = trim(parameter);
    if (*parameter == '\0') {
        return "Missing file name!";
    }
    int8_t state = get_sd_state(true);
    if (state != SDCARD_IDLE) {
        webPrintln( (state == SDCARD_NOT_PRESENT) ? "No SD card" : "Busy");
        return NULL;
    }
    String path = parameter;
    if (parameter[0] != '/') {
        path = "/" + path;
    }
    File file2del = SD.open(path.c_str());
    if (!file2del) {
        return "Cannot stat file!";
    }
    if (file2del.isDirectory()) {
        if (!SD.rmdir((char*)path.c_str())) {
            return "Cannot delete directory! Is directory empty?";
        }
        webPrintln("Directory deleted.");
    } else {
        if (!SD.remove((char*)path.c_str())) {
            return "Cannot delete file!";
        }
        webPrintln("File deleted.");
    }
    file2del.close();
    return NULL;
}
static const char* sendMessage(char *parameter) { // ESP600
    if (*parameter == '\0') {
        return "Invalid message!";
    }
    if (!notificationsservice.sendMSG("GRBL Notification", parameter)) {
        return "Cannot send message!";
    }
    return NULL;
}
static const char* formatSpiffs(char *parameter) { // ESP710
    if (strcmp(parameter, "FORMAT") != 0) {
        return "Parameter must be FORMAT";
    }
    webPrint("Formatting");
    SPIFFS.format();
    webPrintln("...Done");
    return NULL;
}
#ifdef ENABLE_AUTHENTICATION
static const char* setUserPassword(char *parameter) { // ESP555
    if (*parameter == '\0') {
        user_password.setDefault();
        return NULL;
    }
    return user_password.setStringValue(parameter) ? "Invalid Password" : NULL;
}
#endif
static const char* setSystemMode(char *parameter) { // ESP444
    parameter = trim(parameter);
    if (strcmp(parameter, "RESTART") != 0) {
        return "Incorrect command";
    }
    grbl_send(CLIENT_ALL, "[MSG:Restart ongoing]\r\n");
    COMMANDS::restart_ESP();
    return NULL;
}
static const char* setWebSetting(char *parameter) { // ESP401
    // We do not need the "T=" (type) parameter because the
    // Setting objects know their own type
    const char *spos = get_param(parameter, "P=", false).cstr;
    const char *sval = get_param(parameter, "V=", true).cstr;
    if (*spos == '\0') {
        return "Missing parameter";
    }
    return do_setting(spos, sval) ? "error" : NULL;
}
static const char* setRadioState(char *parameter) { // ESP115
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
        return NULL;
    }
    int8_t on = -1;
    if (strcmp(parameter, "ON") == 0) {
        on = 1;
    } else if (strcmp(parameter, "OFF") == 0) {
        on = 0;
    }
    if (on == -1) {
        return "only ON or OFF mode supported!";
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
        return NULL;
    }
    //On
    switch (wifi_radio_mode->get()) {
    case ESP_WIFI_AP:
    case ESP_WIFI_STA:
        #if !defined (ENABLE_WIFI)
            return "WiFi is not enabled!";
        #else
            wifi_config.begin();
            return NULL;
        #endif
    case ESP_BT:
        #if !defined (ENABLE_BLUETOOTH)
            return "Bluetooth is not enabled!";
        #else
            bt_config.begin();
            return NULL;
        #endif
    default:
        webPrintln("[MSG: Radio is Off]");
        return NULL;
    }
}
Setting* webSettings[20];
void make_web_settings()
{
    // WebUI Settings
    // Standard WEBUI authentication is user+ to get, admin to set unless otherwise specified
    #ifdef ENABLE_NOTIFICATIONS
// TODO ESP610 Set type, t1, t2, ts all at once  (standard AUTH)
        notification_ts = new StringSetting(NULL, WEBUI, "Notification Settings", "NotifyTS", DEFAULT_TOKEN, 0, MAX_NOTIFICATION_SETTING_LENGTH, NULL);
        notification_t2 = new StringSetting(NULL, WEBUI, "Notification Token 2", "NotifyT2", DEFAULT_TOKEN, MIN_NOTIFICATION_TOKEN_LENGTH, MAX_NOTIFICATION_TOKEN_LENGTH, NULL);
        notification_t1 = new StringSetting(NULL, WEBUI, "Notification Token 1", "NotifyT1", DEFAULT_TOKEN , MIN_NOTIFICATION_TOKEN_LENGTH, MAX_NOTIFICATION_TOKEN_LENGTH, NULL);
        notification_type = new EnumSetting(NULL, WEBUI, "Notification type", "NotifyType", DEFAULT_NOTIFICATION_TYPE, &notificationOptions);
    #endif

    #ifdef ENABLE_AUTHENTICATION
        // XXX need ADMIN_ONLY and if it is called without a parameter it sets the default
        admin_password = new StringSetting(WEBUI, NULL, "AdminPwd", DEFAULT_ADMIN_PWD, isLocalPasswordValid);
        user_password = new StringSetting(WEBUI, NULL, "UserPwd", DEFAULT_USER_PWD, isLocalPasswordValid);
    #endif
    // RU - need user or admin password to read
    // WU - need user or admin password to set
    // WA - need admin password to set
    Setting **sp = webSettings;
    *sp++ = new WebCommandSetting("SPIFFS size",      WEBCMDRU, "ESP720", "SPIFFSSize",      SPIFFSSize);
    *sp++ = new WebCommandSetting("Run local file",   WEBCMDRU, "ESP700", "runFile",         runFile);
    *sp++ = new WebCommandSetting("Get system stats", WEBCMDRU, "ESP420", "getSysStats",     getSysStats);
    *sp++ = new WebCommandSetting("Get AP list",      WEBCMDRU, "ESP410", "getAPList",       getAPList);
    *sp++ = new WebCommandSetting("Get settings",     WEBCMDRU, "ESP400", "getSettings",     getSettings);
    *sp++ = new WebCommandSetting("Print SD file",    WEBCMDRU, "ESP220", "printSDFile",     printSDFile);
    *sp++ = new WebCommandSetting("Get SD content",   WEBCMDRU, "ESP210", "getSDData",       getSDData);
    *sp++ = new WebCommandSetting("Get SD status",    WEBCMDRU, "ESP200", "getSDStatus",     getSDStatus);
    *sp++ = new WebCommandSetting("Delete SD item",   WEBCMDWU, "ESP215", "deleteSDItem",    deleteSDObject);
    *sp++ = new WebCommandSetting("Send message",     WEBCMDWU, "ESP600", "sendMessage",     sendMessage);
    *sp++ = new WebCommandSetting("Format SPIFFS",    WEBCMDWA, "ESP710", "formatSpiffs",    formatSpiffs);
    #ifdef ENABLE_AUTHENTICATION
    *sp++ = new WebCommandSetting("Set user password",WEBCMDWA, "ESP555", "setUserPassword", setUserPassword);
    #endif
    *sp++ = new WebCommandSetting("Set system mode",  WEBCMDWA, "ESP444", "setSystemMode",   setSystemMode);
    *sp++ = new WebCommandSetting("Set web setting",  WEBCMDWA, "ESP401", "setRadioState",   setWebSetting);
    *sp++ = new WebCommandSetting("Set radio state",  WEBCMDWA, "ESP115", "setRadioState",   setRadioState);
// If authentication enabled, display_settings skips or displays <Authentication Required>

// TODO ESP or ESP0 - list ESP commands  WEBNOAUTH
// TODO ESP111 get current IP,    no auth to get, no set  WEBNOAUTH
// TODO ESP800 Get info,          no auth to get, no set  WEBAUTH with erroring set method
// TODO ESP600 Send message,            no get, user+ to set

    #ifdef ENABLE_AUTHENTICATION
        user_password = new StringSetting("User password", WEBUI, NULL, "UserPassword", &isLocalPasswordValid);
    #endif
    #ifdef ENABLE_BLUETOOTH
        bt_name = new StringSetting("Bluetooth name", WEBUI, "ESP140", "BTName", DEFAULT_BT_NAME, 0, 0, BTConfig::isBTnameValid);
    #endif

    #if defined(ENABLE_WIFI) || defined(ENABLE_BLUETOOTH)
        // user+ to get, admin to set
        wifi_radio_mode = new EnumSetting("Radio mode", WEBUI, "ESP110", "RadioMode", DEFAULT_RADIO_MODE, &radioEnabledOptions);
    #endif

    #ifdef ENABLE_WIFI
        telnet_port = new IntSetting("Telnet Port", WEBUI, "ESP131", "TelnetPort", DEFAULT_TELNETSERVER_PORT, MIN_TELNET_PORT, MAX_TELNET_PORT, NULL);
        telnet_enable = new EnumSetting("Telnet protocol", WEBUI, "ESP130", "TelnetOn", DEFAULT_TELNET_STATE, &onoffOptions);
        http_port = new IntSetting("HTTP Port", WEBUI, "ESP121", "HTTPPort", DEFAULT_WEBSERVER_PORT, MIN_HTTP_PORT, MAX_HTTP_PORT, NULL);
        http_enable = new EnumSetting("HTTP protocol", WEBUI, "ESP120", "HTTPOn", DEFAULT_HTTP_STATE, &onoffOptions);
        wifi_hostname = new StringSetting("Hostname", WEBUI, "ESP112", "ESPHostname", DEFAULT_HOSTNAME, 0, 0, WiFiConfig::isHostnameValid);
        wifi_ap_channel = new IntSetting("AP Channel", WEBUI, "ESP108", "APChannel", DEFAULT_AP_CHANNEL, MIN_CHANNEL, MAX_CHANNEL, NULL);
        wifi_ap_ip = new IPaddrSetting("AP Static IP", WEBUI, "ESP107", "APIP", DEFAULT_AP_IP, NULL);
        // no get, admin to set
        wifi_ap_password = new StringSetting("AP Password", WEBUI, "ESP106", "APPassword", DEFAULT_AP_PWD, 0, 0, WiFiConfig::isPasswordValid);
        wifi_ap_ssid = new StringSetting("AP SSID", WEBUI, "ESP105", "ApSSID", DEFAULT_AP_SSID, 0, 0, WiFiConfig::isSSIDValid);
        //XXX for compatibility, implement wifi_sta_ip_gw_mk()
        // TODO ESP103 Set type, t1, t2, ts all at once  (standard AUTH)
        wifi_sta_netmask = new IPaddrSetting("Station Static Mask", WEBUI, NULL, "StaNetmask", DEFAULT_STA_MK, NULL);
        wifi_sta_gateway = new IPaddrSetting("Station Static Gateway", WEBUI, NULL, "StaGateway", DEFAULT_STA_GW, NULL);
        wifi_sta_ip = new IPaddrSetting("Station Static IP", WEBUI, NULL, "StaIP", DEFAULT_STA_IP, NULL);
        wifi_sta_mode = new EnumSetting("Station IP Mode", WEBUI, "ESP102", "StaIPMode", DEFAULT_STA_IP_MODE, &staModeOptions);
        // no get, admin to set
        wifi_sta_password = new StringSetting("Station Password", WEBUI, "ESP101", "StaPwd", DEFAULT_STA_PWD, 0, 0, WiFiConfig::isPasswordValid);
        wifi_sta_ssid = new StringSetting("Station SSID", WEBUI, "ESP100", "StaSSID", DEFAULT_STA_SSID, 0, 0, WiFiConfig::isSSIDValid);
    #endif
}
