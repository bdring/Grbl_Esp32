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
#include "SpindleClass.h"

// ======================= NullSpindle ==============================
// NullSpindle is just bunch of do nothing (ignore) methods to be used when you don't want a spindle

void NullSpindle :: init() {
    is_reversable = false;
    config_message();
}
uint32_t NullSpindle :: set_rpm(uint32_t rpm) {
    return rpm;
}
void NullSpindle :: set_state(uint8_t state, uint32_t rpm) {}
uint8_t NullSpindle :: get_state() {
    return (SPINDLE_STATE_DISABLE);
}
void NullSpindle :: stop() {}
void NullSpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "No spindle");
}
