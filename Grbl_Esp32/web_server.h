/*
  web_server.h -  wifi services functions class

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


#ifndef _WEB_SERVER_H
#define _WEB_SERVER_H


#include "config.h"
class WebSocketsServer;
class WebServer;

//Authentication level
typedef enum {
    LEVEL_GUEST = 0,
    LEVEL_USER = 1,
    LEVEL_ADMIN = 2
} level_authenticate_type;

#ifdef ENABLE_AUTHENTICATION
struct auth_ip {
    IPAddress ip;
    level_authenticate_type level;
    char userID[17];
    char sessionID[17];
    uint32_t last_time;
    auth_ip * _next;
};

#endif




class ESPResponseStream{
    public:
    void print(const char *data);
    void println(const char *data);
    void flush();
    ESPResponseStream(WebServer * webserver);
    private:
    bool _header_sent;
    WebServer * _webserver;
    String _buffer;
};

class Web_Server {
    public:
    Web_Server();
    ~Web_Server();
    bool begin();
    void end();
    void handle();
    static long get_client_ID();
    private:
    static bool _setupdone;
    static WebServer * _webserver;
    static long _id_connection;
    static WebSocketsServer * _socket_server;
    static uint16_t _port;
    static uint16_t _data_port;
    static String _hostname;
    static uint8_t _upload_status;
    static char * mac2str (uint8_t mac [8]);
    static String formatBytes (uint32_t bytes);
    static String getContentType (String filename);
    static String get_Splited_Value(String data, char separator, int index);
    static level_authenticate_type  is_authenticated();
#ifdef ENABLE_AUTHENTICATION
    static auth_ip * _head;
    static uint8_t _nb_ip;
    static bool AddAuthIP (auth_ip * item);
    static char * create_session_ID();
    static bool ClearAuthIP (IPAddress ip, const char * sessionID);
    static auth_ip * GetAuth (IPAddress ip, const char * sessionID);
    static level_authenticate_type ResetAuthIP (IPAddress ip, const char * sessionID);
    static bool isLocalPasswordValid (const char * password);
#endif
    static String get_param (String & cmd_params, const char * id, bool withspace);
    static bool execute_internal_command (int cmd, String cmd_params, level_authenticate_type auth_level,  ESPResponseStream  *espresponse);
#ifdef ENABLE_SSDP
    static void handle_SSDP ();
#endif
    static void handle_root();
    static void handle_login();
    static void handle_not_found ();
    static void handle_web_command ();
    static void handle_web_command_silent ();
    static void handle_Websocket_Event(uint8_t num, uint8_t type, uint8_t * payload, size_t length);
    static void SPIFFSFileupload ();
    static void handleFileList ();
    static void handleUpdate ();
    static void WebUpdateUpload ();
    static bool is_realtime_cmd(char c);
#ifdef ENABLE_SD_CARD
    static void handle_direct_SDFileList();
    static void SDFile_direct_upload();
    static bool deleteRecursive(String path);
#endif
};

extern Web_Server web_server;

#endif

