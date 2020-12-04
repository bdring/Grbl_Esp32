#pragma once
// clang-format off

/*
    test_drive.h
    Part of Grbl_ESP32

    Pin assignments (or lack thereof) for testing Grbl_ESP32.

    It creates a basic 3 axis machine without actually driving
    I/O pins.  Grbl will report that axes are moving, but no physical
    motor motion will occur.

    This can be uploaded to an unattached ESP32 or attached to
    unknown hardware with no risk of pins trying to output signals
    into a short, etc that could dmamge the ESP32

    It can also be used to get the basic program running so OTA
    (over the air) firmware loading can be done.

    2018    - Bart Dring
    2020    - Mitch Bradley

    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Grbl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/

#define MACHINE_NAME "Test Drive - Demo Only No I/O!"


#define N_AXIS 3
// This cannot use homing because there are no switches
#ifdef DEFAULT_HOMING_CYCLE_0
    #undef DEFAULT_HOMING_CYCLE_0
#endif

#ifdef DEFAULT_HOMING_CYCLE_1
    #undef DEFAULT_HOMING_CYCLE_1
#endif

#define SPINDLE_TYPE    SpindleType::NONE

#ifdef USE_RMT_STEPS
#undef USE_RMT_STEPS  // Suppress unused variable warning
#endif
