#pragma once
// clang-format off

/*
    6_pack_external_XY_2130_Z_1SW_XYX_2RelayMist_3RelayFlood_4RelayUser.h

    Covers all V1 versions V1p0, V1p1, etc

    Part of Grbl_ESP32
    Pin assignments for the ESP32 I2S 6-axis board
   
    2020-10-05 B. Dring for David K.

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
#define MACHINE_NAME            "6 Pack StepStick XYYZ SW_XYZP"

#define N_AXIS 6



#define X_SERVO_PIN             GPIO_NUM_2 // RC Servo
#define X_SERVO_CAL_MIN         1.0       // RC Servo calibration factor for the minimum PWM duty
#define X_SERVO_CAL_MAX         1.0       // RC Servo calibration factor for the maximum PWM duty

#define Y_SERVO_PIN             GPIO_NUM_25 // RC Servo
#define Y_SERVO_CAL_MIN         1.0       // RC Servo calibration factor for the minimum PWM duty
#define Y_SERVO_CAL_MAX         1.0       // RC Servo calibration factor for the maximum PWM duty

#define Z_SERVO_PIN             GPIO_NUM_14 // RC Servo
#define Z_SERVO_CAL_MIN         1.0       // RC Servo calibration factor for the minimum PWM duty
#define Z_SERVO_CAL_MAX         1.0       // RC Servo calibration factor for the maximum PWM duty

#define A_SERVO_PIN             GPIO_NUM_13 // RC Servo
#define A_SERVO_CAL_MIN         1.0       // RC Servo calibration factor for the minimum PWM duty
#define A_SERVO_CAL_MAX         1.0       // RC Servo calibration factor for the maximum PWM duty

#define B_SERVO_PIN             GPIO_NUM_15 // RC Servo
#define B_SERVO_CAL_MIN         1.0       // RC Servo calibration factor for the minimum PWM duty
#define B_SERVO_CAL_MAX         1.0       // RC Servo calibration factor for the maximum PWM duty

#define C_SERVO_PIN             GPIO_NUM_12 // RC Servo
#define C_SERVO_CAL_MIN         1.0       // RC Servo calibration factor for the minimum PWM duty
#define C_SERVO_CAL_MAX         1.0       // RC Servo calibration factor for the maximum PWM duty
