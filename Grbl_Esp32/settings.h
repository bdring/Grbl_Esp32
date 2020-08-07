#pragma once

/*
  settings.h - eeprom configuration handling
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

#include "grbl.h"

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
#define EEPROM_SIZE 1024U
#define EEPROM_ADDR_PARAMETERS 512U
#define EEPROM_ADDR_BUILD_INFO 942U

// Define EEPROM address indexing for coordinate parameters
#define N_COORDINATE_SYSTEM 6                         // Number of supported work coordinate systems (from index 1)
#define SETTING_INDEX_NCOORD N_COORDINATE_SYSTEM + 1  // Total number of system stored (from index 0)
// NOTE: Work coordinate indices are (0=G54, 1=G55, ... , 6=G59)
#define SETTING_INDEX_G28 N_COORDINATE_SYSTEM      // Home position 1
#define SETTING_INDEX_G30 N_COORDINATE_SYSTEM + 1  // Home position 2
// #define SETTING_INDEX_G92    N_COORDINATE_SYSTEM+2  // Coordinate offset (G92.2,G92.3 not supported)

#define USER_SETTING_COUNT 5  // for user to define for their machine

// Initialize the configuration subsystem (load settings from EEPROM)
void settings_init();
void settings_restore(uint8_t restore_flag);
void write_global_settings();

uint8_t settings_read_build_info(char* line);
void    settings_store_build_info(const char* line);

// Writes selected coordinate data to EEPROM
void settings_write_coord_data(uint8_t coord_select, float* coord_data);

// Reads selected coordinate data from EEPROM
uint8_t settings_read_coord_data(uint8_t coord_select, float* coord_data);

// Returns the step pin mask according to Grbl's internal axis numbering
uint8_t get_step_pin_mask(uint8_t i);

// Returns the direction pin mask according to Grbl's internal axis numbering
uint8_t get_direction_pin_mask(uint8_t i);
