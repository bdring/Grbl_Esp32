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
    espresponse = out;
    // TODO authenticate
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
    webPrint(s.c_str());
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
static void print_mac(const char *s, String mac)
{
    webPrint(s);
    webPrint(" (");
    webPrint(mac);
    webPrintln(")");
}

static err_t SPIFFSSize(char *parameter) { // ESP720
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
            // TODO feed into command interpreter
            // while accumulating error codes
        }
    }
    currentfile.close();
    return STATUS_OK;
}
static err_t getSysStats(char *parameter) { // ESP420
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
    return STATUS_OK;
}
static err_t getAPList(char *parameter) { // ESP410
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
static err_t getSettings(char *parameter) { // ESP400
    JSONencoder* j = new JSONencoder(espresponse->client() != CLIENT_WEBUI);
    j->begin();
    j->begin_array("EEPROM");
    for (Setting *js = Setting::List; js; js = js->next()) {
        if (js->getGroup() == WEBUI) {
            js->addWebui(j);
        }
    }
    j->end_array();
    webPrint(j->end());
    delete j;
    return STATUS_OK;
}
static err_t printSDFile(char *parameter) { // ESP220
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
    SD_client = (espresponse) ? espresponse->client() : CLIENT_ALL;
    if (!readFileLine(fileLine)) {
        //No need notification here it is just a macro
        closeFile();
        webPrintln("");
        return STATUS_OK;
    }
    report_status_message(gc_execute_line(fileLine, (espresponse) ? espresponse->client() : CLIENT_ALL), (espresponse) ? espresponse->client() : CLIENT_ALL); // execute the first line
    report_realtime_status((espresponse) ? espresponse->client() : CLIENT_ALL);
    webPrintln("");
    return STATUS_OK;
}
static err_t getSDData(char *parameter) { // ESP210
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
static err_t getSDStatus(char *parameter) {  // ESP200
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
    if (strcmp(parameter, "RESTART") != 0) {
        webPrintln("Incorrect command");
        return STATUS_INVALID_VALUE;
    }
    grbl_send(CLIENT_ALL, "[MSG:Restart ongoing]\r\n");
    COMMANDS::restart_ESP();
    return STATUS_OK;
}
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

static err_t getSetStaParams(char *parameter) { // ESP103
    if (*parameter == '\0') {
        webPrint("IP:", wifi_sta_ip->getStringValue());
        webPrint(" GW:", wifi_sta_gateway->getStringValue());
        webPrintln(" MSK:", wifi_sta_netmask->getStringValue());
        return STATUS_OK;
    }
    const char *gateway = get_param(parameter, "GW=", false);
    const char *netmask = get_param(parameter, "MSK=", false);
    const char *ip = get_param(parameter, "IP=", false);
    err_t err = wifi_sta_ip->setStringValue(ip);
    if (!err) {
        err = wifi_sta_netmask->setStringValue(netmask);
    }
    if (!err) {
        err = wifi_sta_gateway->setStringValue(gateway);
    }
    return err;
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
static err_t getSetNotification(char *parameter) { // ESP610
    if (*parameter == '\0') {
        webPrint("", notification_type->getStringValue());
        webPrintln(" ", notification_ts->getStringValue());
        return STATUS_OK;
    }
    const char *ts = get_param(parameter, "TS=", false);
    const char *t2 = get_param(parameter, "T2=", false);
    const char *t1 = get_param(parameter, "T1=", false);
    const char *ty = get_param(parameter, "type=", false);
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
static err_t getIP(char *parameter) { // ESP111
    webPrintln(parameter, WiFi.getMode() == WIFI_STA ? WiFi.localIP() : WiFi.softAPIP());
    return STATUS_OK;
}
static err_t getFwInfo(char *parameter) { // ESP800
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
Command* webCommands[20];
void make_web_settings()
{

    #ifdef ENABLE_AUTHENTICATION
        // XXX need ADMIN_ONLY and if it is called without a parameter it sets the default
        admin_password = new StringSetting(WEBUI, NULL, "AdminPwd", DEFAULT_ADMIN_PWD, isLocalPasswordValid);
        user_password = new StringSetting(WEBUI, NULL, "UserPwd", DEFAULT_USER_PWD, isLocalPasswordValid);
    #endif
    // RU - need user or admin password to read
    // WU - need user or admin password to set
    // WA - need admin password to set
    Command **sp = webCommands;
    *sp++ = new WebCommand("Get FW info",        WEBCMDRU, "ESP800", "FwInfo",          getFwInfo);
    *sp++ = new WebCommand("SPIFFS size",        WEBCMDRU, "ESP720", "SPIFFSSize",      SPIFFSSize);
    *sp++ = new WebCommand("Run local file",     WEBCMDRU, "ESP700", "runFile",         runFile);
    *sp++ = new WebCommand("Notification",       WEBCMDWA, "ESP610", "Notification",    getSetNotification);
    *sp++ = new WebCommand("Get system stats",   WEBCMDRU, "ESP420", "getSysStats",     getSysStats);
    *sp++ = new WebCommand("Get AP list",        WEBCMDRU, "ESP410", "getAPList",       getAPList);
    *sp++ = new WebCommand("Get settings",       WEBCMDRU, "ESP400", "getSettings",     getSettings);
    *sp++ = new WebCommand("Print SD file",      WEBCMDRU, "ESP220", "printSDFile",     printSDFile);
    *sp++ = new WebCommand("Get SD content",     WEBCMDRU, "ESP210", "getSDData",       getSDData);
    *sp++ = new WebCommand("Get SD status",      WEBCMDRU, "ESP200", "getSDStatus",     getSDStatus);
    *sp++ = new WebCommand("Delete SD item",     WEBCMDWU, "ESP215", "deleteSDItem",    deleteSDObject);
    *sp++ = new WebCommand("Send message",       WEBCMDWU, "ESP600", "sendMessage",     sendMessage);
    *sp++ = new WebCommand("Format SPIFFS",      WEBCMDWA, "ESP710", "formatSpiffs",    formatSpiffs);
    #ifdef ENABLE_AUTHENTICATION
    *sp++ = new WebCommand("Set user password",  WEBCMDWA, "ESP555", "setUserPassword", setUserPassword);
    #endif
    *sp++ = new WebCommand("Set system mode",    WEBCMDWA, "ESP444", "setSystemMode",   setSystemMode);
    *sp++ = new WebCommand("Set web setting",    WEBCMDWA, "ESP401", "setRadioState",   setWebSetting);
    *sp++ = new WebCommand("Set radio state",    WEBCMDWA, "ESP115", "setRadioState",   setRadioState);
    *sp++ = new WebCommand("Get current IP",     WEBNOAUTH,"ESP111", "getIP",           getIP);
    *sp++ = new WebCommand("Set STA IP/Mask/GW", WEBCMDWA, "ESP103", "getSetStaParams", getSetStaParams);
// If authentication enabled, display_settings skips or displays <Authentication Required>

// TODO ESP or ESP0 - list ESP commands  WEBNOAUTH
// TODO ESP800 Get info,          no auth to get, no set  WEBAUTH with erroring set method

    // WebUI Settings
    // Standard WEBUI authentication is user+ to get, admin to set unless otherwise specified
    #ifdef ENABLE_NOTIFICATIONS
        notification_ts   = new StringSetting("Notification Settings",  WEBUI, NULL,     "NotificationTS",     DEFAULT_TOKEN, 0, MAX_NOTIFICATION_SETTING_LENGTH, NULL);
        notification_t2   = new StringSetting("Notification Token 2",   WEBUI, NULL,     "NotificationT2",     DEFAULT_TOKEN, MIN_NOTIFICATION_TOKEN_LENGTH, MAX_NOTIFICATION_TOKEN_LENGTH, NULL);
        notification_t1   = new StringSetting("Notification Token 1",   WEBUI, NULL,     "NotificationT1",     DEFAULT_TOKEN , MIN_NOTIFICATION_TOKEN_LENGTH, MAX_NOTIFICATION_TOKEN_LENGTH, NULL);
        notification_type = new EnumSetting("Notification type",        WEBUI, NULL,     "NotificationType",   DEFAULT_NOTIFICATION_TYPE, &notificationOptions);
    #endif
    #ifdef ENABLE_AUTHENTICATION
        user_password     = new StringSetting("User password",          WEBUI, NULL,     "UserPassword", &isLocalPasswordValid);
        admin_password    = new StringSetting("Admin password",         WEBUI, NULL,     "AdminPassword", &isLocalPasswordValid);
    #endif
    #ifdef ENABLE_BLUETOOTH
        bt_name           = new StringSetting("Bluetooth name",         WEBUI, "ESP140", "BTName",       DEFAULT_BT_NAME, 0, 0, BTConfig::isBTnameValid);
    #endif

    #if defined(ENABLE_WIFI) || defined(ENABLE_BLUETOOTH)
        // user+ to get, admin to set
        wifi_radio_mode   = new EnumSetting("Radio mode",               WEBUI, "ESP110", "RadioMode",      DEFAULT_RADIO_MODE, &radioEnabledOptions);
    #endif

    #ifdef ENABLE_WIFI
        telnet_port       = new IntSetting("Telnet Port",               WEBUI, "ESP131", "TELNET_PORT",   DEFAULT_TELNETSERVER_PORT, MIN_TELNET_PORT, MAX_TELNET_PORT, NULL);
        telnet_enable     = new EnumSetting("Telnet protocol",          WEBUI, "ESP130", "TELNET_ON", DEFAULT_TELNET_STATE, &onoffOptions);
        http_port         = new IntSetting("HTTP Port",                 WEBUI, "ESP121", "HTTP_PORT",     DEFAULT_WEBSERVER_PORT, MIN_HTTP_PORT, MAX_HTTP_PORT, NULL);
        http_enable       = new EnumSetting("HTTP protocol",            WEBUI, "ESP120", "HTTP_ON",   DEFAULT_HTTP_STATE, &onoffOptions);
        wifi_hostname     = new StringSetting("Hostname",               WEBUI, "ESP112", "ESP_HOSTNAME",      DEFAULT_HOSTNAME, 0, 0, WiFiConfig::isHostnameValid);
        wifi_ap_channel   = new IntSetting("AP Channel",                WEBUI, "ESP108", "AP_CHANNEL",    DEFAULT_AP_CHANNEL, MIN_CHANNEL, MAX_CHANNEL, NULL);
        wifi_ap_ip        = new IPaddrSetting("AP Static IP",           WEBUI, "ESP107", "AP_IP",         DEFAULT_AP_IP, NULL);
        // no get, admin to set
        wifi_ap_password  = new StringSetting("AP Password",            WEBUI, "ESP106", "AP_PWD",        DEFAULT_AP_PWD,  0, 0, WiFiConfig::isPasswordValid);
        wifi_ap_ssid      = new StringSetting("AP SSID",                WEBUI, "ESP105", "AP_SSID",       DEFAULT_AP_SSID, 0, 0, WiFiConfig::isSSIDValid);
        wifi_sta_netmask  = new IPaddrSetting("Station Static Mask",    WEBUI, NULL,     "STA_MK",        DEFAULT_STA_MK, NULL);
        wifi_sta_gateway  = new IPaddrSetting("Station Static Gateway", WEBUI, NULL,     "STA_GW",        DEFAULT_STA_GW, NULL);
        wifi_sta_ip       = new IPaddrSetting("Station Static IP",      WEBUI, NULL,     "STA_IP",        DEFAULT_STA_IP, NULL);
        wifi_sta_mode     = new EnumSetting("Station IP Mode",          WEBUI, "ESP102", "STA_IP_MODE",   DEFAULT_STA_IP_MODE, &staModeOptions);
        // no get, admin to set
        wifi_sta_password = new StringSetting("Station Password",       WEBUI, "ESP101", "STA_PWD",       DEFAULT_STA_PWD,  0, 0, WiFiConfig::isPasswordValid);
        wifi_sta_ssid     = new StringSetting("Station SSID",           WEBUI, "ESP100", "StaSSID",      DEFAULT_STA_SSID, 0, 0, WiFiConfig::isSSIDValid);
    #endif
}
