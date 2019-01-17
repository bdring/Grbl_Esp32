/*
  telnet_server.h -  telnet service functions class

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

//how many clients should be able to telnet to this ESP32
#define MAX_TLNT_CLIENTS 1

#ifndef _TELNET_SERVER_H
#define _TELNET_SERVER_H


#include "config.h"
class WiFiServer;
class WiFiClient;

#define TELNETRXBUFFERSIZE 1200
#define FLUSHTIMEOUT 500

class Telnet_Server {
    public:
    Telnet_Server();
    ~Telnet_Server();
    bool begin();
    void end();
    void handle();
    size_t write(const uint8_t *buffer, size_t size);
    int read(void);
    int peek(void);
    int available();
    int get_rx_buffer_available();
    bool push (uint8_t data);
    bool push (const uint8_t * data, int datasize);
    static uint16_t port(){return _port;}
    private:
    static bool _setupdone;
    static WiFiServer * _telnetserver;
    static WiFiClient _telnetClients[MAX_TLNT_CLIENTS];
#ifdef ENABLE_TELNET_WELCOME_MSG
    static IPAddress _telnetClientsIP[MAX_TLNT_CLIENTS];
#endif
    static uint16_t _port;
    void clearClients();
    uint32_t _lastflush;
    uint8_t _RXbuffer[TELNETRXBUFFERSIZE];
    uint16_t _RXbufferSize;
    uint16_t _RXbufferpos;
};

extern Telnet_Server telnet_server;

#endif

