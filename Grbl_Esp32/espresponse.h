/*
  espresponse.h - GRBL_ESP response class

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

#ifndef ESPRESPONSE_h
#define ESPRESPONSE_h
#include "config.h"

#if defined (ENABLE_HTTP) && defined(ENABLE_WIFI)
class WebServer;
#endif

class ESPResponseStream{
    public:
    void print(const char *data);
    void println(const char *data);
    void flush();
    static String formatBytes (uint64_t bytes);
    uint8_t client() {return _client;}
#if defined (ENABLE_HTTP) && defined(ENABLE_WIFI)
    ESPResponseStream(WebServer * webserver);
#endif
    ESPResponseStream(uint8_t client, bool byid = true);
    ESPResponseStream();
    private:
    uint8_t _client;
#if defined (ENABLE_HTTP) && defined(ENABLE_WIFI)
    bool _header_sent;
    WebServer * _webserver;
    String _buffer;
#endif
};

#endif
