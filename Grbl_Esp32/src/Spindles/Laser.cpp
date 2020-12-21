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
    bool Laser::inLaserMode() {
        return laser_mode->get();  // can use M4 (CCW) laser mode.
    }

    void Laser::config_message() {
        grbl_msg_sendf(CLIENT_ALL,
                       MsgLevel::Info,
                       "Laser spindle on Pin:%s, Enbl:%s, Freq:%dHz, Res:%dbits Laser mode:%s",
                       pinName(_output_pin).c_str(),
                       pinName(_enable_pin).c_str(),
                       _pwm_freq,
                       _pwm_precision,
                       laser_mode->getStringValue());  // the current mode

        use_delays = false;  // this will override the value set in Spindle::PWM::init()
    }

    // Get the GPIO from the machine definition
    void Laser::get_pins_and_settings() {
        // setup all the pins

#ifdef LASER_OUTPUT_PIN
        _output_pin = LASER_OUTPUT_PIN;
#else
        _output_pin = UNDEFINED_PIN;
#endif

        _invert_pwm = spindle_output_invert->get();

#ifdef LASER_ENABLE_PIN
        _enable_pin = LASER_ENABLE_PIN;
#else
        _enable_pin = UNDEFINED_PIN;
#endif

        if (_output_pin == UNDEFINED_PIN) {
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Warning: LASER_OUTPUT_PIN not defined");
            return;  // We cannot continue without the output pin
        }

        _off_with_zero_speed = spindle_enbl_off_with_zero_speed->get();

        _direction_pin = UNDEFINED_PIN;
        is_reversable  = false;

        _pwm_freq      = spindle_pwm_freq->get();
        _pwm_precision = calc_pwm_precision(_pwm_freq);  // detewrmine the best precision
        _pwm_period    = (1 << _pwm_precision);

        // pre-caculate some PWM count values
        _pwm_off_value = 0;
        _pwm_min_value = 0;
        _pwm_max_value = _pwm_period;

        _min_rpm = 0;
        _max_rpm = laser_full_power->get();

        _piecewide_linear = false;

        _pwm_chan_num = 0;  // Channel 0 is reserved for spindle use
    }

    void Laser::deinit() {
        stop();
#ifdef LASER_OUTPUT_PIN
        gpio_reset_pin(LASER_OUTPUT_PIN);
        pinMode(LASER_OUTPUT_PIN, INPUT);
#endif

#ifdef LASER_ENABLE_PIN
        gpio_reset_pin(LASER_ENABLE_PIN);
        pinMode(LASER_ENABLE_PIN, INPUT);
#endif
    }
}
