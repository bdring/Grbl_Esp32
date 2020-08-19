#pragma once

/*
  InputBuffer.h -  circular queue for transferring input data from SerialCheckTask
  to protocol_main_loop().  There is an instance of this for each client.

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

#include <stdint.h>
#include <cstring>

class InputBuffer {
public:
    InputBuffer();

    void          begin();
    int           available()  { return _bufferSize; }
    int           availableForPush()  { return BUFFERSIZE - _bufferSize; }
    int           read(void);
    void          push(uint8_t c);

private:
    static const int BUFFERSIZE = 128;

    uint8_t  _buffer[BUFFERSIZE];
    uint16_t _bufferSize;
    uint16_t _bufferpos;
};
