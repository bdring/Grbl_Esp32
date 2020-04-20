/*
    spindle_test.h
    Part of Grbl_ESP32

    Pin assignments (or lack thereof) for testing Grbl_ESP32.

    This is just a minimal machine definition to test Spindle Classes

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

#define MACHINE_NAME "Spindle Class Testing"


#define SPINDLE_TYPE       SPINDLE_TYPE_PWM
//#define SPINDLE_TYPE       SPINDLE_TYPE_BESC

#define SPINDLE_OUTPUT_PIN    GPIO_NUM_27


#define LIMIT_MASK 0  // no limit pins

#ifdef USE_RMT_STEPS
#undef USE_RMT_STEPS  // Suppress unused variable warning
#endif
