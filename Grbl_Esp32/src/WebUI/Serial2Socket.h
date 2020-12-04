#pragma once

/*
  Serial2Socket.h -  serial 2 socket functions class

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

#include <Print.h>
#include <cstring>

class WebSocketsServer;

namespace WebUI {
    class Serial_2_Socket : public Print {
        static const int TXBUFFERSIZE = 1200;
        static const int RXBUFFERSIZE = 256;
        static const int FLUSHTIMEOUT = 500;

    public:
        Serial_2_Socket();

        size_t write(uint8_t c);
        size_t write(const uint8_t* buffer, size_t size);

        inline size_t write(const char* s) { return write((uint8_t*)s, ::strlen(s)); }
        inline size_t write(unsigned long n) { return write((uint8_t)n); }
        inline size_t write(long n) { return write((uint8_t)n); }
        inline size_t write(unsigned int n) { return write((uint8_t)n); }
        inline size_t write(int n) { return write((uint8_t)n); }

        long baudRate();
        void begin(long speed);
        void end();
        int  available();
        int  peek(void);
        int  read(void);
        bool push(const char* data);
        void flush(void);
        void handle_flush();
        bool attachWS(WebSocketsServer* web_socket);
        bool detachWS();

        operator bool() const;

        ~Serial_2_Socket();

    private:
        uint32_t          _lastflush;
        WebSocketsServer* _web_socket;

        uint8_t  _TXbuffer[TXBUFFERSIZE];
        uint16_t _TXbufferSize;

        uint8_t  _RXbuffer[RXBUFFERSIZE];
        uint16_t _RXbufferSize;
        uint16_t _RXbufferpos;
    };

    extern Serial_2_Socket Serial2Socket;
}
