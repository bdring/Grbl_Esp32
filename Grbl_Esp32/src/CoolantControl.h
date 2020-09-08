#pragma once

/*
  CoolantControl.h - spindle control methods
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC

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

// Initializes coolant control pins.
void coolant_init();

// Returns current coolant output state. Overrides may alter it from programmed state.
CoolantState coolant_get_state();

// Immediately disables coolant pins.
void coolant_stop();

// Sets the coolant pins according to state specified.
void coolant_off();
void coolant_set_state(CoolantState state);

// G-code parser entry-point for setting coolant states. Checks for and executes additional conditions.
void coolant_sync(CoolantState state);
