/*
    Motor.cpp
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
    TODO
        Make sure public/private/protected is cleaned up.
        Only a few Unipolar axes have been setup in init()
        Get rid of Z_SERVO, just reply on Z_SERVO_PIN
        Deal with custom machine ... machine_trinamic_setup();
        Class is ready to deal with non SPI pins, but they have not been needed yet.
            It would be nice in the config message though
    Testing
        Done (success)
            3 Axis (3 Standard Steppers)
            MPCNC (ganged with shared direction pin)
            TMC2130 Pen Laser (trinamics, stallguard tuning)
            Unipolar
        TODO
            4 Axis SPI (Daisy Chain, Ganged with unique direction pins)
    Reference
        TMC2130 Datasheet https://www.trinamic.com/fileadmin/assets/Products/ICs_Documents/TMC2130_datasheet.pdf
*/

#include "Motor.h"

namespace Motors {
    Motor::Motor(motor_class_id_t type, uint8_t axis_index) :
        type_id(type), _axis_index(axis_index % MAX_AXES), _dual_axis_index(axis_index / MAX_AXES) {}

    void Motor::init() {}

    void Motor::config_message() {}
    void Motor::debug_message() {}

    void Motor::read_settings() {
        float max_travel = axis_settings[_axis_index]->max_travel->get();
        float mpos       = axis_settings[_axis_index]->home_mpos->get();

        if (bitnum_istrue(homing_dir_mask->get(), _axis_index)) {
            _position_min = mpos;
            _position_max = mpos + max_travel;
        } else {
            _position_min = mpos - max_travel;
            _position_max = mpos;
        }
    }

    void Motor::set_disable(bool disable) {}
    void Motor::set_direction(bool dir) {}
    void Motor::step() {}
    void Motor::unstep() {}
    bool Motor::test() { return true; };  // true = OK
    void Motor::update() {}
    bool Motor::can_home() { return _can_home; };

    void  Motor::set_homing_mode(bool isHoming) {}
    char* Motor::axis_name() {
        static char name[10];
        sprintf(name, "%c%s", report_get_axis_letter(_axis_index), _dual_axis_index ? "2" : " ");
        return name;
    }
}
