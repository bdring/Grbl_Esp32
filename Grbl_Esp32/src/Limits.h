#pragma once

/*
  Limits.h - code pertaining to limit-switches and performing the homing cycle
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

	2018 -	Bart Dring This file was modifed for use on the ESP32
					CPU. Do not use this with Grbl for atMega328P
  2018-12-29 - Wolfgang Lienbacher renamed file from limits.h to grbl_limits.h
          fixing ambiguation issues with limit.h in the esp32 Arduino Framework
          when compiling with VS-Code/PlatformIO.

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

extern uint8_t n_homing_locate_cycle;

// Initialize the limits module
void limits_init();

// Disables hard limits.
void limits_disable();

// Returns limit state as a bit-wise uint8 variable.
AxisMask limits_get_state();

// Perform one portion of the homing cycle based on the input settings.
void limits_go_home(uint8_t cycle_mask);

// Check for soft limit violations
void limits_soft_check(float* target);

void isr_limit_switches();

// A task that runs after a limit switch interrupt.
void limitCheckTask(void* pvParameters);

float limitsMaxPosition(uint8_t axis);
float limitsMinPosition(uint8_t axis);

// Internal factor used by limits_soft_check
bool limitsCheckTravel(float* target);

// check if a switch has been defined
bool limitsSwitchDefined(uint8_t axis, uint8_t gang_index);
