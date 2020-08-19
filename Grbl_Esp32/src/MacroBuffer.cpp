/*
  InputBuffer.cpp -  inputbuffer functions class

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

// #include "Config.h"
#include "MacroBuffer.h"

MacroBuffer macroBuffer;

MacroBuffer::MacroBuffer() {
    begin();
}

void MacroBuffer::begin() {
    _bufferSize = 0;
    _bufferpos  = 0;
}

bool MacroBuffer::push(const char* data) {
    int data_size = strlen(data);
    if ((data_size + _bufferSize) <= BUFFERSIZE) {
        int current = _bufferpos + _bufferSize;
        for (int i = 0; i < data_size; i++) {
            if (current >= BUFFERSIZE) {
                current -= BUFFERSIZE;
            }
            _buffer[current++] = data[i];
        }
        _bufferSize += strlen(data);
        return true;
    }
    return false;
}

int MacroBuffer::read(void) {
    if (!_bufferSize) {
        return -1;
    }
    int v = _buffer[_bufferpos];
    if (++_bufferpos > (BUFFERSIZE - 1)) {
        _bufferpos = 0;
    }
    _bufferSize--;
    return v;
}
