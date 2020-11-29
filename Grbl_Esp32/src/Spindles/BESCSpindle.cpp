/*
    BESCSpindle.cpp

    This a special type of PWM spindle for RC type Brushless DC Speed
    controllers. They use a short pulse for off and a longer pulse for
    full on. The pulse is always a small portion of the full cycle.
    Some BESCs have a special turn on procedure. This may be a one time
    procedure or must be done every time. The user must do that via gcode.

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

    Important ESC Settings
    50 Hz this is a typical frequency for an ESC
    Some ESCs can handle higher frequencies, but there is no advantage to changing it.

    Determine the typical min and max pulse length of your ESC
    BESC_MIN_PULSE_SECS is typically 1ms (0.001 sec) or less
    BESC_MAX_PULSE_SECS is typically 2ms (0.002 sec) or more

*/
#include "BESCSpindle.h"

// don't change these
const double BESC_PWM_FREQ     = 50.0;  // Hz
const double BESC_PULSE_PERIOD = (1.0 / BESC_PWM_FREQ);

// Ok to tweak. These are the pulse lengths in seconds
// #define them in your machine definition file if you want different values
#ifndef BESC_MIN_PULSE_SECS
#    define BESC_MIN_PULSE_SECS 0.0009f  // in seconds
#endif

#ifndef BESC_MAX_PULSE_SECS
#    define BESC_MAX_PULSE_SECS 0.0022f  // in seconds
#endif

//calculations...don't change
const uint16_t BESC_MIN_PULSE_CNT = static_cast<uint16_t>(BESC_MIN_PULSE_SECS / BESC_PULSE_PERIOD * 65535.0);
const uint16_t BESC_MAX_PULSE_CNT = static_cast<uint16_t>(BESC_MAX_PULSE_SECS / BESC_PULSE_PERIOD * 65535.0);

namespace Spindles {
    void BESC::init() {
        get_pins_and_settings();  // these gets the standard PWM settings, but many need to be changed for BESC

        if (_output_pin == UNDEFINED_PIN) {
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Warning: BESC output pin not defined");
            return;  // We cannot continue without the output pin
        }

        // override some settings to what is required for a BESC
        _pwm_freq      = (uint32_t)BESC_PWM_FREQ;
        _pwm_precision = 16;

        // override these settings
        _pwm_off_value = BESC_MIN_PULSE_CNT;
        _pwm_min_value = _pwm_off_value;
        _pwm_max_value = BESC_MAX_PULSE_CNT;

        ledcSetup(_pwm_chan_num, (double)_pwm_freq, _pwm_precision);  // setup the channel
        ledcAttachPin(_output_pin, _pwm_chan_num);                    // attach the PWM to the pin

        pinMode(_enable_pin, OUTPUT);

        set_rpm(0);

        use_delays = true;

        config_message();
    }

    // prints the startup message of the spindle config
    void BESC::config_message() {
        grbl_msg_sendf(CLIENT_ALL,
                       MsgLevel::Info,
                       "BESC spindle on Pin:%s Min:%0.2fms Max:%0.2fms Freq:%dHz Res:%dbits",
                       pinName(_output_pin).c_str(),
                       BESC_MIN_PULSE_SECS * 1000.0,  // convert to milliseconds
                       BESC_MAX_PULSE_SECS * 1000.0,  // convert to milliseconds
                       _pwm_freq,
                       _pwm_precision);
    }

    uint32_t BESC::set_rpm(uint32_t rpm) {
        uint32_t pwm_value;

        if (_output_pin == UNDEFINED_PIN) {
            return rpm;
        }

        // apply speed overrides
        rpm = rpm * sys.spindle_speed_ovr / 100;  // Scale by spindle speed override value (percent)

        // apply limits limits
        if ((_min_rpm >= _max_rpm) || (rpm >= _max_rpm)) {
            rpm = _max_rpm;
        } else if (rpm != 0 && rpm <= _min_rpm) {
            rpm = _min_rpm;
        }
        sys.spindle_speed = rpm;

        // determine the pwm value
        if (rpm == 0) {
            pwm_value = _pwm_off_value;
        } else {
            pwm_value = map_uint32_t(rpm, _min_rpm, _max_rpm, _pwm_min_value, _pwm_max_value);
        }

        set_output(pwm_value);
        return rpm;
    }
}
