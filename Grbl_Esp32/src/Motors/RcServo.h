#pragma once

/*
    RcServo.h

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

#include "Motor.h"

#include "Servo.h"
#include "RcServoSettings.h"

namespace Motors {
    class RcServo : public Servo {
    public:
        RcServo(uint8_t axis_index, uint8_t pwm_pin);

        // Overrides for inherited methods
        void init() override;
        void read_settings() override;
        bool set_homing_mode(bool isHoming) override;
        void set_disable(bool disable) override;
        void update() override;

        void _write_pwm(uint32_t duty);
        

    protected:
        void config_message() override;

        void set_location();

        uint8_t  _pwm_pin;
        uint8_t  _channel_num;
        uint32_t _current_pwm_duty;

        float _homing_position;

        float _pwm_pulse_min;
        float _pwm_pulse_max;

        bool _disabled;

        FloatSetting* rc_servo_cal_min;
        FloatSetting* rc_servo_cal_max;
        };
}
