#pragma once

/*
  Eeprom.h - Header for system level commands and real-time processes
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

#include "Grbl.h"

// Define EEPROM memory address location values for saved coordinate data.
const int EEPROM_SIZE            = 1024U;
const int EEPROM_ADDR_PARAMETERS = 512U;

//unsigned char eeprom_get_char(unsigned int addr);
//void eeprom_put_char(unsigned int addr, unsigned char new_value);
void memcpy_to_eeprom_with_checksum(unsigned int destination, const char* source, unsigned int size);
int  memcpy_from_eeprom_with_checksum(char* destination, unsigned int source, unsigned int size);
int  memcpy_from_eeprom_with_old_checksum(char* destination, unsigned int source, unsigned int size);

// Reads selected coordinate data from EEPROM
bool old_settings_read_coord_data(uint8_t coord_select, float* coord_data);

// Various places in the code access saved coordinate system data
// by a small integer index according to the values below.
enum CoordIndex : uint8_t{
    Begin = 0,
    G54 = Begin,
    G55,
    G56,
    G57,
    G58,
    G59,
    // To support 9 work coordinate systems it would be necessary to define
    // the following 3 and modify GCode.cpp to support G59.1, G59.2, G59.3
    // G59_1,
    // G59_2,
    // G59_3,
    NWCSystems,
    G28 = NWCSystems,
    G30,
    // G92_2,
    // G92_3,
    End,
};
// Allow iteration over CoordIndex values
CoordIndex& operator ++ (CoordIndex& i);
