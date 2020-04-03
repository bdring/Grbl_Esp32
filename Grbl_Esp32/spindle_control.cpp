/*
  spindle_control.cpp - Header for system level commands and real-time processes
  Part of Grbl
  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC

	2018 -	Bart Dring This file was modified for use on the ESP32
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
#include "tools/SpindleClass.h"
#include "tools/SpindleClass.cpp"

int8_t spindle_pwm_chan_num;

// define a spindle type
//RelaySpindle my_spindle;
Laser my_spindle;

#ifdef SPINDLE_PWM_PIN
    static float pwm_gradient; // Precalulated value to speed up rpm to PWM conversions.
    uint32_t spindle_pwm_period; // how many counts in 1 period
    uint32_t spindle_pwm_off_value;
    uint32_t spindle_pwm_min_value;
    uint32_t spindle_pwm_max_value;
#endif

void spindle_init() {
    my_spindle.init();
    my_spindle.config_message();
}

void spindle_stop() {
    my_spindle.stop();
}

uint8_t spindle_get_state() {
    return my_spindle.get_state();
}

void spindle_set_speed(uint32_t pwm_value) {
    my_spindle.set_pwm(pwm_value);
}

uint32_t spindle_compute_pwm_value(float rpm) {
    return 0;
}


// Called by spindle_set_state() and step segment generator. Keep routine small and efficient.
void spindle_set_state(uint8_t state, float rpm) {
    my_spindle.set_state(state, rpm);
}


void spindle_sync(uint8_t state, float rpm) {
    if (sys.state == STATE_CHECK_MODE)
        return;
    protocol_buffer_synchronize(); // Empty planner buffer to ensure spindle is set when programmed.
    spindle_set_state(state, rpm);
}


void grbl_analogWrite(uint8_t chan, uint32_t duty) {
    
}

void spindle_set_enable(bool enable) {

}

uint32_t piecewise_linear_fit(float rpm) {
    uint32_t pwm_value;
#if (N_PIECES > 3)
    if (rpm > RPM_POINT34)
        pwm_value = floor(RPM_LINE_A4 * rpm - RPM_LINE_B4);
    else
#endif
#if (N_PIECES > 2)
        if (rpm > RPM_POINT23)
            pwm_value = floor(RPM_LINE_A3 * rpm - RPM_LINE_B3);
        else
#endif
#if (N_PIECES > 1)
            if (rpm > RPM_POINT12)
                pwm_value = floor(RPM_LINE_A2 * rpm - RPM_LINE_B2);
            else
#endif
            {
                pwm_value = floor(RPM_LINE_A1 * rpm - RPM_LINE_B1);
            }
    return pwm_value;
}



