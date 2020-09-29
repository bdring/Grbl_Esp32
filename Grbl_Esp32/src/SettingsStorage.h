#pragma once

/*
  SettingsStorage.h - eeprom configuration handling
  Part of Grbl

  Copyright (c) 2011-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

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

// Define status reporting boolean enable bit flags in status_report_mask
#define BITFLAG_RT_STATUS_POSITION_TYPE bit(0)
#define BITFLAG_RT_STATUS_BUFFER_STATE bit(1)

// Define settings restore bitflags.
#define SETTINGS_RESTORE_DEFAULTS bit(0)
#define SETTINGS_RESTORE_PARAMETERS bit(1)
#define SETTINGS_RESTORE_STARTUP_LINES bit(2)
#define SETTINGS_RESTORE_BUILD_INFO bit(3)
#define SETTINGS_RESTORE_WIFI_SETTINGS bit(4)
#ifndef SETTINGS_RESTORE_ALL
#    define SETTINGS_RESTORE_ALL 0xFF  // All bitflags
#endif

// Define EEPROM memory address location values for Grbl settings and parameters
// NOTE: The Atmega328p has 1KB EEPROM. The upper half is reserved for parameters and
// the startup script. The lower half contains the global settings and space for future
// developments.
const int EEPROM_SIZE            = 1024U;
const int EEPROM_ADDR_PARAMETERS = 512U;
const int EEPROM_ADDR_BUILD_INFO = 942U;

// Initialize the configuration subsystem (load settings from EEPROM)
void settings_init();
void settings_restore(uint8_t restore_flag);
void write_global_settings();

uint8_t settings_read_build_info(char* line);
void    settings_store_build_info(const char* line);

// Reads selected coordinate data from EEPROM
bool old_settings_read_coord_data(uint8_t coord_select, float* coord_data);

// Returns the step pin mask according to Grbl's internal axis numbering
uint8_t get_step_pin_mask(uint8_t i);

// Returns the direction pin mask according to Grbl's internal axis numbering
uint8_t get_direction_pin_mask(uint8_t i);

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

