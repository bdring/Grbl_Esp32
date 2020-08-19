#pragma once

/*
  MacroBuffer.h -  for injecting macro data as an input stream

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

#include <Stream.h>
#include <cstring>

// MacroBuffer inherits from Stream so it can be used as the input source for
// a "Client".  Clients support input via read() and output via print().

class MacroBuffer : public Stream {
public:
    MacroBuffer();

    void begin();

    // push() injects macro data into the stream
    bool push(const char* data);

    // Virtual method of Print class; discards the data
    size_t write(uint8_t c) { return 0; }

    // Virtual methods of Stream class
    int  available() { return _bufferSize; }
    int  peek(void) { return _bufferSize ? _buffer[_bufferpos] : -1; }
    int  read(void);
    void flush(void) {}

    operator bool() const { return true; }

private:
    static const int BUFFERSIZE = 128;

    uint8_t  _buffer[BUFFERSIZE];
    uint16_t _bufferSize;
    uint16_t _bufferpos;
};

extern MacroBuffer macroBuffer;
