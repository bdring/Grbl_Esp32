/*
    Laser.cpp

    This is similar the the PWM Spindle except that it allows the
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
#include "SpindleClass.h"

// ===================================== Laser ==============================================


bool Laser :: isRateAdjusted() {
    return true; // can use M4 (CCW) laser mode.
}

void Laser :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "Laser spindle on GPIO:%d, Freq:%.2fHz, Res:%dbits Laser mode:$32=%d",
                   report_pin_number(_output_pin),
                   _pwm_freq,
                   _pwm_precision,
                   isRateAdjusted());  // the current mode
}
