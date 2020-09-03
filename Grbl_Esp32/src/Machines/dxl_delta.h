#pragma once
// clang-format off

/*
    dxl_delta.h

    2020    - Bart Dring

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
#define MACHINE_NAME            "Dynamixel Delta"

#define DYNAMIXEL_TXD           GPIO_NUM_4
#define DYNAMIXEL_RXD           GPIO_NUM_13
#define DYNAMIXEL_RTS           GPIO_NUM_17

#define SERVO_TIMER_INTerval    75   // milliseconds

#define X_DYNAMIXEL_ID          1 // protocol ID
#define Y_DYNAMIXEL_ID          2 // protocol ID
#define Z_DYNAMIXEL_ID          3 // protocol ID

// limit servo to 180 degree motion
#define DXL_COUNT_MIN 1024      
#define DXL_COUNT_MAX 3072

#define SPINDLE_TYPE    SPINDLE_TYPE_NONE
