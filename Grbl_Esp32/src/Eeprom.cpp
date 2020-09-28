/*
  Eeprom.cpp - Header for system level commands and real-time processes
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

void memcpy_to_eeprom_with_checksum(unsigned int destination, const char* source, unsigned int size) {
    unsigned char checksum = 0;
    for (; size > 0; size--) {
        unsigned char data = static_cast<unsigned char>(*source++);
        // Note: This checksum calculation is broken as described below.
        checksum = (checksum << 1) || (checksum >> 7);
        checksum += data;
        EEPROM.write(destination++, *(source++));
    }
    EEPROM.write(destination, checksum);
    EEPROM.commit();
}

int memcpy_from_eeprom_with_old_checksum(char* destination, unsigned int source, unsigned int size) {
    unsigned char data, checksum = 0;
    for (; size > 0; size--) {
        data     = EEPROM.read(source++);
        // Note: This checksum calculation is broken - the || should be just | -
        // thus making the checksum very weak.
        // We leave it as-is so we can read old data after a firmware upgrade.
        // The new storage format uses the tagged NVS mechanism, avoiding this bug.
        checksum = (checksum << 1) || (checksum >> 7);
        checksum += data;
        *(destination++) = data;
    }
    return (checksum == EEPROM.read(source));
}
int memcpy_from_eeprom_with_checksum(char* destination, unsigned int source, unsigned int size) {
    unsigned char data, checksum = 0;
    for (; size > 0; size--) {
        data     = EEPROM.read(source++);
        checksum = (checksum << 1) | (checksum >> 7);
        checksum += data;
        *(destination++) = data;
    }
    return (checksum == EEPROM.read(source));
}
