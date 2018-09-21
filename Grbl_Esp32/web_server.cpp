/*
  web_server.cpp -  web server functions class

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

#if defined (ENABLE_WIFI) &&  defined (ENABLE_HTTP)

#include "wifiservices.h"

#include "grbl.h"

#include "serial2socket.h"
#include "web_server.h"
#include <WebSocketsServer.h>
#include "wificonfig.h"
#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#ifdef ENABLE_SD_CARD
#include <SD.h>
#include "grbl_sd.h"
#endif
#include <Preferences.h>
#include "report.h"
#include <WebServer.h>
#include <ESP32SSDP.h>
#include <StreamString.h>
#include <Update.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#ifdef ENABLE_MDNS
#include <ESPmDNS.h>
#endif
#ifdef ENABLE_SSDP
#include <ESP32SSDP.h>
#endif
#ifdef ENABLE_CAPTIVE_PORTAL
#include <DNSServer.h>
const byte DNS_PORT = 53;
DNSServer dnsServer;
#endif

//embedded response file if no files on SPIFFS
#include "nofile.h"

// Define line flags. Includes comment type tracking and line overflow detection.
#define LINE_FLAG_OVERFLOW bit(0)
#define LINE_FLAG_COMMENT_PARENTHESES bit(1)
#define LINE_FLAG_COMMENT_SEMICOLON bit(2)

//Upload status
typedef enum {
    UPLOAD_STATUS_NONE = 0,
    UPLOAD_STATUS_FAILED = 1,
    UPLOAD_STATUS_CANCELLED = 2,
    UPLOAD_STATUS_SUCCESSFUL = 3,
    UPLOAD_STATUS_ONGOING  = 4
} upload_status_type;

#ifdef ENABLE_AUTHENTICATION
#define DEFAULT_ADMIN_PWD "admin"
#define DEFAULT_USER_PWD  "user";
#define DEFAULT_ADMIN_LOGIN  "admin"
#define DEFAULT_USER_LOGIN "user"
#define ADMIN_PWD_ENTRY "ADMIN_PWD"
#define USER_PWD_ENTRY "USER_PWD"
#define AUTH_ENTRY_NB 20
#define MAX_LOCAL_PASSWORD_LENGTH   16
#define MIN_LOCAL_PASSWORD_LENGTH   1
#endif

//Default 404
const char PAGE_404 []  = "<HTML>\n<HEAD>\n<title>Redirecting...</title> \n</HEAD>\n<BODY>\n<CENTER>Unknown page : $QUERY$- you will be redirected...\n<BR><BR>\nif not redirected, <a href='http://$WEB_ADDRESS$'>click here</a>\n<BR><BR>\n<PROGRESS name='prg' id='prg'></PROGRESS>\n\n<script>\nvar i = 0; \nvar x = document.getElementById(\"prg\"); \nx.max=5; \nvar interval=setInterval(function(){\ni=i+1; \nvar x = document.getElementById(\"prg\"); \nx.value=i; \nif (i>5) \n{\nclearInterval(interval);\nwindow.location.href='/';\n}\n},1000);\n</script>\n</CENTER>\n</BODY>\n</HTML>\n\n";
const char PAGE_CAPTIVE [] = "<HTML>\n<HEAD>\n<title>Captive Portal</title> \n</HEAD>\n<BODY>\n<CENTER>Captive Portal page : $QUERY$- you will be redirected...\n<BR><BR>\nif not redirected, <a href='http://$WEB_ADDRESS$'>click here</a>\n<BR><BR>\n<PROGRESS name='prg' id='prg'></PROGRESS>\n\n<script>\nvar i = 0; \nvar x = document.getElementById(\"prg\"); \nx.max=5; \nvar interval=setInterval(function(){\ni=i+1; \nvar x = document.getElementById(\"prg\"); \nx.value=i; \nif (i>5) \n{\nclearInterval(interval);\nwindow.location.href='/';\n}\n},1000);\n</script>\n</CENTER>\n</BODY>\n</HTML>\n\n";


Web_Server web_server;
bool Web_Server::_setupdone = false;
uint16_t Web_Server::_port = 0;
String Web_Server::_hostname = "";
uint16_t Web_Server::_data_port = 0;
long Web_Server::_id_connection = 0;
uint8_t Web_Server::_upload_status = UPLOAD_STATUS_NONE;
WebServer * Web_Server::_webserver = NULL;
WebSocketsServer * Web_Server::_socket_server = NULL;
#ifdef ENABLE_AUTHENTICATION
auth_ip * Web_Server::_head = NULL;
uint8_t Web_Server::_nb_ip = 0;
#define MAX_AUTH_IP 10
#endif
Web_Server::Web_Server(){
    
}
Web_Server::~Web_Server(){
    end();
}

long Web_Server::get_client_ID() {
    return  _id_connection;
}

bool Web_Server::begin(){
   
    bool no_error = true;
    _setupdone = false;
    Preferences prefs;
    prefs.begin(NAMESPACE, true);
    int8_t penabled = prefs.getChar(HTTP_ENABLE_ENTRY, DEFAULT_HTTP_STATE);
    //Get http port
    _port = prefs.getUShort(HTTP_PORT_ENTRY, DEFAULT_WEBSERVER_PORT);
    //Get hostname
    String defV = DEFAULT_HOSTNAME;
    _hostname = prefs.getString(HOSTNAME_ENTRY, defV);
    prefs.end();
    if (penabled == 0) return false;
    //create instance
    _webserver= new WebServer(_port);
#ifdef ENABLE_AUTHENTICATION
    //here the list of headers to be recorded
    const char * headerkeys[] = {"Cookie"} ;
    size_t headerkeyssize = sizeof (headerkeys) / sizeof (char*);
    //ask server to track these headers
    _webserver->collectHeaders (headerkeys, headerkeyssize );
#endif
    _socket_server = new WebSocketsServer(_port + 1);
    _socket_server->begin();
    _socket_server->onEvent(handle_Websocket_Event);

    
    //Websocket output
     Serial2Socket.attachWS(_socket_server);
     
    //events functions
    //_web_events->onConnect(handle_onevent_connect);
    //events management
   // _webserver->addHandler(_web_events);
   
    //Websocket function
    //_web_socket->onEvent(handle_Websocket_Event);
    //Websocket management
    //_webserver->addHandler(_web_socket);
   
    //Web server handlers
    //trick to catch command line on "/" before file being processed
    _webserver->on("/",HTTP_ANY, handle_root);
    
    //Page not found handler
    _webserver->onNotFound (handle_not_found);
    
    //need to be there even no authentication to say to UI no authentication
    _webserver->on("/login", HTTP_ANY, handle_login);
    
    //web commands
    _webserver->on ("/command", HTTP_ANY, handle_web_command);
    _webserver->on ("/command_silent", HTTP_ANY, handle_web_command_silent);
    
    //SPIFFS
    _webserver->on ("/files", HTTP_ANY, handleFileList, SPIFFSFileupload);
    
    //web update
    _webserver->on ("/updatefw", HTTP_ANY, handleUpdate, WebUpdateUpload);
        
#ifdef ENABLE_SD_CARD    
    //Direct SD management
    _webserver->on("/upload", HTTP_ANY, handle_direct_SDFileList,SDFile_direct_upload);
    //_webserver->on("/SD", HTTP_ANY, handle_SDCARD);
#endif
    
#ifdef ENABLE_CAPTIVE_PORTAL
     if(WiFi.getMode() == WIFI_AP){
        // if DNSServer is started with "*" for domain name, it will reply with
        // provided IP to all DNS request
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
        _webserver->on ("/generate_204", HTTP_ANY,  handle_root);
        _webserver->on ("/gconnectivitycheck.gstatic.com", HTTP_ANY, handle_root);
        //do not forget the / at the end
        _webserver->on ("/fwlink/", HTTP_ANY, handle_root);
     }
#endif
    
#ifdef ENABLE_SSDP
    //SSDP service presentation
    if(WiFi.getMode() == WIFI_STA){
        _webserver->on ("/description.xml", HTTP_GET, handle_SSDP);
        //Add specific for SSDP
        SSDP.setSchemaURL ("description.xml");
        SSDP.setHTTPPort (_port);
        SSDP.setName (_hostname);
        SSDP.setURL ("/");
        SSDP.setDeviceType ("upnp:rootdevice");
        /*Any customization could be here
        SSDP.setModelName (ESP32_MODEL_NAME);
        SSDP.setModelURL (ESP32_MODEL_URL);
        SSDP.setModelNumber (ESP_MODEL_NUMBER);
        SSDP.setManufacturer (ESP_MANUFACTURER_NAME);
        SSDP.setManufacturerURL (ESP_MANUFACTURER_URL);
        */
            
        //Start SSDP
        grbl_send(CLIENT_ALL,"[MSG:SSDP Started]\r\n");
        SSDP.begin();
    }
#endif
    grbl_send(CLIENT_ALL,"[MSG:HTTP Started]\r\n");
    //start webserver
    _webserver->begin();
#ifdef ENABLE_MDNS
    //add mDNS
    if(WiFi.getMode() == WIFI_STA){
        MDNS.addService("http","tcp",_port);
    }
#endif
    _setupdone = true;
   return no_error;
}

void Web_Server::end(){
    _setupdone = false;
#ifdef ENABLE_MDNS
    //remove mDNS
    mdns_service_remove("_http", "_tcp");
#endif
    if (_socket_server) {
        delete _socket_server;
        _socket_server = NULL;
    }
    if (_webserver) {
        delete _webserver;
        _webserver = NULL;
    }
#ifdef ENABLE_AUTHENTICATION
    while (_head) {
        auth_ip * current = _head;
        _head = _head->_next;
        delete current;
    }
    _nb_ip = 0;
#endif
}

//Root of Webserver/////////////////////////////////////////////////////

void Web_Server::handle_root()
{
    String path = "/index.html";
    String contentType =  getContentType(path);
    String pathWithGz = path + ".gz";
    //if have a index.html or gzip version this is default root page
    if((SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) && !_webserver->hasArg("forcefallback") && _webserver->arg("forcefallback")!="yes") {
        if(SPIFFS.exists(pathWithGz)) {
            path = pathWithGz;
        }
		File file = SPIFFS.open(path, FILE_READ);
        _webserver->streamFile(file, contentType);
        file.close();
        return;
    }
    //if no lets launch the default content
    _webserver->sendHeader("Content-Encoding", "gzip");
    _webserver->send_P(200,"text/html",PAGE_NOFILES,PAGE_NOFILES_SIZE);
}

//Handle not registred path on SPIFFS neither SD ///////////////////////
void Web_Server:: handle_not_found()
{
    if (is_authenticated() == LEVEL_GUEST) {
        _webserver->sendContent_P("HTTP/1.1 301 OK\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n");
        //_webserver->client().stop();
        return;
    }
    bool page_not_found = false;
    String path = _webserver->urlDecode(_webserver->uri());
    String contentType =  getContentType(path);
    String pathWithGz = path + ".gz";

#ifdef ENABLE_SD_CARD
    if ((path.substring(0,4) == "/SD/")) {
        //remove /SD
        path = path.substring(3);
        if(SD.exists((char *)pathWithGz.c_str()) || SD.exists((char *)path.c_str())) {
            if(SD.exists((char *)pathWithGz.c_str())) {
                path = pathWithGz;
            }
            File datafile = SD.open((char *)path.c_str());
            if (datafile) {
               if( _webserver->streamFile(datafile, contentType) == datafile.size()) {
                    datafile.close();
                    wifi_config.wait(0);
                    return;
               } else{
                   datafile.close();
               }
            }
        } 
        String content = "cannot find ";
        content+=path;
        _webserver->send(404,"text/plain",content.c_str());
        return;
    } else
#endif
        if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
            if(SPIFFS.exists(pathWithGz)) {
                path = pathWithGz;
            }
            File file = SPIFFS.open(path, FILE_READ);
            _webserver->streamFile(file, contentType);
            file.close();
            return;
        } else {
            page_not_found = true;
        }

    if (page_not_found ) {
#ifdef ENABLE_CAPTIVE_PORTAL
        if (WiFi.getMode()!=WIFI_STA ) {
            String contentType= PAGE_CAPTIVE;
            String stmp = WiFi.softAPIP().toString();
            //Web address = ip + port
            String KEY_IP = "$WEB_ADDRESS$";
            String KEY_QUERY = "$QUERY$";
            if (_port != 80) {
                stmp+=":";
                stmp+=String(_port);
            }
            contentType.replace(KEY_IP,stmp);
            contentType.replace(KEY_IP,stmp);
            contentType.replace(KEY_QUERY,_webserver->uri());
            _webserver->send(200,"text/html",contentType);
            //_webserver->sendContent_P(NOT_AUTH_NF);
            //_webserver->client().stop();
            return;
        }
#endif
        path = "/404.htm";
        contentType =  getContentType(path);
        pathWithGz =  path + ".gz";
        if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
            if(SPIFFS.exists(pathWithGz)) {
                path = pathWithGz;
            }
            File file = SPIFFS.open(path, FILE_READ);
            _webserver->streamFile(file, contentType);
            file.close();
           
        } else {
            //if not template use default page
            contentType = PAGE_404;
            String stmp;
            if (WiFi.getMode()==WIFI_STA ) {
                stmp=WiFi.localIP().toString();
            } else {
                stmp=WiFi.softAPIP().toString();
            }
            //Web address = ip + port
            String KEY_IP = "$WEB_ADDRESS$";
            String KEY_QUERY = "$QUERY$";
            if ( _port != 80) {
                stmp+=":";
                stmp+=String(_port);
            }
            contentType.replace(KEY_IP,stmp);
            contentType.replace(KEY_QUERY,_webserver->uri());
            _webserver->send(200,"text/html",contentType);
        }
    }
}

//http SSDP xml presentation
void Web_Server::handle_SSDP ()
{
    StreamString sschema ;
    if (sschema.reserve (1024) ) {
        String templ =  "<?xml version=\"1.0\"?>"
                        "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
                        "<specVersion>"
                        "<major>1</major>"
                        "<minor>0</minor>"
                        "</specVersion>"
                        "<URLBase>http://%s:%u/</URLBase>"
                        "<device>"
                        "<deviceType>upnp:rootdevice</deviceType>"
                        "<friendlyName>%s</friendlyName>"
                        "<presentationURL>/</presentationURL>"
                        "<serialNumber>%s</serialNumber>"
                        "<modelName>ESP32</modelName>"
                        "<modelNumber>Marlin</modelNumber>"
                        "<modelURL>http://espressif.com/en/products/hardware/esp-wroom-32/overview</modelURL>"
                        "<manufacturer>Espressif Systems</manufacturer>"
                        "<manufacturerURL>http://espressif.com</manufacturerURL>"
                        "<UDN>uuid:%s</UDN>"
                        "</device>"
                        "</root>\r\n"
                        "\r\n";
        char uuid[37];
        String sip = WiFi.localIP().toString();
        uint32_t chipId = (uint16_t) (ESP.getEfuseMac() >> 32);
        sprintf (uuid, "38323636-4558-4dda-9188-cda0e6%02x%02x%02x",
                 (uint16_t) ( (chipId >> 16) & 0xff),
                 (uint16_t) ( (chipId >>  8) & 0xff),
                 (uint16_t)   chipId        & 0xff  );
        String serialNumber = String (chipId);
        sschema.printf (templ.c_str(),
                        sip.c_str(),
                        _port,
                        _hostname.c_str(),
                        serialNumber.c_str(),
                        uuid);
        _webserver->send (200, "text/xml", (String) sschema);
    } else {
        _webserver->send (500);
    }
}

bool Web_Server::is_realtime_cmd(char c){
    if (c == CMD_STATUS_REPORT) return true;
    if (c == CMD_CYCLE_START) return true;
    if (c == CMD_RESET) return true;
    if (c == CMD_FEED_HOLD) return true;
    if (c == CMD_SAFETY_DOOR) return true;
    if (c == CMD_JOG_CANCEL) return true;
    if (c == CMD_DEBUG_REPORT) return true;
    if (c == CMD_FEED_OVR_RESET) return true;
    if (c == CMD_FEED_OVR_COARSE_PLUS) return true;
    if (c == CMD_FEED_OVR_COARSE_MINUS) return true;
    if (c == CMD_FEED_OVR_FINE_PLUS) return true;
    if (c == CMD_FEED_OVR_FINE_MINUS) return true;
    if (c == CMD_RAPID_OVR_RESET) return true;
    if (c == CMD_RAPID_OVR_MEDIUM) return true;
    if (c == CMD_RAPID_OVR_LOW) return true;
    if (c == CMD_SPINDLE_OVR_COARSE_PLUS) return true;
    if (c == CMD_SPINDLE_OVR_COARSE_MINUS) return true;
    if (c == CMD_SPINDLE_OVR_FINE_PLUS) return true;
    if (c == CMD_SPINDLE_OVR_FINE_MINUS) return true;
    if (c == CMD_SPINDLE_OVR_STOP) return true;
    if (c == CMD_COOLANT_FLOOD_OVR_TOGGLE) return true;
    if (c == CMD_COOLANT_MIST_OVR_TOGGLE) return true;
    return false;
}

//Handle web command query and send answer//////////////////////////////
void Web_Server::handle_web_command ()
{
    //to save time if already disconnected
    //if (_webserver->hasArg ("PAGEID") ) {
    //    if (_webserver->arg ("PAGEID").length() > 0 ) {
    //       if (_webserver->arg ("PAGEID").toInt() != _id_connection) {
    //       _webserver->send (200, "text/plain", "Invalid command");
    //       return;
    //       }
    //    }
    //}
    level_authenticate_type auth_level = is_authenticated();
    String cmd = "";
    if (_webserver->hasArg ("plain") || _webserver->hasArg ("commandText") ) {
        if (_webserver->hasArg ("plain") ) {
            cmd = _webserver->arg ("plain");
        } else {
            cmd = _webserver->arg ("commandText");
        }
    } else {
        _webserver->send (200, "text/plain", "Invalid command");
        return;
    }
    //if it is internal command [ESPXXX]<parameter>
    cmd.trim();
    int ESPpos = cmd.indexOf ("[ESP");
    if (ESPpos > -1) {
        //is there the second part?
        int ESPpos2 = cmd.indexOf ("]", ESPpos);
        if (ESPpos2 > -1) {
            //Split in command and parameters
            String cmd_part1 = cmd.substring (ESPpos + 4, ESPpos2);
            String cmd_part2 = "";
            //only [ESP800] is allowed login free if authentication is enabled
            if ( (auth_level == LEVEL_GUEST)  && (cmd_part1.toInt() != 800) ) {
                _webserver->send (401, "text/plain", "Authentication failed!\n");
                return;
            }
            //is there space for parameters?
            if (ESPpos2 < cmd.length() ) {
                cmd_part2 = cmd.substring (ESPpos2 + 1);
            }
            //if command is a valid number then execute command
            if (cmd_part1.toInt() != 0) {
                ESPResponseStream espresponse(_webserver);
                //commmand is web only 
                execute_internal_command (cmd_part1.toInt(), cmd_part2, auth_level, &espresponse);
                //flush
                espresponse.flush();
            }
            //if not is not a valid [ESPXXX] command
        }
    } else { //execute GCODE
        if (auth_level == LEVEL_GUEST) {
            _webserver->send (401, "text/plain", "Authentication failed!\n");
            return;
        }
        //Instead of send several commands one by one by web  / send full set and split here
        String scmd;
        String res = "Ok";
        uint8_t sindex = 0;
        scmd = get_Splited_Value(cmd,'\n', sindex);
        while ( scmd != "" ){
        if (scmd.length() > 1)scmd += "\n";
        else if (!is_realtime_cmd(scmd[0]) )scmd += "\n";
        if (!Serial2Socket.push(scmd.c_str()))res = "Error";
        sindex++;
        scmd = get_Splited_Value(cmd,'\n', sindex);
        }
        _webserver->send (200, "text/plain", res.c_str());
    }
}
//Handle web command query and send answer//////////////////////////////
void Web_Server::handle_web_command_silent ()
{
     //to save time if already disconnected
     //if (_webserver->hasArg ("PAGEID") ) {
     //   if (_webserver->arg ("PAGEID").length() > 0 ) {
     //      if (_webserver->arg ("PAGEID").toInt() != _id_connection) {
     //      _webserver->send (200, "text/plain", "Invalid command");
     //      return;
     //      }
     //   }
     //}
    level_authenticate_type auth_level = is_authenticated();
    String cmd = "";
    if (_webserver->hasArg ("plain") || _webserver->hasArg ("commandText") ) {
        if (_webserver->hasArg ("plain") ) {
            cmd = _webserver->arg ("plain");
        } else {
            cmd = _webserver->arg ("commandText");
        }
    } else {
        _webserver->send (200, "text/plain", "Invalid command");
        return;
    }
    //if it is internal command [ESPXXX]<parameter>
    cmd.trim();
    int ESPpos = cmd.indexOf ("[ESP");
    if (ESPpos > -1) {
        //is there the second part?
        int ESPpos2 = cmd.indexOf ("]", ESPpos);
        if (ESPpos2 > -1) {
            //Split in command and parameters
            String cmd_part1 = cmd.substring (ESPpos + 4, ESPpos2);
            String cmd_part2 = "";
            //only [ESP800] is allowed login free if authentication is enabled
            if ( (auth_level == LEVEL_GUEST)  && (cmd_part1.toInt() != 800) ) {
                _webserver->send (401, "text/plain", "Authentication failed!\n");
                return;
            }
            //is there space for parameters?
            if (ESPpos2 < cmd.length() ) {
                cmd_part2 = cmd.substring (ESPpos2 + 1);
            }
            //if command is a valid number then execute command
            if (cmd_part1.toInt() != 0) {
                //commmand is web only 
                if(execute_internal_command (cmd_part1.toInt(), cmd_part2, auth_level, NULL)) _webserver->send (200, "text/plain", "ok");
                else  _webserver->send (200, "text/plain", "error");
            }
            //if not is not a valid [ESPXXX] command
        }
    } else { //execute GCODE
        if (auth_level == LEVEL_GUEST) {
            _webserver->send (401, "text/plain", "Authentication failed!\n");
            return;
        }
        //Instead of send several commands one by one by web  / send full set and split here
        String scmd;
        uint8_t sindex = 0;
        scmd = get_Splited_Value(cmd,'\n', sindex);
        String res = "Ok";
        while ( scmd != "" ){
        if (scmd.length() > 1)scmd+="\n";
        else if (!is_realtime_cmd(scmd[0]) )scmd+="\n";
        if (!Serial2Socket.push(scmd.c_str()))res = "Error";
        sindex++;
        scmd = get_Splited_Value(cmd,'\n', sindex);
        }
        _webserver->send (200, "text/plain", res.c_str());
    }
}


bool Web_Server::execute_internal_command (int cmd, String cmd_params, level_authenticate_type auth_level,  ESPResponseStream  *espresponse)
{
    bool response = true;
    level_authenticate_type auth_type = auth_level;

    //manage parameters
    String parameter;
    switch (cmd) {
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
        //Get full ESP32 wifi settings content
        //[ESP400]
        case 400:
            { 
            String v;
            String defV;
            Preferences prefs;
            if (!espresponse) return false;
#ifdef ENABLE_AUTHENTICATION
            if (auth_type == LEVEL_GUEST) return false;
#endif
            int8_t vi;
            espresponse->print("{\"EEPROM\":[");
            prefs.begin(NAMESPACE, true);
            //1 - Hostname
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (HOSTNAME_ENTRY);
            espresponse->print ("\",\"T\":\"S\",\"V\":\"");
            espresponse->print (_hostname.c_str());
            espresponse->print ("\",\"H\":\"Hostname\" ,\"S\":\"");
            espresponse->print (String(MAX_HOSTNAME_LENGTH).c_str());
            espresponse->print ("\", \"M\":\"");
            espresponse->print (String(MIN_HOSTNAME_LENGTH).c_str());
            espresponse->print ("\"}");
            espresponse->print (",");
            
            //2 - http protocol mode
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (HTTP_ENABLE_ENTRY);
            espresponse->print ("\",\"T\":\"B\",\"V\":\"");
            vi = prefs.getChar(HTTP_ENABLE_ENTRY, 1);
            espresponse->print (String(vi).c_str());
            espresponse->print ("\",\"H\":\"HTTP protocol\",\"O\":[{\"Enabled\":\"1\"},{\"Disabled\":\"0\"}]}");
            espresponse->print (",");
            
            //3 - http port
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (HTTP_PORT_ENTRY);
            espresponse->print ("\",\"T\":\"I\",\"V\":\"");
            espresponse->print (String(_port).c_str());
            espresponse->print ("\",\"H\":\"HTTP Port\",\"S\":\"");
            espresponse->print (String(MAX_HTTP_PORT).c_str());
            espresponse->print ("\",\"M\":\"");
            espresponse->print (String(MIN_HTTP_PORT).c_str());
            espresponse->print ("\"}");
            espresponse->print (",");
            
            //4 - telnet protocol mode
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (TELNET_ENABLE_ENTRY);
            espresponse->print ("\",\"T\":\"B\",\"V\":\"");
            vi = prefs.getChar(TELNET_ENABLE_ENTRY, 0);
            espresponse->print (String(vi).c_str());
            espresponse->print ("\",\"H\":\"Telnet protocol\",\"O\":[{\"Enabled\":\"1\"},{\"Disabled\":\"0\"}]}");
            espresponse->print (",");
            
            //5 - telnet Port
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (TELNET_PORT_ENTRY);
            espresponse->print ("\",\"T\":\"I\",\"V\":\"");
            espresponse->print (String(_data_port).c_str());
            espresponse->print ("\",\"H\":\"Telnet Port\",\"S\":\"");
            espresponse->print (String(MAX_TELNET_PORT).c_str());
            espresponse->print ("\",\"M\":\"");
            espresponse->print (String(MIN_TELNET_PORT).c_str());
            espresponse->print ("\"}");
            espresponse->print (",");
            
            //6 - wifi mode
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (ESP_WIFI_MODE);
            espresponse->print ("\",\"T\":\"B\",\"V\":\"");
            vi = prefs.getChar(ESP_WIFI_MODE, ESP_WIFI_OFF);
            espresponse->print (String(vi).c_str());
            espresponse->print ("\",\"H\":\"Wifi mode\",\"O\":[{\"STA\":\"1\"},{\"AP\":\"2\"},{\"None\":\"0\"}]}");
            espresponse->print (",");

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
            
            // 9 - STA IP mode
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (STA_IP_MODE_ENTRY);
            espresponse->print ("\",\"T\":\"B\",\"V\":\"");
            espresponse->print (String(prefs.getChar(STA_IP_MODE_ENTRY, DHCP_MODE)).c_str());
            espresponse->print ("\",\"H\":\"Station IP Mode\",\"O\":[{\"DHCP\":\"0\"},{\"Static\":\"1\"}]}");
            espresponse->print (",");

            //10-STA static IP
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (STA_IP_ENTRY);
            espresponse->print ("\",\"T\":\"A\",\"V\":\"");
            espresponse->print (wifi_config.IP_string_from_int(prefs.getInt(STA_IP_ENTRY, 0)).c_str());
            espresponse->print ("\",\"H\":\"Station Static IP\"}");
            espresponse->print (",");

            //11-STA static Gateway
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (STA_GW_ENTRY);
            espresponse->print ("\",\"T\":\"A\",\"V\":\"");
            espresponse->print (wifi_config.IP_string_from_int(prefs.getInt(STA_GW_ENTRY, 0)).c_str());
            espresponse->print ("\",\"H\":\"Station Static Gateway\"}");
            espresponse->print (",");

            //12-STA static Mask
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (STA_MK_ENTRY);
            espresponse->print ("\",\"T\":\"A\",\"V\":\"");
            espresponse->print (wifi_config.IP_string_from_int(prefs.getInt(STA_MK_ENTRY, 0)).c_str());
            espresponse->print ("\",\"H\":\"Station Static Mask\"}");
            espresponse->print (",");
            
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
            
            //15 - AP static IP
            espresponse->print ("{\"F\":\"network\",\"P\":\"");
            espresponse->print (AP_IP_ENTRY);
            espresponse->print ("\",\"T\":\"A\",\"V\":\"");
            defV = DEFAULT_AP_IP;
            espresponse->print (wifi_config.IP_string_from_int(prefs.getInt(AP_IP_ENTRY, wifi_config.IP_int_from_string(defV))).c_str());
            espresponse->print ("\",\"H\":\"AP Static IP\"}");
            espresponse->print (",");
            
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
            
            espresponse->print ("]}");
            prefs.end();
            }
            break;
        //Set EEPROM setting
        //[ESP401]P=<position> T=<type> V=<value> pwd=<user/admin password>
        case 401:    
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) return false;
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
                        //dynamique refresh is better than restart the board
                        if (spos == ESP_WIFI_MODE){
                            //TODO
                        }
                        if (spos == AP_CHANNEL_ENTRY) {
                            //TODO
                        }
                        if (spos == HTTP_ENABLE_ENTRY) {
                            //TODO
                        }
                        if (spos == TELNET_ENABLE_ENTRY) {
                            //TODO
                        }
                    }
                }
                //Integer value
                if (styp == "I") {
                    int16_t ibuf = sval.toInt();
                    if (prefs.putUShort(spos.c_str(), ibuf) == 0) {
                        response = false;
                    } else {
                        if (spos == HTTP_PORT_ENTRY){
                            //TODO
                        }
                        if (spos == TELNET_PORT_ENTRY){
                            //TODO
                            //Serial.println(ibuf);
                        }
                    }
                    
                }
                //String value
                if (styp == "S") {
                   if (prefs.putString(spos.c_str(), sval) == 0) {
                        response = false;
                    } else {
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
                    }
                    
                }
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
                prefs.end();
            }
            if (!response) {
                if (espresponse) espresponse->println ("Error: Incorrect Command");
            } else {
                 if (espresponse) espresponse->println ("ok");
            }

            }
            break;
    //Get available AP list (limited to 30)
    //output is JSON 
    //[ESP410]
    case 410: {
        if (!espresponse)return false;
#ifdef ENABLE_AUTHENTICATION
            if (auth_type == LEVEL_GUEST) return false;
#endif
        espresponse->print("{\"AP_LIST\":[");
        int n = WiFi.scanComplete();
        if (n == -2) {
            WiFi.scanNetworks (true);
        } else if (n) {	
            for (int i = 0; i < n; ++i) {
                if (i > 0) {
                    espresponse->print (",");
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
    }
    break;
        //Get ESP current status 
       case 420:
            {
#ifdef ENABLE_AUTHENTICATION
            if (auth_type == LEVEL_GUEST) return false;
#endif
            if (!espresponse)return false;
            espresponse->print ("Chip ID: ");
            espresponse->print (String ( (uint16_t) (ESP.getEfuseMac() >> 32) ).c_str());
            espresponse->print ("\n");
            espresponse->print ("CPU Frequency: ");
            espresponse->print (String (ESP.getCpuFreqMHz() ).c_str());
            espresponse->print ("Mhz");
            espresponse->print ("\n");
            espresponse->print ("CPU Temperature: ");
            espresponse->print (String (temperatureRead(), 1).c_str());
            espresponse->print ("&deg;C");
            espresponse->print ("\n");
            espresponse->print ("Free memory: ");
            espresponse->print (formatBytes (ESP.getFreeHeap()).c_str());
            espresponse->print ("\n");
            espresponse->print ("SDK: ");
            espresponse->print (ESP.getSdkVersion());
            espresponse->print ("\n");
            espresponse->print ("Flash Size: ");
            espresponse->print (formatBytes (ESP.getFlashChipSize()).c_str());
            espresponse->print ("\n");
            espresponse->print ("Available Size for update: ");
            //Not OTA on 2Mb board per spec
            if (ESP.getFlashChipSize() > 0x20000) {
                espresponse->print (formatBytes (0x140000).c_str());
            } else {
                espresponse->print (formatBytes (0x0).c_str());
            }
            espresponse->print ("\n");
            espresponse->print ("Available Size for SPIFFS: ");
            espresponse->print (formatBytes (SPIFFS.totalBytes()).c_str());
            espresponse->print ("\n");
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
            espresponse->print ("\n");
            espresponse->print ("Sleep mode: ");
            if (WiFi.getSleep())espresponse->print ("Modem");
            else espresponse->print ("None");
            espresponse->print ("\n");
            espresponse->print ("Web port: ");
            espresponse->print (String(_port).c_str());
            espresponse->print ("\n");
            espresponse->print ("Data port: ");
            if (_data_port!=0)espresponse->print (String(_data_port).c_str());
            else espresponse->print ("Disabled");
            espresponse->print ("\n");
            espresponse->print ("Hostname: ");
            espresponse->print ( _hostname.c_str());
            espresponse->print ("\n");
            espresponse->print ("Active Mode: ");
            if (WiFi.getMode() == WIFI_STA) {
                 espresponse->print ("STA (");
                 espresponse->print ( WiFi.macAddress().c_str());
                 espresponse->print (")");
                 espresponse->print ("\n");
                 espresponse->print ("Connected to: ");
                 if (WiFi.isConnected()){ //in theory no need but ...  
                     espresponse->print (WiFi.SSID().c_str());
                     espresponse->print ("\n");
                     espresponse->print ("Signal: ");
                     espresponse->print ( String(wifi_config.getSignal (WiFi.RSSI())).c_str());
                     espresponse->print ("%");
                     espresponse->print ("\n");
                     uint8_t PhyMode;
                     esp_wifi_get_protocol (ESP_IF_WIFI_STA, &PhyMode);
                     espresponse->print ("Phy Mode: ");
                     if (PhyMode == (WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N)) espresponse->print ("11n");
                     else if (PhyMode == (WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G)) espresponse->print ("11g");
                     else if (PhyMode == (WIFI_PROTOCOL_11B )) espresponse->print ("11b");
                     else espresponse->print ("???");
                     espresponse->print ("\n");
                     espresponse->print ("Channel: ");
                     espresponse->print (String (WiFi.channel()).c_str());
                     espresponse->print ("\n");
                     espresponse->print ("IP Mode: ");
                     tcpip_adapter_dhcp_status_t dhcp_status;
                     tcpip_adapter_dhcpc_get_status (TCPIP_ADAPTER_IF_STA, &dhcp_status);
                     if (dhcp_status == TCPIP_ADAPTER_DHCP_STARTED)espresponse->print ("DHCP");
                     else espresponse->print ("Static");
                     espresponse->print ("\n");
                     espresponse->print ("IP: ");
                     espresponse->print (WiFi.localIP().toString().c_str());
                     espresponse->print ("\n");
                     espresponse->print ("Gateway: ");
                     espresponse->print (WiFi.gatewayIP().toString().c_str());
                     espresponse->print ("\n");
                     espresponse->print ("Mask: ");
                     espresponse->print (WiFi.subnetMask().toString().c_str());
                     espresponse->print ("\n");
                     espresponse->print ("DNS: ");
                     espresponse->print (WiFi.dnsIP().toString().c_str());
                     espresponse->print ("\n");
                 } //this is web command so connection => no command 
                 espresponse->print ("Disabled Mode: ");
                 espresponse->print ("AP (");
                 espresponse->print (WiFi.softAPmacAddress().c_str());
                 espresponse->print (")");
                 espresponse->print ("\n");
            } else if (WiFi.getMode() == WIFI_AP) {
                 espresponse->print ("AP (");
                 espresponse->print (WiFi.softAPmacAddress().c_str());
                 espresponse->print (")");
                 espresponse->print ("\n");
                 wifi_config_t conf;
                 esp_wifi_get_config (ESP_IF_WIFI_AP, &conf);
                 espresponse->print ("SSID: ");
                 espresponse->print ((const char*) conf.ap.ssid);
                 espresponse->print ("\n");
                 espresponse->print ("Visible: ");
                 espresponse->print ( (conf.ap.ssid_hidden == 0) ? "Yes" : "No");
                 espresponse->print ("\n");
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
                espresponse->print ("\n");
                espresponse->print ("Max Connections: ");
                espresponse->print (String(conf.ap.max_connection).c_str());
                espresponse->print ("\n");
                espresponse->print ("DHCP Server: ");
                tcpip_adapter_dhcp_status_t dhcp_status;
                tcpip_adapter_dhcps_get_status (TCPIP_ADAPTER_IF_AP, &dhcp_status);
                if (dhcp_status == TCPIP_ADAPTER_DHCP_STARTED)espresponse->print ("Started");
                else espresponse->print ("Stopped");
                espresponse->print ("\n");
                espresponse->print ("IP: ");
                espresponse->print (WiFi.softAPIP().toString().c_str());
                espresponse->print ("\n");
                tcpip_adapter_ip_info_t ip_AP;
                tcpip_adapter_get_ip_info (TCPIP_ADAPTER_IF_AP, &ip_AP);
                espresponse->print ("Gateway: ");
                espresponse->print (IPAddress (ip_AP.gw.addr).toString().c_str());
                espresponse->print ("\n");
                espresponse->print ("Mask: ");
                espresponse->print (IPAddress (ip_AP.netmask.addr).toString().c_str());
                espresponse->print ("\n");
                espresponse->print ("Connected clients: ");
                wifi_sta_list_t station;
                tcpip_adapter_sta_list_t tcpip_sta_list;
                esp_wifi_ap_get_sta_list (&station);
                tcpip_adapter_get_sta_list (&station, &tcpip_sta_list);
                espresponse->print (String(station.num).c_str());
                espresponse->print ("\n");
                for (int i = 0; i < station.num; i++) {
                    espresponse->print (mac2str(tcpip_sta_list.sta[i].mac));
                    espresponse->print (" ");
                    espresponse->print ( IPAddress (tcpip_sta_list.sta[i].ip.addr).toString().c_str());
                    espresponse->print ("\n");
                    }
                espresponse->print ("Disabled Mode: ");
                espresponse->print ("STA (");
                espresponse->print (WiFi.macAddress().c_str());
                espresponse->print (")");
                espresponse->print ("\n");
            } else if (WiFi.getMode() == WIFI_AP_STA) //we should not be in this state but just in case ....
            {
               espresponse->print ("Mixed");
               espresponse->print ("\n");
               espresponse->print ("STA (");
               espresponse->print (WiFi.macAddress().c_str());
               espresponse->print (")");
               espresponse->print ("\n");
               espresponse->print ("AP (");
               espresponse->print (WiFi.softAPmacAddress().c_str());
               espresponse->print (")");
               espresponse->print ("\n");
        
            } else { //we should not be there if no wifi ....
               espresponse->print ("Wifi Off");
               espresponse->print ("\n");
            }
            //TODO to complete
            espresponse->print ("FW version: ");
            espresponse->print (GRBL_VERSION);
            espresponse->print (" (");
            espresponse->print (GRBL_VERSION_BUILD);
            espresponse->print (") (ESP32)");
            }
            break;
                 //Set ESP mode
         //cmd is RESTART
         //[ESP444]<cmd>
      case 444:
            parameter = get_param(cmd_params,"", true);
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) {
                response = false;
            } else
#endif
            {
                if (parameter=="RESTART") {
                 grbl_send(CLIENT_ALL,"[MSG:Restart ongoing]\r\n");
                 wifi_config.restart_ESP();
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
                    espresponse->println ("error");
                } else espresponse->println ("ok");
                prefs.end();
                
            } else {
                if (isLocalPasswordValid (parameter.c_str() ) ) {
                    Preferences prefs;
                    prefs.begin(NAMESPACE, false);
                    if (prefs.putString(USER_PWD_ENTRY, parameter) != parameter.length()) {
                        response = false;
                        espresponse->println ("error");
                    } else espresponse->println ("ok");
                    prefs.end();
                } else {
                    espresponse->println ("error");
                    response = false;
                }
            }
        } else {
            espresponse->println ("error");
            response = false;
        }
        break;
    }
#endif
    //[ESP700]<filename>
    case 700: { //read local file
#ifdef ENABLE_AUTHENTICATION
            if (auth_type == LEVEL_GUEST) return false;
#endif
        cmd_params.trim() ;
        if ( (cmd_params.length() > 0) && (cmd_params[0] != '/') ) {
            cmd_params = "/" + cmd_params;
        }
        File currentfile = SPIFFS.open (cmd_params, FILE_READ);
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
                        uint8_t index = 0;
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
                        wifi_config.wait (1);
                    }
                wifi_config.wait (1);
                }
            }
            currentfile.close();
             if (espresponse)espresponse->println ("ok");
        } else {
             if (espresponse)espresponse->println ("error");
            response = false;
        }
        break;
    }
    //Format SPIFFS
    //[ESP710]FORMAT pwd=<admin password>
    case 710:
#ifdef ENABLE_AUTHENTICATION
            if (auth_type != LEVEL_ADMIN) return false;
#endif
        parameter = get_param (cmd_params, "", true);
#ifdef ENABLE_AUTHENTICATION
        if (auth_type != LEVEL_ADMIN) {
            espresponse->println ("error");
            response = false;
            break;
        } else
#endif
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
            resp += " # webcommunication: Sync: ";
            resp += String(_port + 1);
            resp += "# hostname:";
            resp += _hostname;
            if (WiFi.getMode() == WIFI_AP)resp += "(AP mode)";
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

//login status check
void Web_Server::handle_login()
{
#ifdef ENABLE_AUTHENTICATION
    String smsg;
    String sUser,sPassword;
    String auths;
    int code = 200;
    bool msg_alert_error=false;
    //disconnect can be done anytime no need to check credential
    if (_webserver->hasArg("DISCONNECT")) {
        String cookie = _webserver->header("Cookie");
        int pos = cookie.indexOf("ESPSESSIONID=");
        String sessionID;
        if (pos!= -1) {
            int pos2 = cookie.indexOf(";",pos);
            sessionID = cookie.substring(pos+strlen("ESPSESSIONID="),pos2);
            }
        ClearAuthIP(_webserver->client().remoteIP(), sessionID.c_str());
        _webserver->sendHeader("Set-Cookie","ESPSESSIONID=0");
        _webserver->sendHeader("Cache-Control","no-cache");
        String buffer2send = "{\"status\":\"Ok\",\"authentication_lvl\":\"guest\"}";
        _webserver->send(code, "application/json", buffer2send);
        //_webserver->client().stop();
        return;
    }

    level_authenticate_type auth_level = is_authenticated();
   if (auth_level == LEVEL_GUEST) auths = "guest";
    else if (auth_level == LEVEL_USER) auths = "user";
    else if (auth_level == LEVEL_ADMIN) auths = "admin";
    else auths = "???";

    //check is it is a submission or a query
    if (_webserver->hasArg("SUBMIT")) {
        //is there a correct list of query?
        if ( _webserver->hasArg("PASSWORD") && _webserver->hasArg("USER")) {
            //USER
            sUser = _webserver->arg("USER");
            if ( !((sUser == DEFAULT_ADMIN_LOGIN) || (sUser == DEFAULT_USER_LOGIN))) {
                msg_alert_error=true;
                smsg = "Error : Incorrect User";
                code = 401;
            }
            if (msg_alert_error == false) {
                //Password
                sPassword = _webserver->arg("PASSWORD");
                String sadminPassword;

                Preferences prefs;
                prefs.begin(NAMESPACE, true);
                String defV = DEFAULT_ADMIN_PWD;
                sadminPassword = prefs.getString(ADMIN_PWD_ENTRY, defV);
                String suserPassword;
                defV = DEFAULT_USER_PWD;
                suserPassword = prefs.getString(USER_PWD_ENTRY, defV);
                prefs.end();

                if(!(((sUser == DEFAULT_ADMIN_LOGIN) && (strcmp(sPassword.c_str(),sadminPassword.c_str()) == 0)) ||
                        ((sUser == DEFAULT_USER_LOGIN) && (strcmp(sPassword.c_str(),suserPassword.c_str()) == 0)))) {
                    msg_alert_error=true;
                    smsg = "Error: Incorrect password";
                    code = 401;
                }
            }
        } else {
            msg_alert_error=true;
            smsg = "Error: Missing data";
            code = 500;
        }
        //change password
        if (_webserver->hasArg("PASSWORD") && _webserver->hasArg("USER") && _webserver->hasArg("NEWPASSWORD") && (msg_alert_error==false) ) {
            String newpassword =  _webserver->arg("NEWPASSWORD");
            if (isLocalPasswordValid(newpassword.c_str())) {
                String spos;
                if(sUser == DEFAULT_ADMIN_LOGIN) spos = ADMIN_PWD_ENTRY;
                else spos = USER_PWD_ENTRY;
                
                Preferences prefs;
                prefs.begin(NAMESPACE, false);
                if (prefs.putString(spos.c_str(), newpassword) != newpassword.length()) {
                     msg_alert_error = true;
                     smsg = "Error: Cannot apply changes";
                     code = 500;
                }
                prefs.end();
            } else {
                msg_alert_error=true;
                smsg = "Error: Incorrect password";
                code = 500;
            }
        }
   if ((code == 200) || (code == 500)) {
      level_authenticate_type current_auth_level;
      if(sUser == DEFAULT_ADMIN_LOGIN) {
            current_auth_level = LEVEL_ADMIN;
        } else  if(sUser == DEFAULT_USER_LOGIN){
            current_auth_level = LEVEL_USER;
        } else {
            current_auth_level = LEVEL_GUEST;
        }
        //create Session
        if ((current_auth_level != auth_level) || (auth_level== LEVEL_GUEST)) {
            auth_ip * current_auth = new auth_ip;
            current_auth->level = current_auth_level;
            current_auth->ip=_webserver->client().remoteIP();
            strcpy(current_auth->sessionID,create_session_ID());
            strcpy(current_auth->userID,sUser.c_str());
            current_auth->last_time=millis();
            if (AddAuthIP(current_auth)) {
                String tmps ="ESPSESSIONID=";
                tmps+=current_auth->sessionID;
                _webserver->sendHeader("Set-Cookie",tmps);
                _webserver->sendHeader("Cache-Control","no-cache");
                switch(current_auth->level) {
                    case LEVEL_ADMIN:
                        auths = "admin";
                        break;
                     case LEVEL_USER:
                        auths = "user";
                        break;
                    default:
                        auths = "guest";
                        break;
                    }
            } else {
                delete current_auth;
                msg_alert_error=true;
                code = 500;
                smsg = "Error: Too many connections";
            }
        }
   }
    if (code == 200) smsg = "Ok";

    //build  JSON
    String buffer2send = "{\"status\":\"" + smsg + "\",\"authentication_lvl\":\"";
    buffer2send += auths;
    buffer2send += "\"}";
    _webserver->send(code, "application/json", buffer2send);
    } else {
    if (auth_level != LEVEL_GUEST) {
        String cookie = _webserver->header("Cookie");
        int pos = cookie.indexOf("ESPSESSIONID=");
        String sessionID;
        if (pos!= -1) {
            int pos2 = cookie.indexOf(";",pos);
            sessionID = cookie.substring(pos+strlen("ESPSESSIONID="),pos2);
            auth_ip * current_auth_info = GetAuth(_webserver->client().remoteIP(), sessionID.c_str());
            if (current_auth_info != NULL){
                    sUser = current_auth_info->userID;
                }
        }
    }
    String buffer2send = "{\"status\":\"200\",\"authentication_lvl\":\"";
    buffer2send += auths;
    buffer2send += "\",\"user\":\"";
    buffer2send += sUser;
    buffer2send +="\"}";
    _webserver->send(code, "application/json", buffer2send);
    }
#else
	_webserver->sendHeader("Cache-Control","no-cache");
    _webserver->send(200, "application/json", "{\"status\":\"Ok\",\"authentication_lvl\":\"admin\"}");
#endif
}
//SPIFFS
//SPIFFS files list and file commands
void Web_Server::handleFileList ()
{
    level_authenticate_type auth_level = is_authenticated();
    if (auth_level == LEVEL_GUEST) {
        _upload_status = UPLOAD_STATUS_NONE;
        _webserver->send (401, "text/plain", "Authentication failed!\n");
        return;
    }
    String path ;
    String status = "Ok";
    if ( (_upload_status == UPLOAD_STATUS_FAILED) || (_upload_status == UPLOAD_STATUS_CANCELLED) ) {
        status = "Upload failed";
    }
    //be sure root is correct according authentication
    if (auth_level == LEVEL_ADMIN) {
        path = "/";
    } else {
        path = "/user";
    }
    //get current path
    if (_webserver->hasArg ("path") ) {
        path += _webserver->arg ("path") ;
    }
    //to have a clean path
    path.trim();
    path.replace ("//", "/");
    if (path[path.length() - 1] != '/') {
        path += "/";
    }
    //check if query need some action
    if (_webserver->hasArg ("action") ) {
        //delete a file
        if (_webserver->arg ("action") == "delete" && _webserver->hasArg ("filename") ) {
            String filename;
            String shortname = _webserver->arg ("filename");
            shortname.replace ("/", "");
            filename = path + _webserver->arg ("filename");
            filename.replace ("//", "/");
            if (!SPIFFS.exists (filename) ) {
                status = shortname + " does not exists!";
            } else {
                if (SPIFFS.remove (filename) ) {
                    status = shortname + " deleted";
                    //what happen if no "/." and no other subfiles ?
                    String ptmp = path;
                    if ( (path != "/") && (path[path.length() - 1] = '/') ) {
                        ptmp = path.substring (0, path.length() - 1);
                    }
                    File dir = SPIFFS.open (ptmp);
                    File dircontent = dir.openNextFile();
                    if (!dircontent) {
                        //keep directory alive even empty
                        File r = SPIFFS.open (path + "/.", FILE_WRITE);
                        if (r) {
                            r.close();
                        }
                    }
                } else {
                    status = "Cannot deleted " ;
                    status += shortname ;
                }
            }
        }
        //delete a directory
        if (_webserver->arg ("action") == "deletedir" && _webserver->hasArg ("filename") ) {
            String filename;
            String shortname = _webserver->arg ("filename");
            shortname.replace ("/", "");
            filename = path + _webserver->arg ("filename");
            filename += "/";
            filename.replace ("//", "/");
            if (filename != "/") {
                bool delete_error = false;
                File dir = SPIFFS.open (path + shortname);
                {
                    File file2deleted = dir.openNextFile();
                    while (file2deleted) {
                        String fullpath = file2deleted.name();
                        if (!SPIFFS.remove (fullpath) ) {
                            delete_error = true;
                            status = "Cannot deleted " ;
                            status += fullpath;
                        }
                        file2deleted = dir.openNextFile();
                    }
                }
                if (!delete_error) {
                    status = shortname ;
                    status += " deleted";
                }
            }
        }
        //create a directory
        if (_webserver->arg ("action") == "createdir" && _webserver->hasArg ("filename") ) {
            String filename;
            filename = path + _webserver->arg ("filename") + "/.";
            String shortname = _webserver->arg ("filename");
            shortname.replace ("/", "");
            filename.replace ("//", "/");
            if (SPIFFS.exists (filename) ) {
                status = shortname + " already exists!";
            } else {
                File r = SPIFFS.open (filename, FILE_WRITE);
                if (!r) {
                    status = "Cannot create ";
                    status += shortname ;
                } else {
                    r.close();
                    status = shortname + " created";
                }
            }
        }
    }
    String jsonfile = "{";
    String ptmp = path;
    if ( (path != "/") && (path[path.length() - 1] = '/') ) {
        ptmp = path.substring (0, path.length() - 1);
    }
    File dir = SPIFFS.open (ptmp);
    jsonfile += "\"files\":[";
    bool firstentry = true;
    String subdirlist = "";
    File fileparsed = dir.openNextFile();
    while (fileparsed) {
        String filename = fileparsed.name();
        String size = "";
        bool addtolist = true;
        //remove path from name
        filename = filename.substring (path.length(), filename.length() );
        //check if file or subfile
        if (filename.indexOf ("/") > -1) {
            //Do not rely on "/." to define directory as SPIFFS upload won't create it but directly files
            //and no need to overload SPIFFS if not necessary to create "/." if no need
            //it will reduce SPIFFS available space so limit it to creation
            filename = filename.substring (0, filename.indexOf ("/") );
            String tag = "*";
            tag += filename + "*";
            if (subdirlist.indexOf (tag) > -1 || filename.length() == 0) { //already in list
                addtolist = false; //no need to add
            } else {
                size = "-1"; //it is subfile so display only directory, size will be -1 to describe it is directory
                if (subdirlist.length() == 0) {
                    subdirlist += "*";
                }
                subdirlist += filename + "*"; //add to list
            }
        } else {
            //do not add "." file
            if (! ( (filename == ".") || (filename == "") ) ) {
                size = formatBytes (fileparsed.size() );
            } else {
                addtolist = false;
            }
        }
        if (addtolist) {
            if (!firstentry) {
                jsonfile += ",";
            } else {
                firstentry = false;
            }
            jsonfile += "{";
            jsonfile += "\"name\":\"";
            jsonfile += filename;
            jsonfile += "\",\"size\":\"";
            jsonfile += size;
            jsonfile += "\"";
            jsonfile += "}";
        }
        fileparsed = dir.openNextFile();
    }
    jsonfile += "],";
    jsonfile += "\"path\":\"" + path + "\",";
    jsonfile += "\"status\":\"" + status + "\",";
    size_t totalBytes;
    size_t usedBytes;
    totalBytes = SPIFFS.totalBytes();
    usedBytes = SPIFFS.usedBytes();
    jsonfile += "\"total\":\"" + formatBytes (totalBytes) + "\",";
    jsonfile += "\"used\":\"" + formatBytes (usedBytes) + "\",";
    jsonfile.concat (F ("\"occupation\":\"") );
    jsonfile += String (100 * usedBytes / totalBytes);
    jsonfile += "\"";
    jsonfile += "}";
    path = "";
    _webserver->sendHeader("Cache-Control", "no-cache");
    _webserver->send(200, "application/json", jsonfile);
    _upload_status = UPLOAD_STATUS_NONE;
}

//SPIFFS files uploader handle
void Web_Server::SPIFFSFileupload ()
{
     //get authentication status
    level_authenticate_type auth_level= is_authenticated();
    //Guest cannot upload - only admin
    if (auth_level == LEVEL_GUEST) {
        _upload_status = UPLOAD_STATUS_CANCELLED;
        grbl_send(CLIENT_ALL,"[MSG:Upload rejected]\r\n");
        _webserver->client().stop();
        return;
    }
    static String filename;
    static File fsUploadFile = (File)0;
    
    HTTPUpload& upload = _webserver->upload();
    //Upload start
    //**************
    if(upload.status == UPLOAD_FILE_START) {
        String upload_filename = upload.filename;
        if (upload_filename[0] != '/') filename = "/" + upload_filename;
        else filename = upload.filename;
        //according User or Admin the root is different as user is isolate to /user when admin has full access
        if(auth_level != LEVEL_ADMIN) {
            upload_filename = filename;
            filename = "/user" + upload_filename;
        }
        
        if (SPIFFS.exists (filename) ) {
            SPIFFS.remove (filename);
        }
        if (fsUploadFile ) {
            fsUploadFile.close();
        }
        //create file
        fsUploadFile = SPIFFS.open(filename, FILE_WRITE);
        //check If creation succeed
        if (fsUploadFile) {
            //if yes upload is started
            _upload_status= UPLOAD_STATUS_ONGOING;
        } else {
            //if no set cancel flag
            _upload_status=UPLOAD_STATUS_CANCELLED;
            grbl_send(CLIENT_ALL,"[MSG:Upload error]\r\n");
            _webserver->client().stop();
        }
        //Upload write
        //**************
    } else if(upload.status == UPLOAD_FILE_WRITE) {
        //check if file is available and no error
        if(fsUploadFile && _upload_status == UPLOAD_STATUS_ONGOING) {
            //no error so write post date
            fsUploadFile.write(upload.buf, upload.currentSize);
        } else {
            //we have a problem set flag UPLOAD_STATUS_CANCELLED
            _upload_status=UPLOAD_STATUS_CANCELLED;
            fsUploadFile.close();
            if (SPIFFS.exists (filename) ) {
                SPIFFS.remove (filename);
            }
            _webserver->client().stop();
            grbl_send(CLIENT_ALL,"[MSG:Upload error]\r\n");
        }
        //Upload end
        //**************
    } else if(upload.status == UPLOAD_FILE_END) {
        //check if file is still open
        if(fsUploadFile) {
            //close it
            fsUploadFile.close();
            //check size 
            String  sizeargname  = upload.filename + "S";
            fsUploadFile = SPIFFS.open (filename, FILE_READ);
            uint32_t filesize = fsUploadFile.size();
            fsUploadFile.close();
            if (_webserver->hasArg (sizeargname.c_str()) ) {
                if (_webserver->arg (sizeargname.c_str()) != String(filesize)) {
                    _upload_status = UPLOAD_STATUS_FAILED;
                    SPIFFS.remove (filename);
                }
            } 
            if (_upload_status == UPLOAD_STATUS_ONGOING) {
                _upload_status = UPLOAD_STATUS_SUCCESSFUL;
            } else grbl_send(CLIENT_ALL,"[MSG:Upload error]\r\n");
        } else {
            //we have a problem set flag UPLOAD_STATUS_CANCELLED
            _upload_status=UPLOAD_STATUS_CANCELLED;
            _webserver->client().stop();
            if (SPIFFS.exists (filename) ) {
                SPIFFS.remove (filename);
                }
            grbl_send(CLIENT_ALL,"[MSG:Upload error]\r\n");
            
        }
        //Upload cancelled
        //**************
    } else {
            if (_upload_status == UPLOAD_STATUS_ONGOING) {
                _upload_status = UPLOAD_STATUS_CANCELLED;
            }
            if(fsUploadFile)fsUploadFile.close();
            if (SPIFFS.exists (filename) ) {
                SPIFFS.remove (filename);
            }
            grbl_send(CLIENT_ALL,"[MSG:Upload error]\r\n");
    }
    wifi_config.wait(0);
}

//Web Update handler 
void Web_Server::handleUpdate ()
{
    level_authenticate_type auth_level = is_authenticated();
    if (auth_level != LEVEL_ADMIN) {
        _upload_status = UPLOAD_STATUS_NONE;
        _webserver->send (403, "text/plain", "Not allowed, log in first!\n");
        return;
    }
    String jsonfile = "{\"status\":\"" ;
    jsonfile += String(_upload_status);
    jsonfile += "\"}";
    //send status
    _webserver->sendHeader("Cache-Control", "no-cache");
    _webserver->send(200, "application/json", jsonfile);
    //if success restart
    if (_upload_status == UPLOAD_STATUS_SUCCESSFUL) {
        wifi_config.wait(1000);
        wifi_config.restart_ESP();
    } else {
        _upload_status = UPLOAD_STATUS_NONE;
    }
}

//File upload for Web update
void Web_Server::WebUpdateUpload ()
{
    static size_t last_upload_update;
    static uint32_t maxSketchSpace ;
    //only admin can update FW
    if (is_authenticated() != LEVEL_ADMIN) {
        _upload_status = UPLOAD_STATUS_CANCELLED;
       _webserver->client().stop();
        grbl_send(CLIENT_ALL,"[MSG:Upload rejected]\r\n");
        return;
    }
    
    //get current file ID
    HTTPUpload& upload = _webserver->upload();
    //Upload start
    //**************
    if(upload.status == UPLOAD_FILE_START) {
        grbl_send(CLIENT_ALL,"[MSG:Update Firmware]\r\n");
        _upload_status= UPLOAD_STATUS_ONGOING;

        //Not sure can do OTA on 2Mb board
        maxSketchSpace = (ESP.getFlashChipSize() > 0x20000) ? 0x140000 : 0x140000 / 2;
        last_upload_update = 0;
        if(!Update.begin(maxSketchSpace)) { //start with max available size
            _upload_status=UPLOAD_STATUS_CANCELLED;
            grbl_send(CLIENT_ALL,"[MSG:Update cancelled]\r\n");
            _webserver->client().stop();
            return;
        } else {
            grbl_send(CLIENT_ALL,"\n[MSG:Update 0%]\r\n");
        }
        //Upload write
        //**************
    } else if(upload.status == UPLOAD_FILE_WRITE) {
        //check if no error
        if (_upload_status == UPLOAD_STATUS_ONGOING) {
            //we do not know the total file size yet but we know the available space so let's use it
            if ( ((100 * upload.totalSize) / maxSketchSpace) !=last_upload_update) {
                last_upload_update = (100 * upload.totalSize) / maxSketchSpace;
                String s = "Update ";
                s+= String(last_upload_update);
                s+="%";
                grbl_sendf(CLIENT_ALL,"[MSG:%s]\r\n", s.c_str());
            }
            if(Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                _upload_status=UPLOAD_STATUS_CANCELLED;
            }
        }
        //Upload end
        //**************
    } else if(upload.status == UPLOAD_FILE_END) {
        if(Update.end(true)) { //true to set the size to the current progress
            //Now Reboot
            grbl_send(CLIENT_ALL,"[MSG:Update 100%]\r\n");
            _upload_status=UPLOAD_STATUS_SUCCESSFUL;
        }
    } else if(upload.status == UPLOAD_FILE_ABORTED) {
        grbl_send(CLIENT_ALL,"[MSG:Update failed]\r\n");
        Update.end();
        _upload_status=UPLOAD_STATUS_CANCELLED;
    }
    wifi_config.wait(0);
}


#ifdef ENABLE_SD_CARD

//Function to delete not empty directory on SD card
bool  Web_Server::deleteRecursive(String path)
{
    bool result = true;
    File file = SD.open((char *)path.c_str());
    //failed
    if (!file) {
		return false;
	}
    if(!file.isDirectory()) {
        file.close();
        //return if success or not
        return SD.remove((char *)path.c_str());
    }
    file.rewindDirectory();
    while(true) {
        File entry = file.openNextFile();
        if (!entry) {
            break;
        }
        String entryPath = entry.name();
        if(entry.isDirectory()) {
            entry.close();
            if(!deleteRecursive(entryPath)) {
                result =  false;
            }
        } else {
            entry.close();
            if (!SD.remove((char *)entryPath.c_str())) {
                result = false;
                break;
            }
        }
       wifi_config.wait(0); //wdtFeed
    }
    file.close();
    if (result) return SD.rmdir((char *)path.c_str());
    else return false;
}

//direct SD files list//////////////////////////////////////////////////
void Web_Server::handle_direct_SDFileList()
{
    //this is only for admin and user
    if (is_authenticated() == LEVEL_GUEST) {
        _upload_status=UPLOAD_STATUS_NONE;
        _webserver->send(401, "application/json", "{\"status\":\"Authentication failed!\"}");
        return;
    }

    String path="/";
    String sstatus="Ok";
    if ((_upload_status == UPLOAD_STATUS_FAILED) || (_upload_status == UPLOAD_STATUS_CANCELLED)) {
        sstatus = "Upload failed";
        _upload_status = UPLOAD_STATUS_NONE;
    }
    bool list_files = true;
    uint32_t totalspace = 0;
    uint32_t usedspace = 0;
    if (get_sd_state(true) != SDCARD_IDLE) {
        _webserver->sendHeader("Cache-Control","no-cache");
        _webserver->send(200, "application/json", "{\"status\":\"No SD Card\"}");
        return;
    }
    set_sd_state(SDCARD_BUSY_PARSING);
    //get current path
    if(_webserver->hasArg("path")) {
        path += _webserver->arg("path") ;
    }
    //to have a clean path
    path.trim();
    path.replace("//","/");
    if (path[path.length()-1] !='/') {
        path +="/";
    }
    //check if query need some action
    if(_webserver->hasArg("action")) {
        //delete a file
        if(_webserver->arg("action") == "delete" && _webserver->hasArg("filename")) {
            String filename;
            String shortname = _webserver->arg("filename");
            filename = path + shortname;
            shortname.replace("/","");
            filename.replace("//","/");
            if(!SD.exists((char *)filename.c_str())) {
                sstatus = shortname + " does not exist!";
            } else {
                if (SD.remove((char *)filename.c_str())) {
                    sstatus = shortname + " deleted";
                } else {
                    sstatus = "Cannot deleted " ;
                    sstatus+=shortname ;
                }
            }
        }
        //delete a directory
        if( _webserver->arg("action") == "deletedir" &&  _webserver->hasArg("filename")) {
            String filename;
            String shortname = _webserver->arg("filename");
            shortname.replace("/","");
            filename = path + "/" + shortname;
            filename.replace("//","/");
            if (filename != "/") {
                if(!SD.exists((char *)filename.c_str())) {
                    sstatus = shortname + " does not exist!";
                } else {
                    if (!deleteRecursive(filename)) {
                        sstatus ="Error deleting: ";
                        sstatus += shortname ;
                    } else {
                        sstatus = shortname ;
                        sstatus+=" deleted";
                    }
                }
            } else {
                sstatus ="Cannot delete root";
            }
        }
        //create a directory
        if( _webserver->arg("action")=="createdir" &&  _webserver->hasArg("filename")) {
            String filename;
            String shortname =  _webserver->arg("filename"); 
            filename = path + shortname;
            shortname.replace("/","");
            filename.replace("//","/");
            if(SD.exists((char *)filename.c_str())) {
                sstatus = shortname + " already exists!";
            } else {
                if (!SD.mkdir((char *)filename.c_str())) {
                    sstatus = "Cannot create ";
                    sstatus += shortname ;
                } else {
                    sstatus = shortname + " created";
                }
            }
        }
    }
    //check if no need build file list
    if( _webserver->hasArg("dontlist")) {
        if( _webserver->arg("dontlist") == "yes") {
            list_files = false;
        }
    }
    String jsonfile = "{" ;
    jsonfile+="\"files\":[";

    if (path!="/")path = path.substring(0,path.length()-1);
    if (path!="/" && !SD.exists((char *)path.c_str())) {

        String s =  "{\"status\":\" ";
        s += path;
        s+=  " does not exist on SD Card\"}";
         _webserver->send(200, "application/json", s.c_str());
        return;
    }
    if (list_files) {
        File dir = SD.open((char *)path.c_str());
        if (!dir) {
        }
        if(!dir.isDirectory()) {
            dir.close();
        }
        dir.rewindDirectory();
        File entry =  dir.openNextFile();
        int i = 0;
        while(entry) {
            wifi_config.wait (1);
            if (i>0) {
                jsonfile+=",";
            }
            jsonfile+="{\"name\":\"";
            String tmpname = entry.name();
            int pos = tmpname.lastIndexOf("/");
            tmpname = tmpname.substring(pos+1);
            jsonfile+=tmpname;
            jsonfile+="\",\"shortname\":\""; //No need here
            jsonfile+=tmpname;
            jsonfile+="\",\"size\":\"";
            if (entry.isDirectory()) {
                jsonfile+="-1";
            } else {
                // files have sizes, directories do not
                jsonfile+=formatBytes(entry.size());
            }
            jsonfile+="\",\"datetime\":\"";
            //TODO - can be done later
            jsonfile+="\"}";
            i++;
            entry.close();
            entry =  dir.openNextFile();
            }
        dir.close();
    }
    jsonfile+="],\"path\":\"";
    jsonfile+=path + "\",";
    static uint32_t volTotal = 1;
    static uint32_t volFree = 0;
    jsonfile+="\"total\":\"";
    String stotalspace,susedspace;
    //SDCard are in GB or MB but no less
    totalspace = SD.totalBytes();
    usedspace = SD.usedBytes();
    stotalspace = formatBytes(totalspace);
    susedspace =  formatBytes(usedspace);

    uint32_t  occupedspace = (volFree/volTotal)*100;
    //minimum if even one byte is used is 1%
    if ( (occupedspace <= 1) && (volTotal!=volFree)) {
        occupedspace=1;
    }
    if (totalspace) {
        jsonfile+=  stotalspace ;
    } else {
        jsonfile+=  "-1";
    }
    jsonfile+="\",\"used\":\"";
    jsonfile+=  susedspace ;
    jsonfile+="\",\"occupation\":\"";
    if (totalspace) {
        jsonfile+=  String(occupedspace);
    } else {
        jsonfile+=  "-1";
    }
    jsonfile+= "\",";
    jsonfile+= "\"mode\":\"direct\",";
    jsonfile+= "\"status\":\"";
    jsonfile+=sstatus + "\"";
    jsonfile+= "}";
    _webserver->sendHeader("Cache-Control","no-cache");
    _webserver->send (200, "application/json", jsonfile.c_str());
    _upload_status=UPLOAD_STATUS_NONE;
    set_sd_state(SDCARD_IDLE);
}

//SD File upload with direct access to SD///////////////////////////////
void Web_Server::SDFile_direct_upload()
{
    static String filename ;
    static File sdUploadFile;
    //this is only for admin and user
    if (is_authenticated() == LEVEL_GUEST) {
        _upload_status=UPLOAD_STATUS_NONE;
        _webserver->send(401, "application/json", "{\"status\":\"Authentication failed!\"}");
        return;
    }
    //retrieve current file id
    HTTPUpload& upload = _webserver->upload();
    //Upload start
    //**************
    if(upload.status == UPLOAD_FILE_START) {
        filename= upload.filename;
        //on SD need to add / if not present
        if (filename[0]!='/') {
            filename= "/"+upload.filename;
        }
        //check if SD Card is available
        if ( get_sd_state(true) != SDCARD_IDLE) {
            _upload_status=UPLOAD_STATUS_CANCELLED;
            grbl_send(CLIENT_ALL,"[MSG:Upload cancelled]\r\n");
            _webserver->client().stop();
            return;
        }
        set_sd_state(SDCARD_BUSY_UPLOADING);
        //delete file on SD Card if already present
        if(SD.exists((char *)filename.c_str())) {
            SD.remove((char *)filename.c_str());
        }
        //Create file for writing
        sdUploadFile = SD.open((char *)filename.c_str(), FILE_WRITE);
        //check if creation succeed
        if (!sdUploadFile) {
            //if creation failed
            _upload_status=UPLOAD_STATUS_FAILED;
            set_sd_state(SDCARD_IDLE);
            grbl_send(CLIENT_ALL,"[MSG:Upload failed]\r\n");
            _webserver->client().stop();
        }
        //if creation succeed set flag UPLOAD_STATUS_ONGOING
        else {
            _upload_status= UPLOAD_STATUS_ONGOING;
        }
        //Upload write
        //**************
    } else if(upload.status == UPLOAD_FILE_WRITE) {
        if(sdUploadFile && (_upload_status == UPLOAD_STATUS_ONGOING) && (get_sd_state(false) == SDCARD_BUSY_UPLOADING)) {
            //no error write post data
            sdUploadFile.write(upload.buf, upload.currentSize);
        } else { //if error set flag UPLOAD_STATUS_FAILED
            _upload_status = UPLOAD_STATUS_FAILED;
            set_sd_state(SDCARD_IDLE);
            grbl_send(CLIENT_ALL,"[MSG:Upload failed]\r\n");
            _webserver->client().stop();
        }
        //Upload end
        //**************
    } else if(upload.status == UPLOAD_FILE_END) {
        //if file is open close it
        if(sdUploadFile) {
            sdUploadFile.close();
            //TODO Check size
            String  sizeargname  = upload.filename + "S";
            if (_webserver->hasArg (sizeargname.c_str()) ) {
                uint32_t filesize = 0;
                sdUploadFile = SD.open (filename.c_str(), FILE_READ);
                filesize = sdUploadFile.size();
                sdUploadFile.close();
                if (_webserver->arg (sizeargname.c_str()) != String(filesize)) {
                    _upload_status = UPLOAD_STATUS_FAILED;
                    SD.remove (filename.c_str());
                    }
            }
        } else {
            _upload_status = UPLOAD_STATUS_FAILED;
            set_sd_state(SDCARD_IDLE);
            grbl_send(CLIENT_ALL,"[MSG:Upload failed]\r\n");
        }
        if (_upload_status == UPLOAD_STATUS_ONGOING) {
            _upload_status = UPLOAD_STATUS_SUCCESSFUL;
        }
        set_sd_state(SDCARD_IDLE);
    } else {//Upload cancelled
        _upload_status=UPLOAD_STATUS_FAILED;
        set_sd_state(SDCARD_IDLE);
        grbl_send(CLIENT_ALL,"[MSG:Upload failed]\r\n");
        _webserver->client().stop();
        if(sdUploadFile) {
            sdUploadFile.close();
        }
    }
    wifi_config.wait(0);
}
#endif

void Web_Server::handle(){
static uint32_t timeout = millis();
#ifdef ENABLE_CAPTIVE_PORTAL
    if(WiFi.getMode() == WIFI_AP){
        dnsServer.processNextRequest();
    }
#endif
    if (_webserver)_webserver->handleClient();
    if (_socket_server && _setupdone)_socket_server->loop();
    if ((millis() - timeout) > 10000) {
        if (_socket_server){
             String s = "PING:";
             s+=String(_id_connection);
            _socket_server->broadcastTXT(s);
            timeout=millis();
        }
    }
}


void Web_Server::handle_Websocket_Event(uint8_t num, uint8_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            //USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = _socket_server->remoteIP(num);
                //USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                String s = "CURRENT_ID:" + String(num);
                // send message to client
                _id_connection = num;
                _socket_server->sendTXT(_id_connection, s);
                s = "ACTIVE_ID:" + String(_id_connection);
                _socket_server->broadcastTXT(s);
            }
            break;
        case WStype_TEXT:
            //USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            //USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
            //hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
        default:
            break;
    }

}

//just simple helper to convert mac address to string
char * Web_Server::mac2str (uint8_t mac [8])
{
    static char macstr [18];
    if (0 > sprintf (macstr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ) {
        strcpy (macstr, "00:00:00:00:00:00");
    }
    return macstr;
}

String Web_Server::get_Splited_Value(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}


//helper to format size to readable string
String Web_Server::formatBytes (uint32_t bytes)
{
    if (bytes < 1024) {
        return String (bytes) + " B";
    } else if (bytes < (1024 * 1024) ) {
        return String (bytes / 1024.0) + " KB";
    } else if (bytes < (1024 * 1024 * 1024) ) {
        return String (bytes / 1024.0 / 1024.0) + " MB";
    } else {
        return String (bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
    }
}

//helper to extract content type from file extension
//Check what is the content tye according extension file
String Web_Server::getContentType (String filename)
{
    String file_name = filename;
    file_name.toLowerCase();
    if (filename.endsWith (".htm") ) {
        return "text/html";
    } else if (file_name.endsWith (".html") ) {
        return "text/html";
    } else if (file_name.endsWith (".css") ) {
        return "text/css";
    } else if (file_name.endsWith (".js") ) {
        return "application/javascript";
    } else if (file_name.endsWith (".png") ) {
        return "image/png";
    } else if (file_name.endsWith (".gif") ) {
        return "image/gif";
    } else if (file_name.endsWith (".jpeg") ) {
        return "image/jpeg";
    } else if (file_name.endsWith (".jpg") ) {
        return "image/jpeg";
    } else if (file_name.endsWith (".ico") ) {
        return "image/x-icon";
    } else if (file_name.endsWith (".xml") ) {
        return "text/xml";
    } else if (file_name.endsWith (".pdf") ) {
        return "application/x-pdf";
    } else if (file_name.endsWith (".zip") ) {
        return "application/x-zip";
    } else if (file_name.endsWith (".gz") ) {
        return "application/x-gzip";
    } else if (file_name.endsWith (".txt") ) {
        return "text/plain";
    }
    return "application/octet-stream";
}

//check authentification
level_authenticate_type Web_Server::is_authenticated()
{
#ifdef ENABLE_AUTHENTICATION
    if (_webserver->hasHeader ("Cookie") ) {
        String cookie = _webserver->header ("Cookie");
        int pos = cookie.indexOf ("ESPSESSIONID=");
        if (pos != -1) {
            int pos2 = cookie.indexOf (";", pos);
            String sessionID = cookie.substring (pos + strlen ("ESPSESSIONID="), pos2);
            IPAddress ip = _webserver->client().remoteIP();
            //check if cookie can be reset and clean table in same time
            return ResetAuthIP (ip, sessionID.c_str() );
        }
    }
    return LEVEL_GUEST;
#else
    return LEVEL_ADMIN;
#endif
}

#ifdef ENABLE_AUTHENTICATION

bool Web_Server::isLocalPasswordValid (const char * password)
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

//add the information in the linked list if possible
bool Web_Server::AddAuthIP (auth_ip * item)
{
    if (_nb_ip > MAX_AUTH_IP) {
        return false;
    }
    item->_next = _head;
    _head = item;
    _nb_ip++;
    return true;
}

//Session ID based on IP and time using 16 char
char * Web_Server::create_session_ID()
{
    static char  sessionID[17];
//reset SESSIONID
    for (int i = 0; i < 17; i++) {
        sessionID[i] = '\0';
    }
//get time
    uint32_t now = millis();
//get remote IP
    IPAddress remoteIP = _webserver->client().remoteIP();
//generate SESSIONID
    if (0 > sprintf (sessionID, "%02X%02X%02X%02X%02X%02X%02X%02X", remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3], (uint8_t) ( (now >> 0) & 0xff), (uint8_t) ( (now >> 8) & 0xff), (uint8_t) ( (now >> 16) & 0xff), (uint8_t) ( (now >> 24) & 0xff) ) ) {
        strcpy (sessionID, "NONE");
    }
    return sessionID;
}


bool Web_Server::ClearAuthIP (IPAddress ip, const char * sessionID)
{
    auth_ip * current = _head;
    auth_ip * previous = NULL;
    bool done = false;
    while (current) {
        if ( (ip == current->ip) && (strcmp (sessionID, current->sessionID) == 0) ) {
            //remove
            done = true;
            if (current == _head) {
                _head = current->_next;
                _nb_ip--;
                delete current;
                current = _head;
            } else {
                previous->_next = current->_next;
                _nb_ip--;
                delete current;
                current = previous->_next;
            }
        } else {
            previous = current;
            current = current->_next;
        }
    }
    return done;
}

//Get info
auth_ip * Web_Server::GetAuth (IPAddress ip, const char * sessionID)
{
    auth_ip * current = _head;
    auth_ip * previous = NULL;
    //get time
    //uint32_t now = millis();
    while (current) {
        if (ip == current->ip) {
            if (strcmp (sessionID, current->sessionID) == 0) {
                //found
                return current;
            }
        }
        previous = current;
        current = current->_next;
    }
    return NULL;
}

//Review all IP to reset timers
level_authenticate_type Web_Server::ResetAuthIP (IPAddress ip, const char * sessionID)
{
    auth_ip * current = _head;
    auth_ip * previous = NULL;
    //get time
    //uint32_t now = millis();
    while (current) {
        if ( (millis() - current->last_time) > 360000) {
            //remove
            if (current == _head) {
                _head = current->_next;
                _nb_ip--;
                delete current;
                current = _head;
            } else {
                previous->_next = current->_next;
                _nb_ip--;
                delete current;
                current = previous->_next;
            }
        } else {
            if (ip == current->ip) {
                if (strcmp (sessionID, current->sessionID) == 0) {
                    //reset time
                    current->last_time = millis();
                    return (level_authenticate_type) current->level;
                }
            }
            previous = current;
            current = current->_next;
        }
    }
    return LEVEL_GUEST;
}
#endif

String Web_Server::get_param (String & cmd_params, const char * id, bool withspace)
{
    static String parameter;
    String sid = id;
    int start;
    int end = -1;
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

ESPResponseStream::ESPResponseStream(WebServer * webserver){
    _header_sent=false;
    _webserver = webserver;
}

void ESPResponseStream::println(const char *data){
    print(data);
    print("\n");
}

void ESPResponseStream::print(const char *data){
    if (!_header_sent) {
         _webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
         _webserver->sendHeader("Content-Type","text/html");
         _webserver->sendHeader("Cache-Control","no-cache");
         _webserver->send(200);
         _header_sent = true;
        }
    _buffer+=data;
    if (_buffer.length() > 1200) {
        //send data
        _webserver->sendContent(_buffer);
        //reset buffer
        _buffer = "";
    }

}

void ESPResponseStream::flush(){
    if(_header_sent) {
        //send data
        if(_buffer.length() > 0)_webserver->sendContent(_buffer);
        //close connection
        _webserver->sendContent("");
        }
    _header_sent = false;
    _buffer = "";

}


#endif // Enable HTTP && ENABLE_WIFI

#endif // ARDUINO_ARCH_ESP32
