/*
    SpindleClass.cpp

    A Spindle Class
        Spindle         - A base class. Do not use
        PWMSpindle     - A spindle with a PWM output
        RelaySpindle    - An on/off only spindle
        Laser           - Output is PWM, but the M4 laser power mode can be used
        DacSpindle      - Uses the DAC to output a 0-3.3V output

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
        Consider breaking into one file per class.

    Get rid of dependance on machine definition #defines       
        SPINDLE_PWM_PIN
        SPINDLE_ENABLE_PIN
        SPINDLE_DIR_PIN

*/
#include "grbl.h"
#include "SpindleClass.h"
#include "NullSpindle.cpp"
#include "PWMSpindle.cpp"
#include "DacSpindle.cpp"
#include "RelaySpindle.cpp"
#include "Laser.cpp"

NullSpindle null_spindle;
PWMSpindle pwm_spindle;
RelaySpindle relay_spindle;
Laser laser;
DacSpindle dac_spindle;

void spindle_select(uint8_t spindle_type) {
    
    switch (spindle_type) {
    case SPINDLE_TYPE_PWM:        
        my_spindle = &pwm_spindle;
        break;
    case SPINDLE_TYPE_RELAY:
        my_spindle = &relay_spindle;
        break;
    case SPINDLE_TYPE_LASER:
        my_spindle = &laser;
        break;
    case SPINDLE_TYPE_DAC:
        my_spindle = &dac_spindle;
        break;
    case SPINDLE_TYPE_NONE:
    default:
        my_spindle = &null_spindle;
        break;
    }
    my_spindle->init();
}

void spindle_read_prefs(Preferences &prefs) {
    uint8_t foo = prefs.getUChar("SPIN_TYPE", SPINDLE_TYPE_PWM);
}



bool Spindle::isRateAdjusted() {
    return false; // default for basic spindles is false
}

void Spindle :: spindle_sync(uint8_t state, float rpm) {
    if (sys.state == STATE_CHECK_MODE)
        return;
    protocol_buffer_synchronize(); // Empty planner buffer to ensure spindle is set when programmed.
    set_state(state, rpm);
}


