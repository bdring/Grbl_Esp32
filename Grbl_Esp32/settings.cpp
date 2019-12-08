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

settings_t settings;

// Method to store startup lines into EEPROM
void settings_store_startup_line(uint8_t n, char *line)
{
  #ifdef FORCE_BUFFER_SYNC_DURING_EEPROM_WRITE
    protocol_buffer_synchronize(); // A startup line may contain a motion and be executing. 
  #endif
  uint32_t addr = n*(LINE_BUFFER_SIZE+1)+EEPROM_ADDR_STARTUP_BLOCK;
  memcpy_to_eeprom_with_checksum(addr,(char*)line, LINE_BUFFER_SIZE);
}

void settings_init()
{
	EEPROM.begin(EEPROM_SIZE);
  
  if(!read_global_settings()) {
    report_status_message(STATUS_SETTING_READ_FAIL, CLIENT_SERIAL);
    settings_restore(SETTINGS_RESTORE_ALL); // Force restore all EEPROM data.
    report_grbl_settings(CLIENT_SERIAL); // only the serial could be working at this point
  }
}

// Method to restore EEPROM-saved Grbl global settings back to defaults.
void settings_restore(uint8_t restore_flag) {
#if defined(ENABLE_BLUETOOTH) || defined(ENABLE_WIFI)
  if (restore_flag & SETTINGS_RESTORE_WIFI_SETTINGS){
#ifdef ENABLE_WIFI
      wifi_config.reset_settings();
#endif
#ifdef ENABLE_BLUETOOTH
      bt_config.reset_settings();
#endif
  }
#endif
  if (restore_flag & SETTINGS_RESTORE_DEFAULTS) {
    settings.pulse_microseconds = DEFAULT_STEP_PULSE_MICROSECONDS;
    settings.stepper_idle_lock_time = DEFAULT_STEPPER_IDLE_LOCK_TIME;
    settings.step_invert_mask = DEFAULT_STEPPING_INVERT_MASK;
    settings.dir_invert_mask = DEFAULT_DIRECTION_INVERT_MASK;
    settings.status_report_mask = DEFAULT_STATUS_REPORT_MASK;
    settings.junction_deviation = DEFAULT_JUNCTION_DEVIATION;

    settings.arc_tolerance = DEFAULT_ARC_TOLERANCE;
	
	settings.spindle_pwm_freq = DEFAULT_SPINDLE_FREQ;      // $33 Hz (extended set)
	settings.spindle_pwm_off_value = DEFAULT_SPINDLE_OFF_VALUE; // $34 Percent (extended set)
	settings.spindle_pwm_min_value = DEFAULT_SPINDLE_MIN_VALUE; // $35 Percent (extended set)
	settings.spindle_pwm_max_value = DEFAULT_SPINDLE_MAX_VALUE; // $36 Percent (extended set)
	
    settings.rpm_max = DEFAULT_SPINDLE_RPM_MAX;
    settings.rpm_min = DEFAULT_SPINDLE_RPM_MIN;

    settings.homing_dir_mask = DEFAULT_HOMING_DIR_MASK;
    settings.homing_feed_rate = DEFAULT_HOMING_FEED_RATE;
    settings.homing_seek_rate = DEFAULT_HOMING_SEEK_RATE;
    settings.homing_debounce_delay = DEFAULT_HOMING_DEBOUNCE_DELAY;
    settings.homing_pulloff = DEFAULT_HOMING_PULLOFF;

    settings.flags = 0;
    if (DEFAULT_REPORT_INCHES) { settings.flags |= BITFLAG_REPORT_INCHES; }
    if (DEFAULT_LASER_MODE) { settings.flags |= BITFLAG_LASER_MODE; }
    if (DEFAULT_INVERT_ST_ENABLE) { settings.flags |= BITFLAG_INVERT_ST_ENABLE; }
    if (DEFAULT_HARD_LIMIT_ENABLE) { settings.flags |= BITFLAG_HARD_LIMIT_ENABLE; }
    if (DEFAULT_HOMING_ENABLE) { settings.flags |= BITFLAG_HOMING_ENABLE; }
    if (DEFAULT_SOFT_LIMIT_ENABLE) { settings.flags |= BITFLAG_SOFT_LIMIT_ENABLE; }
    if (DEFAULT_INVERT_LIMIT_PINS) { settings.flags |= BITFLAG_INVERT_LIMIT_PINS; }
    if (DEFAULT_INVERT_PROBE_PIN) { settings.flags |= BITFLAG_INVERT_PROBE_PIN; }

    settings.steps_per_mm[X_AXIS] = DEFAULT_X_STEPS_PER_MM;
    settings.steps_per_mm[Y_AXIS] = DEFAULT_Y_STEPS_PER_MM;
    settings.steps_per_mm[Z_AXIS] = DEFAULT_Z_STEPS_PER_MM;
	
    settings.max_rate[X_AXIS] = DEFAULT_X_MAX_RATE;
    settings.max_rate[Y_AXIS] = DEFAULT_Y_MAX_RATE;
    settings.max_rate[Z_AXIS] = DEFAULT_Z_MAX_RATE;
	
    settings.acceleration[X_AXIS] = DEFAULT_X_ACCELERATION;
    settings.acceleration[Y_AXIS] = DEFAULT_Y_ACCELERATION;
    settings.acceleration[Z_AXIS] = DEFAULT_Z_ACCELERATION;
	
    settings.max_travel[X_AXIS] = (-DEFAULT_X_MAX_TRAVEL);
    settings.max_travel[Y_AXIS] = (-DEFAULT_Y_MAX_TRAVEL);
    settings.max_travel[Z_AXIS] = (-DEFAULT_Z_MAX_TRAVEL);
	
	settings.current[X_AXIS] = DEFAULT_X_CURRENT;
	settings.current[Y_AXIS] = DEFAULT_Y_CURRENT;
	settings.current[Z_AXIS] = DEFAULT_Z_CURRENT;
	
	settings.hold_current[X_AXIS] = DEFAULT_X_HOLD_CURRENT;
	settings.hold_current[Y_AXIS] = DEFAULT_Y_HOLD_CURRENT;
	settings.hold_current[Z_AXIS] = DEFAULT_Z_HOLD_CURRENT;
	
	settings.microsteps[X_AXIS] = DEFAULT_X_MICROSTEPS;
	settings.microsteps[Y_AXIS] = DEFAULT_Y_MICROSTEPS;
	settings.microsteps[Z_AXIS] = DEFAULT_Z_MICROSTEPS;
	
  settings.stallguard[X_AXIS] = DEFAULT_X_STALLGUARD;
	settings.stallguard[Y_AXIS] = DEFAULT_Y_STALLGUARD;
	settings.stallguard[Z_AXIS] = DEFAULT_Z_STALLGUARD;
	
	#if (N_AXIS > A_AXIS)
		 settings.steps_per_mm[A_AXIS] = DEFAULT_A_STEPS_PER_MM;
		 settings.max_rate[A_AXIS] = DEFAULT_A_MAX_RATE;
		 settings.acceleration[A_AXIS] = DEFAULT_A_ACCELERATION;
		 settings.max_travel[A_AXIS] = (-DEFAULT_A_MAX_TRAVEL);
		 settings.current[A_AXIS] = DEFAULT_A_CURRENT;
		 settings.hold_current[A_AXIS] = DEFAULT_A_HOLD_CURRENT;
		 settings.microsteps[A_AXIS] = DEFAULT_A_MICROSTEPS;
     settings.stallguard[A_AXIS] = DEFAULT_Z_STALLGUARD;
	#endif
	
	#if (N_AXIS > B_AXIS)
		 settings.steps_per_mm[B_AXIS] = DEFAULT_B_STEPS_PER_MM;
		 settings.max_rate[B_AXIS] = DEFAULT_B_MAX_RATE;
		 settings.acceleration[B_AXIS] = DEFAULT_B_ACCELERATION;
		 settings.max_travel[B_AXIS] = (-DEFAULT_B_MAX_TRAVEL);
		 settings.current[B_AXIS] = DEFAULT_B_CURRENT;
		 settings.hold_current[B_AXIS] = DEFAULT_B_HOLD_CURRENT;
		 settings.microsteps[B_AXIS] = DEFAULT_B_MICROSTEPS;
     settings.stallguard[B_AXIS] = DEFAULT_Z_STALLGUARD;
	#endif
	
	#if (N_AXIS > C_AXIS)
		 settings.steps_per_mm[C_AXIS] = DEFAULT_C_STEPS_PER_MM;
		 settings.max_rate[C_AXIS] = DEFAULT_C_MAX_RATE;
		 settings.acceleration[C_AXIS] = DEFAULT_C_ACCELERATION;
		 settings.max_travel[C_AXIS] = (-DEFAULT_C_MAX_TRAVEL);
		 settings.current[C_AXIS] = DEFAULT_C_CURRENT;
		 settings.hold_current[C_AXIS] = DEFAULT_C_HOLD_CURRENT;
		 settings.microsteps[C_AXIS] = DEFAULT_C_MICROSTEPS;
     settings.stallguard[C_AXIS] = DEFAULT_Z_STALLGUARD;
	#endif
	
    // TODO figure out a clean way to add actual default values
    for (uint8_t index = 0; index<USER_SETTING_COUNT; index++) {
      settings.machine_int16[index] = 0;
      settings.machine_float[index] = 0.0;
    }
    

    write_global_settings();
  }

  if (restore_flag & SETTINGS_RESTORE_PARAMETERS) {
    uint8_t idx;
    float coord_data[N_AXIS];
    memset(&coord_data, 0, sizeof(coord_data));
    for (idx=0; idx <= SETTING_INDEX_NCOORD; idx++) { settings_write_coord_data(idx, coord_data); }
  }

  if (restore_flag & SETTINGS_RESTORE_STARTUP_LINES) {
    #if N_STARTUP_LINE > 0
      EEPROM.write(EEPROM_ADDR_STARTUP_BLOCK, 0);
      EEPROM.write(EEPROM_ADDR_STARTUP_BLOCK+1, 0); // Checksum
      EEPROM.commit();
    #endif
    #if N_STARTUP_LINE > 1
      EEPROM.write(EEPROM_ADDR_STARTUP_BLOCK+(LINE_BUFFER_SIZE+1), 0);
      EEPROM.write(EEPROM_ADDR_STARTUP_BLOCK+(LINE_BUFFER_SIZE+2), 0); // Checksum
      EEPROM.commit();
    #endif
  }

  if (restore_flag & SETTINGS_RESTORE_BUILD_INFO) {
    EEPROM.write(EEPROM_ADDR_BUILD_INFO , 0);
    EEPROM.write(EEPROM_ADDR_BUILD_INFO+1 , 0); // Checksum
    EEPROM.commit();
  }

  
}

// Reads Grbl global settings struct from EEPROM.
uint8_t read_global_settings() {
  // Check version-byte of eeprom
  uint8_t version = EEPROM.read(0);
  if (version == SETTINGS_VERSION) {
    // Read settings-record and check checksum
    if (!(memcpy_from_eeprom_with_checksum((char*)&settings, EEPROM_ADDR_GLOBAL, sizeof(settings_t)))) {
      return(false);
    }
  } else {
    return(false);
  }
  return(true);
}

// Method to store Grbl global settings struct and version number into EEPROM
// NOTE: This function can only be called in IDLE state.
void write_global_settings()
{
  EEPROM.write(0, SETTINGS_VERSION);
  memcpy_to_eeprom_with_checksum(EEPROM_ADDR_GLOBAL, (char*)&settings, sizeof(settings_t));
  
}

// Read selected coordinate data from EEPROM. Updates pointed coord_data value.
uint8_t settings_read_coord_data(uint8_t coord_select, float *coord_data)
{
  uint32_t addr = coord_select*(sizeof(float)*N_AXIS+1) + EEPROM_ADDR_PARAMETERS;
  if (!(memcpy_from_eeprom_with_checksum((char*)coord_data, addr, sizeof(float)*N_AXIS))) {
    // Reset with default zero vector
    clear_vector_float(coord_data);
    settings_write_coord_data(coord_select,coord_data);
    return(false);
  }
  return(true);
}

// Method to store coord data parameters into EEPROM
void settings_write_coord_data(uint8_t coord_select, float *coord_data)
{
  #ifdef FORCE_BUFFER_SYNC_DURING_EEPROM_WRITE
    protocol_buffer_synchronize();
  #endif
  uint32_t addr = coord_select*(sizeof(float)*N_AXIS+1) + EEPROM_ADDR_PARAMETERS; 
  memcpy_to_eeprom_with_checksum(addr,(char*)coord_data, sizeof(float)*N_AXIS);
}

// Method to store build info into EEPROM
// NOTE: This function can only be called in IDLE state.
void settings_store_build_info(char *line)
{
  // Build info can only be stored when state is IDLE.
  memcpy_to_eeprom_with_checksum(EEPROM_ADDR_BUILD_INFO,(char*)line, LINE_BUFFER_SIZE);
}

// Reads startup line from EEPROM. Updated pointed line string data.
uint8_t settings_read_build_info(char *line)
{
  if (!(memcpy_from_eeprom_with_checksum((char*)line, EEPROM_ADDR_BUILD_INFO, LINE_BUFFER_SIZE))) {
    // Reset line with default value
    line[0] = 0; // Empty line
    settings_store_build_info(line);
    return(false);
  }
  return(true);
}



// Reads startup line from EEPROM. Updated pointed line string data.
uint8_t settings_read_startup_line(uint8_t n, char *line)
{
  uint32_t addr = n*(LINE_BUFFER_SIZE+1)+EEPROM_ADDR_STARTUP_BLOCK;
  if (!(memcpy_from_eeprom_with_checksum((char*)line, addr, LINE_BUFFER_SIZE))) {
    // Reset line with default value
    line[0] = 0; // Empty line
    settings_store_startup_line(n, line);
    return(false);
  }
  return(true);
}

// A helper method to set settings from command line
uint8_t settings_store_global_setting(uint8_t parameter, float value) {
  if (value < 0.0) { return(STATUS_NEGATIVE_VALUE); }
  uint8_t int_value = trunc(value); // integer version
  if (parameter >= AXIS_SETTINGS_START_VAL) {
    // Store axis configuration. Axis numbering sequence set by AXIS_SETTING defines.
    // NOTE: Ensure the setting index corresponds to the report.c settings printout.
    parameter -= AXIS_SETTINGS_START_VAL;
    uint8_t set_idx = 0;
    while (set_idx < AXIS_N_SETTINGS) {
      if (parameter < N_AXIS) {
        // Valid axis setting found.
        switch (set_idx) {
          case 0:
            #ifdef MAX_STEP_RATE_HZ
              if (value*settings.max_rate[parameter] > (MAX_STEP_RATE_HZ*60.0)) { return(STATUS_MAX_STEP_RATE_EXCEEDED); }
            #endif
            settings.steps_per_mm[parameter] = value;
            break;
          case 1:
            #ifdef MAX_STEP_RATE_HZ
              if (value*settings.steps_per_mm[parameter] > (MAX_STEP_RATE_HZ*60.0)) {  return(STATUS_MAX_STEP_RATE_EXCEEDED); }
            #endif
            settings.max_rate[parameter] = value;
            break;
          case 2: settings.acceleration[parameter] = value*60*60; break; // Convert to mm/min^2 for grbl internal use.
          case 3: settings.max_travel[parameter] = -value; break;  // Store as negative for grbl internal use.
		  case 4: // run current
				settings.current[parameter] = value;
				settings_spi_driver_init();
			break;
		  case 5: // hold current
				settings.hold_current[parameter] = value;
				settings_spi_driver_init();
		  break;
		  case 6: // microstepping
				settings.microsteps[parameter] = int_value;
				settings_spi_driver_init();
      break;
      case 7: // stallguard
        settings.stallguard[parameter] = int_value;
				settings_spi_driver_init();
		  break;
        }
        break; // Exit while-loop after setting has been configured and proceed to the EEPROM write call.
      } else {
        set_idx++;
        // If axis index greater than N_AXIS or setting index greater than number of axis settings, error out.
        if ((parameter < AXIS_SETTINGS_INCREMENT) || (set_idx == AXIS_N_SETTINGS)) { return(STATUS_INVALID_STATEMENT); }
        parameter -= AXIS_SETTINGS_INCREMENT;
      }
    }
  } else {
    // Store non-axis Grbl settings
    
    switch(parameter) {
      case 0:
        if (int_value < 3) { return(STATUS_SETTING_STEP_PULSE_MIN); }
        settings.pulse_microseconds = int_value; break;
      case 1: settings.stepper_idle_lock_time = int_value; break;
      case 2:
        settings.step_invert_mask = int_value;
        st_generate_step_dir_invert_masks(); // Regenerate step and direction port invert masks.
        break;
      case 3:
        settings.dir_invert_mask = int_value;
        st_generate_step_dir_invert_masks(); // Regenerate step and direction port invert masks.
        break;
      case 4: // Reset to ensure change. Immediate re-init may cause problems.
        if (int_value) { settings.flags |= BITFLAG_INVERT_ST_ENABLE; }
        else { settings.flags &= ~BITFLAG_INVERT_ST_ENABLE; }
        break;
      case 5: // Reset to ensure change. Immediate re-init may cause problems.
        if (int_value) { settings.flags |= BITFLAG_INVERT_LIMIT_PINS; }
        else { settings.flags &= ~BITFLAG_INVERT_LIMIT_PINS; }
        break;
      case 6: // Reset to ensure change. Immediate re-init may cause problems.
        if (int_value) { settings.flags |= BITFLAG_INVERT_PROBE_PIN; }
        else { settings.flags &= ~BITFLAG_INVERT_PROBE_PIN; }
        probe_configure_invert_mask(false);
        break;
      case 10: settings.status_report_mask = int_value; break;
      case 11: settings.junction_deviation = value; break;
      case 12: settings.arc_tolerance = value; break;
      case 13:
        if (int_value) { settings.flags |= BITFLAG_REPORT_INCHES; }
        else { settings.flags &= ~BITFLAG_REPORT_INCHES; }
        system_flag_wco_change(); // Make sure WCO is immediately updated.
        break;
      case 20:
        if (int_value) {
          if (bit_isfalse(settings.flags, BITFLAG_HOMING_ENABLE)) { return(STATUS_SOFT_LIMIT_ERROR); }
          settings.flags |= BITFLAG_SOFT_LIMIT_ENABLE;
        } else { settings.flags &= ~BITFLAG_SOFT_LIMIT_ENABLE; }
        break;
      case 21:
        if (int_value) { settings.flags |= BITFLAG_HARD_LIMIT_ENABLE; }
        else { settings.flags &= ~BITFLAG_HARD_LIMIT_ENABLE; }
        limits_init(); // Re-init to immediately change. NOTE: Nice to have but could be problematic later.
        break;
      case 22:
        if (int_value) { settings.flags |= BITFLAG_HOMING_ENABLE; }
        else {
          settings.flags &= ~BITFLAG_HOMING_ENABLE;
          settings.flags &= ~BITFLAG_SOFT_LIMIT_ENABLE; // Force disable soft-limits.
        }
        break;
      case 23: settings.homing_dir_mask = int_value; break;
      case 24: settings.homing_feed_rate = value; break;
      case 25: settings.homing_seek_rate = value; break;
      case 26: settings.homing_debounce_delay = int_value; break;
      case 27: settings.homing_pulloff = value; break;
      case 30: settings.rpm_max = value; spindle_init(); break; // Re-initialize spindle rpm calibration
      case 31: settings.rpm_min = value; spindle_init(); break; // Re-initialize spindle rpm calibration
      case 32:
        #ifdef VARIABLE_SPINDLE
          if (int_value) { settings.flags |= BITFLAG_LASER_MODE; }
          else { settings.flags &= ~BITFLAG_LASER_MODE; }
        #else
          return(STATUS_SETTING_DISABLED_LASER);
        #endif
        break;
	  case 33: settings.spindle_pwm_freq = value; spindle_init(); break; // Re-initialize spindle pwm calibration
      case 34: settings.spindle_pwm_off_value = value; spindle_init(); break; // Re-initialize spindle pwm calibration
      case 35: settings.spindle_pwm_min_value = value; spindle_init(); break; // Re-initialize spindle pwm calibration
      case 36: settings.spindle_pwm_max_value = value; spindle_init(); break; // Re-initialize spindle pwm calibration

      case 80:
      case 81:
      case 82:
      case 83:
      case 84:
        settings.machine_int16[parameter - 80] = int_value;
        break;

      case 90:
      case 91:
      case 92:
      case 93:
      case 94:
        settings.machine_float[parameter - 90] = value;
        break;
      default:
        return(STATUS_INVALID_STATEMENT);
    }
  }
  write_global_settings();
  return(STATUS_OK);
}

// Returns step pin mask according to Grbl internal axis indexing.
uint8_t get_step_pin_mask(uint8_t axis_idx)
{
  // todo clean this up further up stream
  return(1<<axis_idx);
}

// Returns direction pin mask according to Grbl internal axis indexing.
uint8_t get_direction_pin_mask(uint8_t axis_idx)
{      
	return(1<<axis_idx);
}

// this allows a conditional re-init of the trinamic settings
void settings_spi_driver_init() {
	#ifdef USE_TRINAMIC
		trinamic_change_settings();
	#else
		grbl_send(CLIENT_ALL, "[MSG: No SPI drivers setup]\r\n");
	#endif
}
