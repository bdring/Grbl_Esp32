/*
  eeprom.h - Header for system level commands and real-time processes
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

#ifndef eeprom_memcpy_h
#define eeprom_memcpy_h

#include "grbl.h"

//unsigned char eeprom_get_char(unsigned int addr);
//void eeprom_put_char(unsigned int addr, unsigned char new_value);
void memcpy_to_eeprom_with_checksum(unsigned int destination, char *source, unsigned int size);
int memcpy_from_eeprom_with_checksum(char *destination, unsigned int source, unsigned int size);

#endif
