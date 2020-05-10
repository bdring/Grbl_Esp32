/*
    StandardStepperClass.cpp

    This is used for a stepper motor that just requires step and direction
    pins.

    TODO: Add an enable pin 

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

StandardStepper :: StandardStepper(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin) {
    this->axis_index = axis_index;
    this->step_pin = step_pin;
    this->dir_pin = dir_pin;
    init();
    config_message();
}

void StandardStepper :: init() {
    _is_homing = false;

    // TODO Step but, but RMT complicates things
    pinMode(dir_pin, OUTPUT);
}


void StandardStepper :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "%c Axis standard stepper motor Step:%d Dir:%d",
                   report_get_axis_letter(axis_index),
                   step_pin,
                   dir_pin);
}