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

#include "RcServoSettings.h"

namespace Motors {
    class RcServo : public Motor {
    public:
        RcServo();
        RcServo(uint8_t axis_index, uint8_t pwm_pin, float cal_min, float cal_max);
        virtual void config_message();
        virtual void init();
        void         _write_pwm(uint32_t duty);
        virtual void set_disable(bool disable);
        virtual void update();
        void         read_settings();
        void         set_homing_mode(uint8_t homing_mask, bool isHoming) override;

    protected:
        void set_location();

        uint8_t  _pwm_pin;
        uint8_t  _channel_num;
        uint32_t _current_pwm_duty;
        

        float _homing_position;

        float _pwm_pulse_min;
        float _pwm_pulse_max;

        float _cal_min = 1.0;
        float _cal_max = 1.0;
    };
}
