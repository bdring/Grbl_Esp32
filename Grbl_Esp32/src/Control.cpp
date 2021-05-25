/*
  Control.cpp - input pins
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC

	2018 -	Bart Dring This file was modifed for use on the ESP32
					CPU. Do not use this with Grbl for atMega328P

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
#include "Control.h"

// TODO: the plumbing to communicate with System.cpp is not yet done.
// These variables are placeholders.
bool rtSafetyDoor;
bool rtReset;
bool rtFeedHold;
bool rtCycleStart;
bool rtButtonMacro0;
bool rtButtonMacro1;
bool rtButtonMacro2;
bool rtButtonMacro3;

Control::Control() :
    _safetyDoor(0, &rtSafetyDoor, "Door", 'D'), _reset(1, &rtReset, "Reset", 'R'), _feedHold(2, &rtFeedHold, "FeedHold", 'H'),
    _cycleStart(3, &rtCycleStart, "CycleStart", 'S'), _macro0(4, &rtButtonMacro0, "Macro 0", '0'),
    _macro1(5, &rtButtonMacro1, "Macro 1", '1'), _macro2(6, &rtButtonMacro2, "Macro 2", '2'), _macro3(7, &rtButtonMacro3, "Macro 3", '3') {}

void Control::init() {
    _safetyDoor.init();
    _reset.init();
    _feedHold.init();
    _cycleStart.init();
    _macro0.init();
    _macro1.init();
    _macro2.init();
    _macro3.init();
}

void Control::validate() const {}

void Control::handle(Configuration::HandlerBase& handler) {
    handler.handle("safetyDoor", _safetyDoor._pin);
    handler.handle("reset", _reset._pin);
    handler.handle("feed_hold", _feedHold._pin);
    handler.handle("cycle_start", _cycleStart._pin);
    handler.handle("macro0", _macro0._pin);
    handler.handle("macro1", _macro1._pin);
    handler.handle("macro2", _macro2._pin);
    handler.handle("macro3", _macro3._pin);
}
#if 0
// Returns if safety door is ajar(T) or closed(F), based on pin state.
bool system_check_safety_door_ajar() {
    // If a safety door pin is not defined, this will return false
    // because that is the default for the value field, which will
    // never be changed for an undefined pin.
    return _safetyDoor.get();
}
#endif
