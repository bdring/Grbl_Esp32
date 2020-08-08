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

// this is the pulse range of a the servo. Typical servos are 0.001 to 0.002 seconds
// some servos have a wider range. You can adjust this here or in the calibration feature
#define SERVO_MIN_PULSE_SEC 0.001  // min pulse in seconds
#define SERVO_MAX_PULSE_SEC 0.002  // max pulse in seconds

#define SERVO_POSITION_MIN_DEFAULT 0.0   // mm
#define SERVO_POSITION_MAX_DEFAULT 20.0  // mm

#define SERVO_PULSE_FREQ 50  // 50Hz ...This is a standard analog servo value. Digital ones can repeat faster

#define SERVO_PULSE_RES_BITS 16      // bits of resolution of PWM (16 is max)
#define SERVO_PULSE_RES_COUNT 65535  // see above  TODO...do the math here 2^SERVO_PULSE_RES_BITS

#define SERVO_TIME_PER_BIT ((1.0 / (float)SERVO_PULSE_FREQ) / ((float)SERVO_PULSE_RES_COUNT))  // seconds

#define SERVO_MIN_PULSE (uint16_t)(SERVO_MIN_PULSE_SEC / SERVO_TIME_PER_BIT)  // in timer counts
#define SERVO_MAX_PULSE (uint16_t)(SERVO_MAX_PULSE_SEC / SERVO_TIME_PER_BIT)  // in timer counts

#define SERVO_PULSE_RANGE (SERVO_MAX_PULSE - SERVO_MIN_PULSE)

#define SERVO_CAL_MIN 20.0   // Percent: the minimum allowable calibration value
#define SERVO_CAL_MAX 180.0  // Percent: the maximum allowable calibration value

#define SERVO_TIMER_INT_FREQ 50.0  // Hz This is the task frequency

namespace Motors {
    class RcServo : public Motor {
    public:
        RcServo();
        RcServo(uint8_t axis_index, uint8_t pwm_pin, float min, float max);
        virtual void config_message();
        virtual void init();
        void         _write_pwm(uint32_t duty);
        virtual void set_disable(bool disable);
        virtual void update();
        void         read_settings();
        void         set_homing_mode(bool is_homing, bool isHoming);

    protected:
        void set_location();
        void _get_calibration();

        uint8_t  _pwm_pin;
        uint8_t  _channel_num;
        uint32_t _current_pwm_duty;
        bool     _disabled;

        float _position_min;
        float _position_max;  // position in millimeters
        float _homing_position;

        float _pwm_pulse_min;
        float _pwm_pulse_max;
    };
}