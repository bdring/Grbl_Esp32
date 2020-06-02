/*
  coolant_control.c - coolant control methods
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

#include "grbl.h"

OutPin* coolantFloodPin;
OutPin* coolantMistPin;
bool invertFloodPin;
bool invertMistPin;

void coolant_init() {
#ifdef COOLANT_FLOOD_PIN
    coolantFloodPin = new COOLANT_FLOOD_PIN;
#else
    coolantFloodPin = UNDEFINED_PIN;
#endif
#ifdef COOLANT_MIST_PIN
    coolantFloodPin = new COOLANT_MIST_PIN;
#else
    coolantMistPin = UNDEFINED_PIN;
#endif
#ifdef INVERT_COOLANT_FLOOD_PIN
    coolantFloodPin = true;
#else
    coolantFloodPin = false;
#endif
#ifdef INVERT_COOLANT_MIST_PIN
    coolantMistPin = true;
#else
    coolantMistPin = false;
#endif
    coolant_stop();
}

// Returns current coolant output state. Overrides may alter it from programmed state.
uint8_t coolant_get_state() {
    uint8_t cl_state = COOLANT_STATE_DISABLE;
    if (coolantFloodPin && (coolantFloodPin->read() ^ invertFloodPin)) {
        cl_state |= COOLANT_STATE_FLOOD;
    }
    if (coolantMistPin && (coolantMistPin->read() ^ invertMistPin)) {
        cl_state |= COOLANT_STATE_MIST;
    }
    return (cl_state);
}

// Directly called by coolant_init(), coolant_set_state(), and mc_reset(), which can be at
// an interrupt-level. No report flag set, but only called by routines that don't need it.

void coolant_stop() {
    if (coolantFloodPin) {
        coolantFloodPin->write(invertFloodPin);
    }

    if (coolantMistPin) {
        coolantMistPin->write(invertMistPin);
    }
}

// Main program only. Immediately sets flood coolant running state and also mist coolant,
// if enabled. Also sets a flag to report an update to a coolant state.
// Called by coolant toggle override, parking restore, parking retract, sleep mode, g-code
// parser program end, and g-code parser coolant_sync().
void coolant_set_state(uint8_t mode) {
    if (sys.abort)  return;   // Block during abort.
    if (mode == COOLANT_DISABLE) {
        coolant_stop();
    } else {
        if (coolantFloodPin) {
            if (mode & COOLANT_FLOOD_ENABLE) {
                coolantFloodPin->write(!invertFloodPin);
            }
        }
        if (coolantMistPin) {
            if (mode & COOLANT_MIST_ENABLE) {
                coolantMistPin->write(!invertMistPin);
            }
        }
    }
    sys.report_ovr_counter = 0; // Set to report change immediately
}

// G-code parser entry-point for setting coolant state. Forces a planner buffer sync and bails
// if an abort or check-mode is active.
void coolant_sync(uint8_t mode) {
    if (sys.state == STATE_CHECK_MODE)  return;
    protocol_buffer_synchronize(); // Ensure coolant turns on when specified in program.
    coolant_set_state(mode);
}
