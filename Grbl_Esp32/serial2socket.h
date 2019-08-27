/*
  serial2socket.h -  serial 2 socket functions class

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


#ifndef _SERIAL_2_SOCKET_H_
#define _SERIAL_2_SOCKET_H_

#include "Print.h"
#define TXBUFFERSIZE 1200
#define RXBUFFERSIZE 128
#define FLUSHTIMEOUT 500
class Serial_2_Socket: public Print{
    public:
    Serial_2_Socket();
    ~Serial_2_Socket();
    size_t write(uint8_t c);
    size_t write(const uint8_t *buffer, size_t size);

    inline size_t write(const char * s)
    {
        return write((uint8_t*) s, strlen(s));
    }
    inline size_t write(unsigned long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(unsigned int n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(int n)
    {
        return write((uint8_t) n);
    }
    long baudRate();
    void begin(long speed);
    void end();
    int available();
    int peek(void);
    int read(void);
    bool push (const char * data);
    void flush(void);
    void handle_flush();
    operator bool() const;
    bool attachWS(void * web_socket);
    bool detachWS();
    private:
    uint32_t _lastflush;
    void * _web_socket;
    uint8_t _TXbuffer[TXBUFFERSIZE];
    uint16_t _TXbufferSize;
    uint8_t _RXbuffer[RXBUFFERSIZE];
    uint16_t _RXbufferSize;
    uint16_t _RXbufferpos;
};


extern Serial_2_Socket Serial2Socket;

#endif
