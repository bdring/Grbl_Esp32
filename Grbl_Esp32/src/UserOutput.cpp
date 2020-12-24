/*
    UserOutput.cpp

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
    DigitalOutput::DigitalOutput() {}

    DigitalOutput::DigitalOutput(uint8_t number, uint8_t pin) {
        _number = number;
        _pin    = pin;

        if (_pin == UNDEFINED_PIN)
            return;

        init();
    }

    void DigitalOutput::init() {
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);

        config_message();
    }

    void DigitalOutput::config_message() {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "User Digital Output:%d on Pin:%s", _number, pinName(_pin).c_str());
    }

    bool DigitalOutput::set_level(bool isOn) {
        if (_number == UNDEFINED_PIN && isOn) {
            return false;
        }

        digitalWrite(_pin, isOn);
        return true;
    }

    // ==================================================================

    AnalogOutput::AnalogOutput() {}

    AnalogOutput::AnalogOutput(uint8_t number, uint8_t pin, float pwm_frequency) {
        _number        = number;
        _pin           = pin;
        _pwm_frequency = pwm_frequency;

        if (pin == UNDEFINED_PIN)
            return;

        // determine the highest resolution (number of precision bits) allowed by frequency

        uint32_t apb_frequency = getApbFrequency();

        // increase the precision (bits) until it exceeds allow by frequency the max or is 16
        _resolution_bits = 0;
        while ((1 << _resolution_bits) < (apb_frequency / _pwm_frequency) && _resolution_bits <= 16) {
            ++_resolution_bits;
        }
        // _resolution_bits is now just barely too high, so drop it down one
        --_resolution_bits;

        init();
    }

    void AnalogOutput::init() {
        _pwm_channel = sys_get_next_PWM_chan_num();
        if (_pwm_channel == -1) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Error, "Error: out of PWM channels");
        } else {
            ledcSetup(_pwm_channel, _pwm_frequency, _resolution_bits);
            ledcAttachPin(_pin, _pwm_channel);
            ledcWrite(_pwm_channel, 0);

            config_message();
        }
    }

    void AnalogOutput::config_message() {
        grbl_msg_sendf(
            CLIENT_SERIAL, MsgLevel::Info, "User Analog Output:%d on Pin:%s Freq:%0.0fHz", _number, pinName(_pin).c_str(), _pwm_frequency);
    }

    // returns true if able to set value
    bool AnalogOutput::set_level(uint32_t numerator) {
        // look for errors, but ignore if turning off to prevent mask turn off from generating errors
        if (_pin == UNDEFINED_PIN) {
            return false;
        }

        if (_pwm_channel == -1) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "M67 PWM channel error");
            return false;
        }

        if (_current_value == numerator) {
            return true;
        }

        _current_value = numerator;

        ledcWrite(_pwm_channel, numerator);

        return true;
    }
}
