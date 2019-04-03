/*
  espresponse.cpp - GRBL_ESP response class

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
#include "espresponse.h"
#if defined (ENABLE_HTTP) && defined(ENABLE_WIFI)
#include "web_server.h"
#include <WebServer.h>
#endif

#if defined (ENABLE_HTTP) && defined(ENABLE_WIFI)
ESPResponseStream::ESPResponseStream(WebServer * webserver){
    _header_sent=false;
    _webserver = webserver;
    _client = CLIENT_WEBUI;
}
#endif

ESPResponseStream::ESPResponseStream(uint8_t client){
    _client = client;
#if defined (ENABLE_HTTP) && defined(ENABLE_WIFI)
    _header_sent=false;
    _webserver = NULL;
#endif
}

void ESPResponseStream::println(const char *data){
    print(data);
    if (_client == CLIENT_TELNET) print("\r\n");
    else print("\n");
}

//helper to format size to readable string
String ESPResponseStream::formatBytes (uint32_t bytes)
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

void ESPResponseStream::print(const char *data){
#if defined (ENABLE_HTTP) && defined(ENABLE_WIFI)
    if (_webserver) {
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
        return;
    }
#endif
    if (_client == CLIENT_WEBUI) return; //this is sanity check
    grbl_send(_client, data);
}

void ESPResponseStream::flush(){
#if defined (ENABLE_HTTP) && defined(ENABLE_WIFI)
    if (_webserver) {
        if(_header_sent) {
            //send data
            if(_buffer.length() > 0)_webserver->sendContent(_buffer);
            //close connection
            _webserver->sendContent("");
            }
        _header_sent = false;
        _buffer = "";
    }
#endif
}
