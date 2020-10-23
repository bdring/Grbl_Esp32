/*
  CoolantControl.cpp - coolant control methods
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

#include "Grbl.h"

void coolant_init() {
    static bool init_message = true;  // used to show messages only once.

#ifdef COOLANT_FLOOD_PIN
    pinMode(COOLANT_FLOOD_PIN, OUTPUT);
#endif
#ifdef COOLANT_MIST_PIN
    pinMode(COOLANT_MIST_PIN, OUTPUT);
#endif

    if (init_message) {
#ifdef COOLANT_FLOOD_PIN
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Flood coolant on pin %s", pinName(COOLANT_FLOOD_PIN).c_str());
#endif
#ifdef COOLANT_MIST_PIN
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Mist coolant on pin %s", pinName(COOLANT_MIST_PIN).c_str());
#endif
        init_message = false;
    }

    coolant_stop();
}

// Returns current coolant output state. Overrides may alter it from programmed state.
CoolantState coolant_get_state() {
    CoolantState cl_state = {};
    bool         pinState;

#ifdef COOLANT_FLOOD_PIN
    pinState = digitalRead(COOLANT_FLOOD_PIN);
#    ifdef INVERT_COOLANT_FLOOD_PIN
    pinState = !pinState;
#    endif
    if (pinState) {
        cl_state.Flood = 1;
    }
#endif

#ifdef COOLANT_MIST_PIN
    pinState = digitalRead(COOLANT_MIST_PIN);
#    ifdef INVERT_COOLANT_MIST_PIN
    pinState = !pinState;
#    endif
    if (pinState) {
        cl_state.Mist = 1;
    }
#endif

    return cl_state;
}

static inline void coolant_write(CoolantState state) {
    bool pinState;

#ifdef COOLANT_FLOOD_PIN
    pinState = state.Flood;
#    ifdef INVERT_COOLANT_FLOOD_PIN
    pinState = !pinState;
#    endif
    digitalWrite(COOLANT_FLOOD_PIN, pinState);
#endif

#ifdef COOLANT_MIST_PIN
    pinState = state.Mist;
#    ifdef INVERT_COOLANT_MIST_PIN
    pinState = !pinState;
#    endif
    digitalWrite(COOLANT_MIST_PIN, pinState);
#endif
}

// Directly called by coolant_init(), coolant_set_state(), and mc_reset(), which can be at
// an interrupt-level. No report flag set, but only called by routines that don't need it.
void coolant_stop() {
    CoolantState disable = {};
    coolant_write(disable);
}

// Main program only. Immediately sets flood coolant running state and also mist coolant,
// if enabled. Also sets a flag to report an update to a coolant state.
// Called by coolant toggle override, parking restore, parking retract, sleep mode, g-code
// parser program end, and g-code parser coolant_sync().

void coolant_set_state(CoolantState state) {
    if (sys.abort) {
        return;  // Block during abort.
    }
    coolant_write(state);
    sys.report_ovr_counter = 0;  // Set to report change immediately
}

void coolant_off() {
    CoolantState disable = {};
    coolant_set_state(disable);
}

// G-code parser entry-point for setting coolant state. Forces a planner buffer sync and bails
// if an abort or check-mode is active.
void coolant_sync(CoolantState state) {
    if (sys.state == State::CheckMode) {
        return;
    }
    protocol_buffer_synchronize();  // Ensure coolant turns on when specified in program.
    coolant_set_state(state);
}
