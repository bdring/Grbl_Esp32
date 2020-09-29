#pragma once

/*
    Motor.h
    Header file for Motor Classes
    Here is the hierarchy
        Motor
            Nullmotor
            StandardStepper
                TrinamicDriver
            Unipolar
            RC Servo

    These are for motors coordinated by Grbl_ESP32
    See motorClass.cpp for more details

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

#include "Motors.h"

#include <cstdint>

namespace Motors {
    class Motor {
    public:
        Motor();

        virtual void init();  // not in constructor because this also gets called when $$ settings change
        virtual void config_message();
        virtual void debug_message();
        virtual void read_settings();
        virtual void set_homing_mode(uint8_t homing_mask, bool isHoming);
        virtual void set_disable(bool disable);
        virtual void set_direction_pins(uint8_t onMask);
        virtual void step(uint8_t step_mask, uint8_t dir_mask);  // only used on Unipolar right now
        virtual bool test();
        virtual void set_axis_name();
        virtual void update();
        virtual bool can_home();

        motor_class_id_t type_id;
        uint8_t          is_active  = false;
        uint8_t          has_errors = false;

    protected:
        uint8_t _axis_index;       // X_AXIS, etc
        uint8_t _dual_axis_index;  // 0 = primary 1=ganged

        bool    _showError;
        bool    _use_mpos = true;
        uint8_t _homing_mask;
        char    _axis_name[10];  // this the name to use when reporting like "X" or "X2"

        float _position_min = 0;
        float _position_max = 0;  // position in millimeters

        bool _can_home = true;
        bool _disabled;
    };
}
