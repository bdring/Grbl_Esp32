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

    DigitalOutput::DigitalOutput(uint8_t number, Pin pin) {
        _number = number;
        _pin    = pin;

        if (_pin == Pin::UNDEFINED) {
            return;
        }

        init();
    }

    void DigitalOutput::init() {
        _pin.setAttr(Pin::Attr::Output);
        _pin.off();

        config_message();
    }

    void DigitalOutput::config_message() {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "User Digital Output:%d on Pin:%s", _number, _pin.name().c_str());
    }

    bool DigitalOutput::set_level(bool isOn) {
        if (_number == UNDEFINED_OUTPUT && isOn) {
            return false;
        }

        _pin.write(isOn);
        return true;
    }

    // ==================================================================

    AnalogOutput::AnalogOutput() {}

    AnalogOutput::AnalogOutput(uint8_t number, Pin pin, float pwm_frequency) {
        _number        = number;
        _pin           = pin;
        _pwm_frequency = pwm_frequency;

        if (pin == Pin::UNDEFINED) {
            return;
        }

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
            _pin.setAttr(Pin::Attr::Output);
            auto nativePin = _pin.getNative(Pin::Capabilities::PWM);

            ledcSetup(_pwm_channel, _pwm_frequency, _resolution_bits);
            ledcAttachPin(nativePin, _pwm_channel);
            ledcWrite(_pwm_channel, 0);

            config_message();
        }
    }

    void AnalogOutput::config_message() {
        grbl_msg_sendf(
            CLIENT_SERIAL, MsgLevel::Info, "User Analog Output:%d on Pin:%s Freq:%0.0fHz", _number, _pin.name().c_str(), _pwm_frequency);
    }

    // returns true if able to set value
    bool AnalogOutput::set_level(uint32_t numerator) {
        // look for errors, but ignore if turning off to prevent mask turn off from generating errors
        if (_pin == Pin::UNDEFINED) {
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
