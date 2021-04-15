/*
  Probe.cpp - code pertaining to probing methods
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

// Inverts the probe pin state depending on user settings and probing cycle mode.
static bool is_probe_away;

// Probe pin initialization routine.
void probe_init() {
    static bool show_init_msg = true;  // used to show message only once.

    if (PROBE_PIN != UNDEFINED_PIN) {
#ifdef DISABLE_PROBE_PIN_PULL_UP
        pinMode(PROBE_PIN, INPUT);
#else
        pinMode(PROBE_PIN, INPUT_PULLUP);  // Enable internal pull-up resistors. Normal high operation.
#endif

        if (show_init_msg) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Probe on pin %s", pinName(PROBE_PIN).c_str());
            show_init_msg = false;
        }
    }
}

void set_probe_direction(bool is_away) {
    is_probe_away = is_away;
}

// Returns the probe pin state. Triggered = true. Called by gcode parser and probe state monitor.
bool probe_get_state() {
    return (PROBE_PIN == UNDEFINED_PIN) ? false : digitalRead(PROBE_PIN) ^ probe_invert->get();
}

// Monitors probe pin state and records the system position when detected. Called by the
// stepper ISR per ISR tick.
// NOTE: This function must be extremely efficient as to not bog down the stepper ISR.
void probe_state_monitor() {
    if (probe_get_state() ^ is_probe_away) {
        sys_probe_state = Probe::Off;
        memcpy(sys_probe_position, sys_position, sizeof(sys_position));
        sys_rt_exec_state.bit.motionCancel = true;
    }
}
