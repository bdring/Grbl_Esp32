/*
  TelnetServer.cpp -  telnet server functions class

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

#include "../Grbl.h"

#if defined(ENABLE_WIFI) && defined(ENABLE_TELNET)

#    include "WifiServices.h"

#    include "TelnetServer.h"
#    include "WifiConfig.h"
#    include <WiFi.h>

namespace WebUI {
    Telnet_Server telnet_server;
    bool          Telnet_Server::_setupdone    = false;
    uint16_t      Telnet_Server::_port         = 0;
    WiFiServer*   Telnet_Server::_telnetserver = NULL;
    WiFiClient    Telnet_Server::_telnetClients[MAX_TLNT_CLIENTS];

#    ifdef ENABLE_TELNET_WELCOME_MSG
    IPAddress Telnet_Server::_telnetClientsIP[MAX_TLNT_CLIENTS];
#    endif

    Telnet_Server::Telnet_Server() {
        _RXbufferSize = 0;
        _RXbufferpos  = 0;
    }

    bool Telnet_Server::begin() {
        bool no_error = true;
        end();
        _RXbufferSize = 0;
        _RXbufferpos  = 0;

        if (telnet_enable->get() == 0) {
            return false;
        }
        _port = telnet_port->get();

        //create instance
        _telnetserver = new WiFiServer(_port, MAX_TLNT_CLIENTS);
        _telnetserver->setNoDelay(true);
        String s = "[MSG:TELNET Started " + String(_port) + "]\r\n";
        grbl_send(CLIENT_ALL, (char*)s.c_str());
        //start telnet server
        _telnetserver->begin();
        _setupdone = true;
        return no_error;
    }

    void Telnet_Server::end() {
        _setupdone    = false;
        _RXbufferSize = 0;
        _RXbufferpos  = 0;
        if (_telnetserver) {
            delete _telnetserver;
            _telnetserver = NULL;
        }
    }

    void Telnet_Server::clearClients() {
        //check if there are any new clients
        if (_telnetserver->hasClient()) {
            uint8_t i;
            for (i = 0; i < MAX_TLNT_CLIENTS; i++) {
                //find free/disconnected spot
                if (!_telnetClients[i] || !_telnetClients[i].connected()) {
#    ifdef ENABLE_TELNET_WELCOME_MSG
                    _telnetClientsIP[i] = IPAddress(0, 0, 0, 0);
#    endif
                    if (_telnetClients[i]) {
                        _telnetClients[i].stop();
                    }
                    _telnetClients[i] = _telnetserver->available();
                    break;
                }
            }
            if (i >= MAX_TLNT_CLIENTS) {
                //no free/disconnected spot so reject
                _telnetserver->available().stop();
            }
        }
    }

    size_t Telnet_Server::write(const uint8_t* buffer, size_t size) {
        size_t wsize = 0;
        if (!_setupdone || _telnetserver == NULL) {
            log_d("[TELNET out blocked]");
            return 0;
        }

        clearClients();

        //log_d("[TELNET out]");
        //push UART data to all connected telnet clients
        for (uint8_t i = 0; i < MAX_TLNT_CLIENTS; i++) {
            if (_telnetClients[i] && _telnetClients[i].connected()) {
                //log_d("[TELNET out connected]");
                wsize = _telnetClients[i].write(buffer, size);
                COMMANDS::wait(0);
            }
        }
        return wsize;
    }

    void Telnet_Server::handle() {
        COMMANDS::wait(0);
        //check if can read
        if (!_setupdone || _telnetserver == NULL) {
            return;
        }
        clearClients();
        //check clients for data
        //uint8_t c;
        for (uint8_t i = 0; i < MAX_TLNT_CLIENTS; i++) {
            if (_telnetClients[i] && _telnetClients[i].connected()) {
#    ifdef ENABLE_TELNET_WELCOME_MSG
                if (_telnetClientsIP[i] != _telnetClients[i].remoteIP()) {
                    report_init_message(CLIENT_TELNET);
                    _telnetClientsIP[i] = _telnetClients[i].remoteIP();
                }
#    endif
                if (_telnetClients[i].available()) {
                    uint8_t buf[1024];
                    COMMANDS::wait(0);
                    int readlen  = _telnetClients[i].available();
                    int writelen = TELNETRXBUFFERSIZE - available();
                    if (readlen > 1024) {
                        readlen = 1024;
                    }
                    if (readlen > writelen) {
                        readlen = writelen;
                    }
                    if (readlen > 0) {
                        _telnetClients[i].read(buf, readlen);
                        push(buf, readlen);
                    }
                    return;
                }
            } else {
                if (_telnetClients[i]) {
#    ifdef ENABLE_TELNET_WELCOME_MSG
                    _telnetClientsIP[i] = IPAddress(0, 0, 0, 0);
#    endif
                    _telnetClients[i].stop();
                }
            }
            COMMANDS::wait(0);
        }
    }

    int Telnet_Server::peek(void) {
        if (_RXbufferSize > 0) {
            return _RXbuffer[_RXbufferpos];
        } else {
            return -1;
        }
    }

    int Telnet_Server::available() { return _RXbufferSize; }

    int Telnet_Server::get_rx_buffer_available() { return TELNETRXBUFFERSIZE - _RXbufferSize; }

    bool Telnet_Server::push(uint8_t data) {
        log_i("[TELNET]push %c", data);
        if ((1 + _RXbufferSize) <= TELNETRXBUFFERSIZE) {
            int current = _RXbufferpos + _RXbufferSize;
            if (current > TELNETRXBUFFERSIZE) {
                current = current - TELNETRXBUFFERSIZE;
            }
            if (current > (TELNETRXBUFFERSIZE - 1)) {
                current = 0;
            }
            _RXbuffer[current] = data;
            _RXbufferSize++;
            log_i("[TELNET]buffer size %d", _RXbufferSize);
            return true;
        }
        return false;
    }

    bool Telnet_Server::push(const uint8_t* data, int data_size) {
        if ((data_size + _RXbufferSize) <= TELNETRXBUFFERSIZE) {
            int data_processed = 0;
            int current        = _RXbufferpos + _RXbufferSize;
            if (current > TELNETRXBUFFERSIZE) {
                current = current - TELNETRXBUFFERSIZE;
            }
            for (int i = 0; i < data_size; i++) {
                if (current > (TELNETRXBUFFERSIZE - 1)) {
                    current = 0;
                }

                _RXbuffer[current] = data[i];
                current++;
                data_processed++;

                COMMANDS::wait(0);
                //vTaskDelay(1 / portTICK_RATE_MS);  // Yield to other tasks
            }
            _RXbufferSize += data_processed;
            return true;
        }
        return false;
    }

    int Telnet_Server::read(void) {
        if (_RXbufferSize > 0) {
            int v = _RXbuffer[_RXbufferpos];
            //log_d("[TELNET]read %c",char(v));
            _RXbufferpos++;
            if (_RXbufferpos > (TELNETRXBUFFERSIZE - 1)) {
                _RXbufferpos = 0;
            }
            _RXbufferSize--;
            return v;
        } else {
            return -1;
        }
    }

    Telnet_Server::~Telnet_Server() { end(); }
}
#endif  // Enable TELNET && ENABLE_WIFI
