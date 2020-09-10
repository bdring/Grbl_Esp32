/*
    AnalogOutput.cpp

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

#include "Grbl.h"

namespace UserOutput {
    AnalogOutput::AnalogOutput() {}

    AnalogOutput::AnalogOutput(uint8_t number, uint8_t pin, float pwm_frequency) {
        _number        = number;
        _pin           = pin;
        _pwm_frequency = pwm_frequency;

        // determine the highest bit precision allowed by frequency
        _resolution_bits = sys_calc_pwm_precision(_pwm_frequency);

        init();
    }

    void AnalogOutput::init() {
        _pwm_channel = sys_get_next_PWM_chan_num();
        if (_pwm_channel == -1) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Error, "Error: out of PWM channels");
        } else {
            ledcSetup(_pwm_channel, _pwm_frequency, _resolution_bits);
            ledcAttachPin(_pin, _pwm_channel);
        }
    }

    void AnalogOutput::disable() {
        if (_pwm_channel == -1)
            return;

        ledcWrite(_pwm_channel, 0);
    }

    void AnalogOutput::set_level(float percent) {
        float duty;

        if (_pwm_channel == -1)
            return;

        if (_current_value == percent)
            return;

        _current_value = percent;

        duty = (percent / 100.0) * (1 << _resolution_bits);

        ledcWrite(_pwm_channel, duty);
    }
}