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

#include "UserOutputs.h"

namespace Machine {
    UserOutputs::UserOutputs() {
        for (int i = 0; i < 4; ++i) {
            _analogFrequency[i] = 5000;
        }
    }

    void UserOutputs::group(Configuration::HandlerBase& handler) {
        handler.item("analog0", _analogOutput[0]);
        handler.item("analog1", _analogOutput[1]);
        handler.item("analog2", _analogOutput[2]);
        handler.item("analog3", _analogOutput[3]);
        handler.item("analog_frequency0", _analogFrequency[0]);
        handler.item("analog_frequency1", _analogFrequency[1]);
        handler.item("analog_frequency2", _analogFrequency[2]);
        handler.item("analog_frequency3", _analogFrequency[3]);
        handler.item("digital0", _digitalOutput[0]);
        handler.item("digital1", _digitalOutput[1]);
        handler.item("digital2", _digitalOutput[2]);
        handler.item("digital3", _digitalOutput[3]);
    }
}
