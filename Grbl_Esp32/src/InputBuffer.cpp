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

#include "InputBuffer.h"

InputBuffer::InputBuffer() {
    begin();
}

void InputBuffer::begin() {
    _bufferSize = 0;
    _bufferpos  = 0;
}

void InputBuffer::push(uint8_t c) {
    if (_bufferSize == BUFFERSIZE) {
        return;
    }
    int writeIndex = _bufferpos + _bufferSize;
    if (writeIndex >= BUFFERSIZE) {
        writeIndex -= BUFFERSIZE;
    }
    _buffer[writeIndex] = c;
    _bufferSize++;
}

int InputBuffer::read(void) {
    if (!_bufferSize) {
        return -1;
    }
    int data = _buffer[_bufferpos];
    if (++_bufferpos == BUFFERSIZE) {
        _bufferpos = 0;
    }
    _bufferSize--;
    return data;
}
