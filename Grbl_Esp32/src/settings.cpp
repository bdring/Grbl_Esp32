/*
  settings.c - eeprom configuration handling
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

// Read selected coordinate data from EEPROM. Updates pointed coord_data value.
uint8_t settings_read_coord_data(uint8_t coord_select, float* coord_data) {
    uint32_t addr = coord_select * (sizeof(float) * N_AXIS + 1) + EEPROM_ADDR_PARAMETERS;
    if (!(memcpy_from_eeprom_with_checksum((char*)coord_data, addr, sizeof(float) * N_AXIS))) {
        // Reset with default zero vector
        clear_vector_float(coord_data);
        settings_write_coord_data(coord_select, coord_data);
        return (false);
    }
    return (true);
}

// Method to store coord data parameters into EEPROM
void settings_write_coord_data(uint8_t coord_select, float* coord_data) {
#ifdef FORCE_BUFFER_SYNC_DURING_EEPROM_WRITE
    protocol_buffer_synchronize();
#endif
    uint32_t addr = coord_select * (sizeof(float) * N_AXIS + 1) + EEPROM_ADDR_PARAMETERS;
    memcpy_to_eeprom_with_checksum(addr, (char*)coord_data, sizeof(float) * N_AXIS);
}

// Method to store build info into EEPROM
// NOTE: This function can only be called in IDLE state.
void settings_store_build_info(const char* line) {
    // Build info can only be stored when state is IDLE.
    memcpy_to_eeprom_with_checksum(EEPROM_ADDR_BUILD_INFO, (char*)line, LINE_BUFFER_SIZE);
}

// Reads startup line from EEPROM. Updated pointed line string data.
uint8_t settings_read_build_info(char* line) {
    if (!(memcpy_from_eeprom_with_checksum((char*)line, EEPROM_ADDR_BUILD_INFO, LINE_BUFFER_SIZE))) {
        // Reset line with default value
        line[0] = 0;  // Empty line
        settings_store_build_info(line);
        return (false);
    }
    return (true);
}

// Returns step pin mask according to Grbl internal axis indexing.
uint8_t get_step_pin_mask(uint8_t axis_idx) {
    // todo clean this up further up stream
    return bit(axis_idx);
}

// Returns direction pin mask according to Grbl internal axis indexing.
uint8_t get_direction_pin_mask(uint8_t axis_idx) {
    return bit(axis_idx);
}
