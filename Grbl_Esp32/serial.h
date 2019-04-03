/*
  serial.h - Header for system level commands and real-time processes
  Part of Grbl
  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC
	
	2018 -	Bart Dring This file was modifed for use on the ESP32
					CPU. Do not use this with Grbl for atMega328P
	
  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef serial_h
#define serial_h

#include "grbl.h"

#ifndef RX_BUFFER_SIZE
  #define RX_BUFFER_SIZE 128
#endif
#ifndef TX_BUFFER_SIZE
  #ifdef USE_LINE_NUMBERS
    #define TX_BUFFER_SIZE 112
  #else
    #define TX_BUFFER_SIZE 104
  #endif
#endif

#define SERIAL_NO_DATA 0xff

// a task to read for incoming data from serial port
void serialCheckTask(void *pvParameters);

void serialCheck();

void serial_write(uint8_t data);
// Fetches the first byte in the serial read buffer. Called by main program.
uint8_t serial_read(uint8_t client);

// See if the character is an action command like feedhold or jogging. If so, do the action and return true
uint8_t check_action_command(uint8_t data);

void serial_init();
void serial_reset_read_buffer(uint8_t client);

// Returns the number of bytes available in the RX serial buffer.
uint8_t serial_get_rx_buffer_available(uint8_t client);

#endif
