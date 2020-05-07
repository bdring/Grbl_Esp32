/*
    DacSpindle.cpp

    This uses the Analog DAC in the ESP32 to generate a voltage
    proportional to the GCode S value desired. Some spindle uses
    a 0-5V or 0-10V value to control the spindle. You would use
    an Op Amp type circuit to get from the 0.3.3V of the ESP32 to that voltage.

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

// ======================================== DacSpindle ======================================
void DacSpindle :: init() {
    get_pins_and_settings();

    if (_output_pin == UNDEFINED_PIN)
        return;

    _min_rpm = settings.rpm_min;
    _max_rpm = settings.rpm_max;
    _pwm_min_value = 0;     // not actually PWM...DAC counts
    _pwm_max_value = 255;   // not actually PWM...DAC counts
    _gpio_ok = true;

    if (_output_pin != GPIO_NUM_25 && _output_pin != GPIO_NUM_26) { // DAC can only be used on these pins
        _gpio_ok = false;
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "DAC spindle pin invalid GPIO_NUM_%d (pin 25 or 26 only)", _output_pin);
        return;
    }

    if (_enable_pin != UNDEFINED_PIN)
        pinMode(_enable_pin, OUTPUT);

    if (_direction_pin != UNDEFINED_PIN) {
        pinMode(_direction_pin, OUTPUT);
    }

    is_reversable = (_direction_pin != UNDEFINED_PIN);

    config_message();
}

void DacSpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "DAC spindle Output:%d, Enbl:%d, Dir:%d, Res:8bits",
                   report_pin_number(_output_pin),
                   report_pin_number(_enable_pin), // 255 means pin not defined
                   report_pin_number(_direction_pin)); // 255 means pin not defined                  
}

uint32_t DacSpindle::set_rpm(uint32_t rpm) {
    if (_output_pin == UNDEFINED_PIN)
        return rpm;

    uint32_t pwm_value;    

    // apply overrides and limits
    rpm = rpm * sys.spindle_speed_ovr / 100; // Scale by spindle speed override value (percent)

    // Calculate PWM register value based on rpm max/min settings and programmed rpm.
    if ((_min_rpm >= _max_rpm) || (rpm >= _max_rpm)) {
        // No PWM range possible. Set simple on/off spindle control pin state.
        sys.spindle_speed = _max_rpm;
        pwm_value = _pwm_max_value;
    } else if (rpm <= _min_rpm) {
        if (rpm == 0) { // S0 disables spindle
            sys.spindle_speed = 0;
            pwm_value = 0;
        } else { // Set minimum PWM output
            rpm = _min_rpm;
            sys.spindle_speed = rpm;
            pwm_value = 0;
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Spindle RPM less than min RPM:%5.2f %d", rpm, pwm_value);
        }
    } else {
        // Compute intermediate PWM value with linear spindle speed model.
        // NOTE: A nonlinear model could be installed here, if required, but keep it VERY light-weight.
        sys.spindle_speed = rpm;
        
        pwm_value = map_uint32_t(rpm, _min_rpm, _max_rpm, _pwm_min_value, _pwm_max_value);
    }

    if (_off_with_zero_speed) {
        set_enable_pin(rpm != 0);
    }
    
    set_output(pwm_value);

    return rpm;
}

void DacSpindle :: set_output(uint32_t duty) {    
    if (_gpio_ok) {
         dacWrite(_output_pin, (uint8_t)duty);
    }
}
