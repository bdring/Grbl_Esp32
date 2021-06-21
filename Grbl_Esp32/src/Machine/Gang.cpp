/*
    Part of Grbl_ESP32
    2021 -  Stefan de Bruijn, Mitch Bradley

    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Grbl_ESP32 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Gang.h"

#include "../Motors/Motor.h"
#include "../Motors/NullMotor.h"

namespace Machine {
    void Gang::group(Configuration::HandlerBase& handler) {
        handler.section("endstops", _endstops);
        Motors::MotorFactory::factory(handler, _motor);
    }
    void Gang::afterParse() {
        if (_motor == nullptr) {
            _motor = new Motors::Nullmotor();
        }
    }

    Gang::~Gang() {
        delete _motor;
        delete _endstops;
    }
}
