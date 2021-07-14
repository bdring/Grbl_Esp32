#include "Endstops.h"

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

namespace Machine {

    void Endstops::validate() const {
        //    if (_dual.defined()) {
        //        Assert(_positive.undefined(), "If dual endstops are defined, you cannot also define positive and negative endstops");
        //        Assert(_negative.undefined(), "If dual endstops are defined, you cannot also define positive and negative endstops");
        //    }
        //    if (_positive.defined() || _negative.defined()) {
        //        Assert(_positive.undefined(), "If positive or negative endstops are defined, you cannot also define dual endstops");
        //    }
    }

    // return true if any switch is active
    // TODO: Would be handy to return some sort of mask of which switch is active
    // or have a parameter to read all or a specific switch
    bool Endstops::read() {
        if (_limitAll.defined()) {
            if (_limitAll.read())
                return true;
        }

        if (_limitNeg.defined()) {
            if (_limitNeg.read())
                return true;
        }

        if (_limitPos.defined()) {
            if (_limitPos.read())
                return true;
        }

        return false;
    }

    void Endstops::group(Configuration::HandlerBase& handler) {
        handler.item("limit_neg", _limitNeg);
        handler.item("limit_pos", _limitPos);
        handler.item("limit_all", _limitAll);
        handler.item("hard_limits", _hardLimits);
    }
}
