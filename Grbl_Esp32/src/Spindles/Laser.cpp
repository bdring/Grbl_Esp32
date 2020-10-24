/*
    Laser.cpp

    This is similar to the PWM Spindle except that it allows the
    M4 speed vs. power copensation.

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
#include "Laser.h"

// ===================================== Laser ==============================================

namespace Spindles {
    bool Laser::isRateAdjusted() {
        return true;  // can use M4 (CCW) laser mode.
    }

    void Laser::config_message() {
        grbl_msg_sendf(CLIENT_SERIAL,
                       MsgLevel::Info,
                       "Laser spindle on Pin:%s, Freq:%dHz, Res:%dbits Laser mode:%s",
                       pinName(_output_pin).c_str(),
                       _pwm_freq,
                       _pwm_precision,
                       laser_mode->getStringValue());  // the current mode

        use_delays = false;  // this will override the value set in Spindle::PWM::init()
    }
}
