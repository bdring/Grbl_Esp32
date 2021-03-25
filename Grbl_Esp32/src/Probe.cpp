#include "Probe.h"

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
#include "Pin.h"

// Probe pin initialization routine.
void Probe::init() {
    static bool show_init_msg = true;  // used to show message only once.

    if (!_probePin.undefined()) {
#ifdef DISABLE_PROBE_PIN_PULL_UP
        _probePin.setAttr(Pin::Attr::Input);
#else
        if (_probePin.capabilities().has(Pins::PinCapabilities::PullUp)) {
            _probePin.setAttr(Pin::Attr::Input | Pin::Attr::PullUp);  // Enable internal pull-up resistors. Normal high operation.
        } else {
            _probePin.setAttr(Pin::Attr::Input);
        }
#endif

        if (show_init_msg) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Probe on pin %s", _probePin.name().c_str());
            show_init_msg = false;
        }
    }
}

void Probe::set_direction(bool is_away) {
    _isProbeAway = is_away;
}

// Returns the probe pin state. Triggered = true. Called by gcode parser and probe state monitor.
bool Probe::get_state() {
    return _probePin.read() ^ probe_invert->get();
}

// Monitors probe pin state and records the system position when detected. Called by the
// stepper ISR per ISR tick.
// NOTE: This function must be extremely efficient as to not bog down the stepper ISR.
void Probe::state_monitor() {
    if (get_state() ^ _isProbeAway) {
        sys_probe_state = ProbeState::Off;
        memcpy(sys_probe_position, sys_position, sizeof(sys_position));
        sys_rt_exec_state.bit.motionCancel = true;
    }
}

void Probe::validate() const {}

void Probe::handle(Configuration::HandlerBase& handler)
{
    handler.handle("pin", _probePin);
}
