#pragma once
// clang-format off

/*
    tapster_pro.h

    2020    - Bart Dring, Jason Huggins (Tapster Robotics)

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

#define MACHINE_NAME "Tapster Pro Delta (Trinamic)"

#define CUSTOM_CODE_FILENAME "Custom/parallel_delta.cpp"
/*
// enable these special machine functions to be called from the main program
#define USE_KINEMATICS             // there are kinematic equations for this machine
#define FWD_KINEMATICS_REPORTING   // report in cartesian
#define USE_RMT_STEPS              // Use the RMT periferal to generate step pulses
#define USE_TRINAMIC               // some Trinamic motors are used on this machine
#define USE_MACHINE_TRINAMIC_INIT  // there is a machine specific setup for the drivers
#define USE_MACHINE_INIT           // There is some custom initialization for this machine

#define SEGMENT_LENGTH 0.5  // segment length in mm
#define KIN_ANGLE_CALC_OK 0
#define KIN_ANGLE_ERROR -1

#define MAX_NEGATIVE_ANGLE -36  // in degrees how far can the arms go up?

#define HOMING_CURRENT_REDUCTION 1.0

*/

#define N_AXIS 3

#define USE_KINEMATICS      // there are kinematic equations for this machine
#define USE_FWD_KINEMATICS  // report in cartesian
#define USE_MACHINE_INIT    // There is some custom initialization for this machine

// ================== Delta Geometry ===========================

#define RADIUS_FIXED                100.0f          // radius of the fixed side (length of motor cranks)
#define RADIUS_EFF                  220.0f          // radius of end effector side (length of linkages)
#define LENGTH_FIXED_SIDE           294.449f        // sized of fixed side triangel
#define LENGTH_EFF_SIDE             86.6025f        // size of end effector side triangle
#define KINEMATIC_SEGMENT_LENGTH    1.0f            // segment length in mm
#define MAX_NEGATIVE_ANGLE          -0.75f          //
#define MAX_POSITIVE_ANGLE          (M_PI / 2.0)    //


// ================== Config ======================

// Set $Homing/Cycle0=XYZ

// ================== CPU MAP ======================

#define TRINAMIC_DAISY_CHAIN

#define TRINAMIC_RUN_MODE       TrinamicMode ::CoolStep
#define TRINAMIC_HOMING_MODE    TrinamicMode ::CoolStep

// Use SPI enable instead of the enable pin
// The hardware enable pin is tied to ground
#define USE_TRINAMIC_ENABLE

#define X_TRINAMIC_DRIVER       2130  // Which Driver Type?
#define X_RSENSE                TMC2130_RSENSE_DEFAULT
#define X_STEP_PIN              GPIO_NUM_12
#define X_DIRECTION_PIN         GPIO_NUM_14
#define X_CS_PIN                GPIO_NUM_17  // Daisy Chain, all share same CS pin

#define Y_TRINAMIC_DRIVER       2130  // Which Driver Type?
#define Y_RSENSE                TMC2130_RSENSE_DEFAULT
#define Y_STEP_PIN              GPIO_NUM_27
#define Y_DIRECTION_PIN         GPIO_NUM_26
#define Y_CS_PIN                X_CS_PIN  // Daisy Chain, all share same CS pin

#define Z_TRINAMIC_DRIVER       2130  // Which Driver Type?
#define Z_RSENSE                TMC2130_RSENSE_DEFAULT
#define Z_STEP_PIN              GPIO_NUM_15
#define Z_DIRECTION_PIN         GPIO_NUM_2
#define Z_CS_PIN                X_CS_PIN  // Daisy Chain, all share same CS pin

#define X_LIMIT_PIN     GPIO_NUM_36
#define Y_LIMIT_PIN     GPIO_NUM_39
#define Z_LIMIT_PIN     GPIO_NUM_34

// ================= defaults ===========================

#define DEFAULT_STEPPER_IDLE_LOCK_TIME 255  // keep them on, the trinamics will reduce power at idle

#define DEFAULT_X_MICROSTEPS    8
#define DEFAULT_Y_MICROSTEPS    DEFAULT_X_MICROSTEPS
#define DEFAULT_Z_MICROSTEPS    DEFAULT_X_MICROSTEPS

// some math to figure out microsteps per unit // units could bedegrees or radians
#define UNITS_PER_REV       (2.0 * M_PI)  // 360.0 degrees or 6.2831853 radians
#define STEPS_PER_REV       400.0
#define REDUCTION_RATIO     (60.0 / 16.0)  // the pulleys on arm and motor
#define MICROSTEPS_PER_REV  (STEPS_PER_REV * (float)DEFAULT_X_MICROSTEPS * REDUCTION_RATIO)

#define DEFAULT_X_STEPS_PER_MM  (MICROSTEPS_PER_REV / UNITS_PER_REV)
#define DEFAULT_Y_STEPS_PER_MM  DEFAULT_X_STEPS_PER_MM
#define DEFAULT_Z_STEPS_PER_MM  DEFAULT_X_STEPS_PER_MM

#define DEFAULT_X_MAX_RATE      100.0  // mm/min
#define DEFAULT_Y_MAX_RATE      DEFAULT_X_MAX_RATE
#define DEFAULT_Z_MAX_RATE      DEFAULT_X_MAX_RATE

#define DEFAULT_X_ACCELERATION  20.0
#define DEFAULT_Y_ACCELERATION  DEFAULT_X_ACCELERATION
#define DEFAULT_Z_ACCELERATION  DEFAULT_X_ACCELERATION

//  homing
#define DEFAULT_HOMING_FEED_RATE    25
#define DEFAULT_HOMING_SEEK_RATE    100
#define DEFAULT_HOMING_DIR_MASK     (bit(X_AXIS) | bit(Y_AXIS) | bit(Z_AXIS))  // all axes home negative
#define DEFAULT_HOMING_ENABLE       1
#define DEFAULT_INVERT_LIMIT_PINS   0

// The machine homes up and above center. MPos is the axis angle in radians
// at the homing posiiton

#define DEFAULT_X_HOMING_MPOS   -0.75  // neagtive because above horizontal
#define DEFAULT_Y_HOMING_MPOS   -0.75
#define DEFAULT_Z_HOMING_MPOS   -0.75

// the total travel is straight down from horizontal (pi/2) + the up travel
#define DEFAULT_X_MAX_TRAVEL    ((M_PI / 2.0) - DEFAULT_X_HOMING_MPOS)
#define DEFAULT_Y_MAX_TRAVEL    DEFAULT_X_MAX_TRAVEL
#define DEFAULT_Z_MAX_TRAVEL    DEFAULT_X_MAX_TRAVEL

#define DEFAULT_HOMING_PULLOFF  -DEFAULT_X_HOMING_MPOS

#define DEFAULT_X_CURRENT 1.0
#define DEFAULT_Y_CURRENT DEFAULT_X_CURRENT
#define DEFAULT_Z_CURRENT DEFAULT_X_CURRENT

#define DEFAULT_X_HOLD_CURRENT 0.25
#define DEFAULT_Y_HOLD_CURRENT DEFAULT_X_HOLD_CURRENT
#define DEFAULT_Z_HOLD_CURRENT DEFAULT_X_HOLD_CURRENT

#define SPINDLE_TYPE    SpindleType::NONE