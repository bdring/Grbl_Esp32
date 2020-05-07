/*
    SpindleClass.cpp

    A Base class for spindles and spinsle like things such as lasers

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

    TODO
        Add Spindle spin up/down delays

    Get rid of dependance on machine definition #defines
        SPINDLE_OUTPUT_PIN
        SPINDLE_ENABLE_PIN
        SPINDLE_DIR_PIN

*/
#include "NullSpindle.cpp"
#include "PWMSpindle.cpp"
#include "DacSpindle.cpp"
#include "RelaySpindle.cpp"
#include "Laser.cpp"
#include "HuanyangSpindle.cpp"
#include "BESCSpindle.cpp"


// An instance of each type of spindle is created here.
// This allows the spindle to be dynamicly switched
NullSpindle null_spindle;
PWMSpindle pwm_spindle;
RelaySpindle relay_spindle;
Laser laser;
DacSpindle dac_spindle;
HuanyangSpindle huanyang_spindle;
BESCSpindle besc_spindle;


void spindle_select(uint8_t spindle_type) {
    
    switch (spindle_type) {
    case SPINDLE_TYPE_PWM:
        spindle = &pwm_spindle;
        break;
    case SPINDLE_TYPE_RELAY:
        spindle = &relay_spindle;
        break;
    case SPINDLE_TYPE_LASER:
        spindle = &laser;
        break;
    case SPINDLE_TYPE_DAC:
        spindle = &dac_spindle;
        break;
    case SPINDLE_TYPE_HUANYANG:
        spindle = &huanyang_spindle;
        break;
    case SPINDLE_TYPE_BESC:
        spindle = &besc_spindle;
        break;
    case SPINDLE_TYPE_NONE:
    default:
        spindle = &null_spindle;
        break;
    }
    
    spindle->init();
}

// ========================= Spindle ==================================

bool Spindle::isRateAdjusted() {
    return false; // default for basic spindle is false
}

void Spindle :: spindle_sync(uint8_t state, uint32_t rpm) {
    if (sys.state == STATE_CHECK_MODE)
        return;
    protocol_buffer_synchronize(); // Empty planner buffer to ensure spindle is set when programmed.
    set_state(state, rpm);
}
