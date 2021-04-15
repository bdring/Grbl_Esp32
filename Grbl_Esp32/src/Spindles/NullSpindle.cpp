/*
    NullSpindle.cpp

    This is used when you don't want to use a spindle No I/O will be used
    and most methods don't do anything

    Part of Grbl_ESP32
    2020 -	Bart Dring

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
#include "NullSpindle.h"

namespace Spindles {
    // ======================= Null ==============================
    // Null is just bunch of do nothing (ignore) methods to be used when you don't want a spindle

    void Null::init() {
        is_reversable = false;
        use_delays    = false;
        config_message();
    }
    uint32_t Null::set_rpm(uint32_t rpm) {
        sys.spindle_speed = rpm;
        return rpm;
    }
    void Null::set_state(SpindleState state, uint32_t rpm) {
        _current_state    = state;
        sys.spindle_speed = rpm;
    }
    SpindleState Null::get_state() { return _current_state; }
    void         Null::stop() {}
    void         Null::config_message() { grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "No spindle"); }
}
