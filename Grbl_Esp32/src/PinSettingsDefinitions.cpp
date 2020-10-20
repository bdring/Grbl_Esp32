// This file attempts to build all the pin settings definitions, while retaining backward compatibility with the
// original machine files.
//
// Note that the source code shouldn't use #define'd pins directly, but always use the settings in the code.
// To aid that, I do NOT want to include the default <Arduino.h> from this file (indirectly). Instead, this
// file defines the GPIO_NUM_* and I2SO(*) as macro's to strings that the Pin class understands, and uses that.
//
// To make sure we don't accidentally ripple macro's some place they don't belong, I tried to do this as isolated
// as possible.
//
// NOTE: The order in which defines and includes are used here matters!
//
// All the preprocessor things are handled first, followed by the real code.

// First do the defines that take care of GPIO pin name mapping:

#include <cstdint>

#define GPIO_NUM_0 "GPIO.0"
#define GPIO_NUM_1 "GPIO.1"
#define GPIO_NUM_2 "GPIO.2"
#define GPIO_NUM_3 "GPIO.3"
#define GPIO_NUM_4 "GPIO.4"
#define GPIO_NUM_5 "GPIO.5"
#define GPIO_NUM_6 "GPIO.6"
#define GPIO_NUM_7 "GPIO.7"
#define GPIO_NUM_8 "GPIO.8"
#define GPIO_NUM_9 "GPIO.9"
#define GPIO_NUM_10 "GPIO.10"
#define GPIO_NUM_11 "GPIO.11"
#define GPIO_NUM_12 "GPIO.12"
#define GPIO_NUM_13 "GPIO.13"
#define GPIO_NUM_14 "GPIO.14"
#define GPIO_NUM_15 "GPIO.15"
#define GPIO_NUM_16 "GPIO.16"
#define GPIO_NUM_17 "GPIO.17"
#define GPIO_NUM_18 "GPIO.18"
#define GPIO_NUM_19 "GPIO.19"

#define GPIO_NUM_21 "GPIO.21"
#define GPIO_NUM_22 "GPIO.22"
#define GPIO_NUM_23 "GPIO.23"

#define GPIO_NUM_25 "GPIO.25"
#define GPIO_NUM_26 "GPIO.26"
#define GPIO_NUM_27 "GPIO.27"

#define GPIO_NUM_32 "GPIO.32"
#define GPIO_NUM_33 "GPIO.33"
#define GPIO_NUM_34 "GPIO.34"
#define GPIO_NUM_35 "GPIO.35"
#define GPIO_NUM_36 "GPIO.36"
#define GPIO_NUM_37 "GPIO.37"
#define GPIO_NUM_38 "GPIO.38"
#define GPIO_NUM_39 "GPIO.39"

#define UNDEFINED_PIN "undef"

#define I2SO(n) "I2S." #n

// Include the file that loads the machine-specific config file.
// machine.h must be edited to choose the desired file.
#include "Machine.h"

// machine_common.h contains settings that do not change
#include "MachineCommon.h"

// Update missing definitions with defaults:
#include "Defaults.h"

// Set defaults to all the macro's:
#ifndef COOLANT_FLOOD_PIN
#    define COOLANT_FLOOD_PIN "undef"
#endif
const char* COOLANT_FLOOD_PIN_DEFAULT = COOLANT_FLOOD_PIN;
#ifndef COOLANT_MIST_PIN
#    define COOLANT_MIST_PIN "undef"
#endif
const char* COOLANT_MIST_PIN_DEFAULT = COOLANT_MIST_PIN;
#ifndef PROBE_PIN
#    define PROBE_PIN "undef"
#endif
const char* PROBE_PIN_DEFAULT = PROBE_PIN;
#ifndef SDCARD_DET_PIN
#    define SDCARD_DET_PIN "undef"
#endif
const char* SDCARD_DET_PIN_DEFAULT = SDCARD_DET_PIN;
#ifndef STEPPERS_DISABLE_PIN
#    define STEPPERS_DISABLE_PIN "undef"
#endif
const char* STEPPERS_DISABLE_PIN_DEFAULT = STEPPERS_DISABLE_PIN;
#ifndef STEPPER_RESET
#    define STEPPER_RESET "undef"
#endif
const char* STEPPER_RESET_DEFAULT = STEPPER_RESET;
#ifndef CONTROL_SAFETY_DOOR_PIN
#    define CONTROL_SAFETY_DOOR_PIN "undef"
#endif
const char* CONTROL_SAFETY_DOOR_PIN_DEFAULT = CONTROL_SAFETY_DOOR_PIN;
#ifndef CONTROL_RESET_PIN
#    define CONTROL_RESET_PIN "undef"
#endif
const char* CONTROL_RESET_PIN_DEFAULT = CONTROL_RESET_PIN;
#ifndef CONTROL_FEED_HOLD_PIN
#    define CONTROL_FEED_HOLD_PIN "undef"
#endif
const char* CONTROL_FEED_HOLD_PIN_DEFAULT = CONTROL_FEED_HOLD_PIN;
#ifndef CONTROL_CYCLE_START_PIN
#    define CONTROL_CYCLE_START_PIN "undef"
#endif
const char* CONTROL_CYCLE_START_PIN_DEFAULT = CONTROL_CYCLE_START_PIN;
#ifndef MACRO_BUTTON_0_PIN
#    define MACRO_BUTTON_0_PIN "undef"
#endif
const char* MACRO_BUTTON_0_PIN_DEFAULT = MACRO_BUTTON_0_PIN;
#ifndef MACRO_BUTTON_1_PIN
#    define MACRO_BUTTON_1_PIN "undef"
#endif
const char* MACRO_BUTTON_1_PIN_DEFAULT = MACRO_BUTTON_1_PIN;
#ifndef MACRO_BUTTON_2_PIN
#    define MACRO_BUTTON_2_PIN "undef"
#endif
const char* MACRO_BUTTON_2_PIN_DEFAULT = MACRO_BUTTON_2_PIN;
#ifndef MACRO_BUTTON_3_PIN
#    define MACRO_BUTTON_3_PIN "undef"
#endif
const char* MACRO_BUTTON_3_PIN_DEFAULT = MACRO_BUTTON_3_PIN;
#ifndef DYNAMIXEL_TXD
#    define DYNAMIXEL_TXD "undef"
#endif
const char* DYNAMIXEL_TXD_DEFAULT = DYNAMIXEL_TXD;
#ifndef DYNAMIXEL_RXD
#    define DYNAMIXEL_RXD "undef"
#endif
const char* DYNAMIXEL_RXD_DEFAULT = DYNAMIXEL_RXD;
#ifndef DYNAMIXEL_RTS
#    define DYNAMIXEL_RTS "undef"
#endif
const char* DYNAMIXEL_RTS_DEFAULT = DYNAMIXEL_RTS;

#ifndef X_LIMIT_PIN
#    define X_LIMIT_PIN "undef"
#endif
const char* X_LIMIT_PIN_DEFAULT = X_LIMIT_PIN;
#ifndef X2_LIMIT_PIN
#    define X2_LIMIT_PIN "undef"
#endif
const char* X2_LIMIT_PIN_DEFAULT = X2_LIMIT_PIN;
#ifndef Y_LIMIT_PIN
#    define Y_LIMIT_PIN "undef"
#endif
const char* Y_LIMIT_PIN_DEFAULT = Y_LIMIT_PIN;
#ifndef Y2_LIMIT_PIN
#    define Y2_LIMIT_PIN "undef"
#endif
const char* Y2_LIMIT_PIN_DEFAULT = Y2_LIMIT_PIN;
#ifndef Z_LIMIT_PIN
#    define Z_LIMIT_PIN "undef"
#endif
const char* Z_LIMIT_PIN_DEFAULT = Z_LIMIT_PIN;
#ifndef Z2_LIMIT_PIN
#    define Z2_LIMIT_PIN "undef"
#endif
const char* Z2_LIMIT_PIN_DEFAULT = Z2_LIMIT_PIN;
#ifndef A_LIMIT_PIN
#    define A_LIMIT_PIN "undef"
#endif
const char* A_LIMIT_PIN_DEFAULT = A_LIMIT_PIN;
#ifndef A2_LIMIT_PIN
#    define A2_LIMIT_PIN "undef"
#endif
const char* A2_LIMIT_PIN_DEFAULT = A2_LIMIT_PIN;
#ifndef B_LIMIT_PIN
#    define B_LIMIT_PIN "undef"
#endif
const char* B_LIMIT_PIN_DEFAULT = B_LIMIT_PIN;
#ifndef B2_LIMIT_PIN
#    define B2_LIMIT_PIN "undef"
#endif
const char* B2_LIMIT_PIN_DEFAULT = B2_LIMIT_PIN;
#ifndef C_LIMIT_PIN
#    define C_LIMIT_PIN "undef"
#endif
const char* C_LIMIT_PIN_DEFAULT = C_LIMIT_PIN;
#ifndef C2_LIMIT_PIN
#    define C2_LIMIT_PIN "undef"
#endif
const char* C2_LIMIT_PIN_DEFAULT = C2_LIMIT_PIN;

#ifndef X_STEP_PIN
#    define X_STEP_PIN "undef"
#endif
const char* X_STEP_PIN_DEFAULT = X_STEP_PIN;
#ifndef X2_STEP_PIN
#    define X2_STEP_PIN "undef"
#endif
const char* X2_STEP_PIN_DEFAULT = X2_STEP_PIN;
#ifndef Y_STEP_PIN
#    define Y_STEP_PIN "undef"
#endif
const char* Y_STEP_PIN_DEFAULT = Y_STEP_PIN;
#ifndef Y2_STEP_PIN
#    define Y2_STEP_PIN "undef"
#endif
const char* Y2_STEP_PIN_DEFAULT = Y2_STEP_PIN;
#ifndef Z_STEP_PIN
#    define Z_STEP_PIN "undef"
#endif
const char* Z_STEP_PIN_DEFAULT = Z_STEP_PIN;
#ifndef Z2_STEP_PIN
#    define Z2_STEP_PIN "undef"
#endif
const char* Z2_STEP_PIN_DEFAULT = Z2_STEP_PIN;
#ifndef A_STEP_PIN
#    define A_STEP_PIN "undef"
#endif
const char* A_STEP_PIN_DEFAULT = A_STEP_PIN;
#ifndef A2_STEP_PIN
#    define A2_STEP_PIN "undef"
#endif
const char* A2_STEP_PIN_DEFAULT = A2_STEP_PIN;
#ifndef B_STEP_PIN
#    define B_STEP_PIN "undef"
#endif
const char* B_STEP_PIN_DEFAULT = B_STEP_PIN;
#ifndef B2_STEP_PIN
#    define B2_STEP_PIN "undef"
#endif
const char* B2_STEP_PIN_DEFAULT = B2_STEP_PIN;
#ifndef C_STEP_PIN
#    define C_STEP_PIN "undef"
#endif
const char* C_STEP_PIN_DEFAULT = C_STEP_PIN;
#ifndef C2_STEP_PIN
#    define C2_STEP_PIN "undef"
#endif
const char* C2_STEP_PIN_DEFAULT = C2_STEP_PIN;

#ifndef X_DIRECTION_PIN
#    define X_DIRECTION_PIN "undef"
#endif
const char* X_DIRECTION_PIN_DEFAULT = X_DIRECTION_PIN;
#ifndef X2_DIRECTION_PIN
#    define X2_DIRECTION_PIN "undef"
#endif
const char* X2_DIRECTION_PIN_DEFAULT = X2_DIRECTION_PIN;
#ifndef Y_DIRECTION_PIN
#    define Y_DIRECTION_PIN "undef"
#endif
const char* Y_DIRECTION_PIN_DEFAULT = Y_DIRECTION_PIN;
#ifndef Y2_DIRECTION_PIN
#    define Y2_DIRECTION_PIN "undef"
#endif
const char* Y2_DIRECTION_PIN_DEFAULT = Y2_DIRECTION_PIN;
#ifndef Z_DIRECTION_PIN
#    define Z_DIRECTION_PIN "undef"
#endif
const char* Z_DIRECTION_PIN_DEFAULT = Z_DIRECTION_PIN;
#ifndef Z2_DIRECTION_PIN
#    define Z2_DIRECTION_PIN "undef"
#endif
const char* Z2_DIRECTION_PIN_DEFAULT = Z2_DIRECTION_PIN;
#ifndef A_DIRECTION_PIN
#    define A_DIRECTION_PIN "undef"
#endif
const char* A_DIRECTION_PIN_DEFAULT = A_DIRECTION_PIN;
#ifndef A2_DIRECTION_PIN
#    define A2_DIRECTION_PIN "undef"
#endif
const char* A2_DIRECTION_PIN_DEFAULT = A2_DIRECTION_PIN;
#ifndef B_DIRECTION_PIN
#    define B_DIRECTION_PIN "undef"
#endif
const char* B_DIRECTION_PIN_DEFAULT = B_DIRECTION_PIN;
#ifndef B2_DIRECTION_PIN
#    define B2_DIRECTION_PIN "undef"
#endif
const char* B2_DIRECTION_PIN_DEFAULT = B2_DIRECTION_PIN;
#ifndef C_DIRECTION_PIN
#    define C_DIRECTION_PIN "undef"
#endif
const char* C_DIRECTION_PIN_DEFAULT = C_DIRECTION_PIN;
#ifndef C2_DIRECTION_PIN
#    define C2_DIRECTION_PIN "undef"
#endif
const char* C2_DIRECTION_PIN_DEFAULT = C2_DIRECTION_PIN;

#ifndef X_DISABLE_PIN
#    define X_DISABLE_PIN "undef"
#endif
const char* X_DISABLE_PIN_DEFAULT = X_DISABLE_PIN;
#ifndef X2_DISABLE_PIN
#    define X2_DISABLE_PIN "undef"
#endif
const char* X2_DISABLE_PIN_DEFAULT = X2_DISABLE_PIN;
#ifndef Y_DISABLE_PIN
#    define Y_DISABLE_PIN "undef"
#endif
const char* Y_DISABLE_PIN_DEFAULT = Y_DISABLE_PIN;
#ifndef Y2_DISABLE_PIN
#    define Y2_DISABLE_PIN "undef"
#endif
const char* Y2_DISABLE_PIN_DEFAULT = Y2_DISABLE_PIN;
#ifndef Z_DISABLE_PIN
#    define Z_DISABLE_PIN "undef"
#endif
const char* Z_DISABLE_PIN_DEFAULT = Z_DISABLE_PIN;
#ifndef Z2_DISABLE_PIN
#    define Z2_DISABLE_PIN "undef"
#endif
const char* Z2_DISABLE_PIN_DEFAULT = Z2_DISABLE_PIN;
#ifndef A_DISABLE_PIN
#    define A_DISABLE_PIN "undef"
#endif
const char* A_DISABLE_PIN_DEFAULT = A_DISABLE_PIN;
#ifndef A2_DISABLE_PIN
#    define A2_DISABLE_PIN "undef"
#endif
const char* A2_DISABLE_PIN_DEFAULT = A2_DISABLE_PIN;
#ifndef B_DISABLE_PIN
#    define B_DISABLE_PIN "undef"
#endif
const char* B_DISABLE_PIN_DEFAULT = B_DISABLE_PIN;
#ifndef B2_DISABLE_PIN
#    define B2_DISABLE_PIN "undef"
#endif
const char* B2_DISABLE_PIN_DEFAULT = B2_DISABLE_PIN;
#ifndef C_DISABLE_PIN
#    define C_DISABLE_PIN "undef"
#endif
const char* C_DISABLE_PIN_DEFAULT = C_DISABLE_PIN;
#ifndef C2_DISABLE_PIN
#    define C2_DISABLE_PIN "undef"
#endif
const char* C2_DISABLE_PIN_DEFAULT = C2_DISABLE_PIN;

#ifndef X_CS_PIN
#    define X_CS_PIN "undef"
#endif
const char* X_CS_PIN_DEFAULT = X_CS_PIN;
#ifndef X2_CS_PIN
#    define X2_CS_PIN "undef"
#endif
const char* X2_CS_PIN_DEFAULT = X2_CS_PIN;
#ifndef Y_CS_PIN
#    define Y_CS_PIN "undef"
#endif
const char* Y_CS_PIN_DEFAULT = Y_CS_PIN;
#ifndef Y2_CS_PIN
#    define Y2_CS_PIN "undef"
#endif
const char* Y2_CS_PIN_DEFAULT = Y2_CS_PIN;
#ifndef Z_CS_PIN
#    define Z_CS_PIN "undef"
#endif
const char* Z_CS_PIN_DEFAULT = Z_CS_PIN;
#ifndef Z2_CS_PIN
#    define Z2_CS_PIN "undef"
#endif
const char* Z2_CS_PIN_DEFAULT = Z2_CS_PIN;
#ifndef A_CS_PIN
#    define A_CS_PIN "undef"
#endif
const char* A_CS_PIN_DEFAULT = A_CS_PIN;
#ifndef A2_CS_PIN
#    define A2_CS_PIN "undef"
#endif
const char* A2_CS_PIN_DEFAULT = A2_CS_PIN;
#ifndef B_CS_PIN
#    define B_CS_PIN "undef"
#endif
const char* B_CS_PIN_DEFAULT = B_CS_PIN;
#ifndef B2_CS_PIN
#    define B2_CS_PIN "undef"
#endif
const char* B2_CS_PIN_DEFAULT = B2_CS_PIN;
#ifndef C_CS_PIN
#    define C_CS_PIN "undef"
#endif
const char* C_CS_PIN_DEFAULT = C_CS_PIN;
#ifndef C2_CS_PIN
#    define C2_CS_PIN "undef"
#endif
const char* C2_CS_PIN_DEFAULT = C2_CS_PIN;

#ifndef X_SERVO_PIN
#    define X_SERVO_PIN "undef"
#endif
const char* X_SERVO_PIN_DEFAULT = X_SERVO_PIN;
#ifndef X2_SERVO_PIN
#    define X2_SERVO_PIN "undef"
#endif
const char* X2_SERVO_PIN_DEFAULT = X2_SERVO_PIN;
#ifndef Y_SERVO_PIN
#    define Y_SERVO_PIN "undef"
#endif
const char* Y_SERVO_PIN_DEFAULT = Y_SERVO_PIN;
#ifndef Y2_SERVO_PIN
#    define Y2_SERVO_PIN "undef"
#endif
const char* Y2_SERVO_PIN_DEFAULT = Y2_SERVO_PIN;
#ifndef Z_SERVO_PIN
#    define Z_SERVO_PIN "undef"
#endif
const char* Z_SERVO_PIN_DEFAULT = Z_SERVO_PIN;
#ifndef Z2_SERVO_PIN
#    define Z2_SERVO_PIN "undef"
#endif
const char* Z2_SERVO_PIN_DEFAULT = Z2_SERVO_PIN;
#ifndef A_SERVO_PIN
#    define A_SERVO_PIN "undef"
#endif
const char* A_SERVO_PIN_DEFAULT = A_SERVO_PIN;
#ifndef A2_SERVO_PIN
#    define A2_SERVO_PIN "undef"
#endif
const char* A2_SERVO_PIN_DEFAULT = A2_SERVO_PIN;
#ifndef B_SERVO_PIN
#    define B_SERVO_PIN "undef"
#endif
const char* B_SERVO_PIN_DEFAULT = B_SERVO_PIN;
#ifndef B2_SERVO_PIN
#    define B2_SERVO_PIN "undef"
#endif
const char* B2_SERVO_PIN_DEFAULT = B2_SERVO_PIN;
#ifndef C_SERVO_PIN
#    define C_SERVO_PIN "undef"
#endif
const char* C_SERVO_PIN_DEFAULT = C_SERVO_PIN;
#ifndef C2_SERVO_PIN
#    define C2_SERVO_PIN "undef"
#endif
const char* C2_SERVO_PIN_DEFAULT = C2_SERVO_PIN;

#ifndef X_STEPPER_MS3
#    define X_STEPPER_MS3 "undef"
#endif
const char* X_STEPPER_MS3_DEFAULT = X_STEPPER_MS3;
#ifndef X2_STEPPER_MS3
#    define X2_STEPPER_MS3 "undef"
#endif
const char* X2_STEPPER_MS3_DEFAULT = X2_STEPPER_MS3;
#ifndef Y_STEPPER_MS3
#    define Y_STEPPER_MS3 "undef"
#endif
const char* Y_STEPPER_MS3_DEFAULT = Y_STEPPER_MS3;
#ifndef Y2_STEPPER_MS3
#    define Y2_STEPPER_MS3 "undef"
#endif
const char* Y2_STEPPER_MS3_DEFAULT = Y2_STEPPER_MS3;
#ifndef Z_STEPPER_MS3
#    define Z_STEPPER_MS3 "undef"
#endif
const char* Z_STEPPER_MS3_DEFAULT = Z_STEPPER_MS3;
#ifndef Z2_STEPPER_MS3
#    define Z2_STEPPER_MS3 "undef"
#endif
const char* Z2_STEPPER_MS3_DEFAULT = Z2_STEPPER_MS3;
#ifndef A_STEPPER_MS3
#    define A_STEPPER_MS3 "undef"
#endif
const char* A_STEPPER_MS3_DEFAULT = A_STEPPER_MS3;
#ifndef A2_STEPPER_MS3
#    define A2_STEPPER_MS3 "undef"
#endif
const char* A2_STEPPER_MS3_DEFAULT = A2_STEPPER_MS3;
#ifndef B_STEPPER_MS3
#    define B_STEPPER_MS3 "undef"
#endif
const char* B_STEPPER_MS3_DEFAULT = B_STEPPER_MS3;
#ifndef B2_STEPPER_MS3
#    define B2_STEPPER_MS3 "undef"
#endif
const char* B2_STEPPER_MS3_DEFAULT = B2_STEPPER_MS3;
#ifndef C_STEPPER_MS3
#    define C_STEPPER_MS3 "undef"
#endif
const char* C_STEPPER_MS3_DEFAULT = C_STEPPER_MS3;
#ifndef C2_STEPPER_MS3
#    define C2_STEPPER_MS3 "undef"
#endif
const char* C2_STEPPER_MS3_DEFAULT = C2_STEPPER_MS3;

#ifndef X_PIN_PHASE_0
#    define X_PIN_PHASE_0 "undef"
#endif
const char* X_PIN_PHASE_0_DEFAULT = X_PIN_PHASE_0;
#ifndef X2_PIN_PHASE_0
#    define X2_PIN_PHASE_0 "undef"
#endif
const char* X2_PIN_PHASE_0_DEFAULT = X2_PIN_PHASE_0;
#ifndef Y_PIN_PHASE_0
#    define Y_PIN_PHASE_0 "undef"
#endif
const char* Y_PIN_PHASE_0_DEFAULT = Y_PIN_PHASE_0;
#ifndef Y2_PIN_PHASE_0
#    define Y2_PIN_PHASE_0 "undef"
#endif
const char* Y2_PIN_PHASE_0_DEFAULT = Y2_PIN_PHASE_0;
#ifndef Z_PIN_PHASE_0
#    define Z_PIN_PHASE_0 "undef"
#endif
const char* Z_PIN_PHASE_0_DEFAULT = Z_PIN_PHASE_0;
#ifndef Z2_PIN_PHASE_0
#    define Z2_PIN_PHASE_0 "undef"
#endif
const char* Z2_PIN_PHASE_0_DEFAULT = Z2_PIN_PHASE_0;
#ifndef A_PIN_PHASE_0
#    define A_PIN_PHASE_0 "undef"
#endif
const char* A_PIN_PHASE_0_DEFAULT = A_PIN_PHASE_0;
#ifndef A2_PIN_PHASE_0
#    define A2_PIN_PHASE_0 "undef"
#endif
const char* A2_PIN_PHASE_0_DEFAULT = A2_PIN_PHASE_0;
#ifndef B_PIN_PHASE_0
#    define B_PIN_PHASE_0 "undef"
#endif
const char* B_PIN_PHASE_0_DEFAULT = B_PIN_PHASE_0;
#ifndef B2_PIN_PHASE_0
#    define B2_PIN_PHASE_0 "undef"
#endif
const char* B2_PIN_PHASE_0_DEFAULT = B2_PIN_PHASE_0;
#ifndef C_PIN_PHASE_0
#    define C_PIN_PHASE_0 "undef"
#endif
const char* C_PIN_PHASE_0_DEFAULT = C_PIN_PHASE_0;
#ifndef C2_PIN_PHASE_0
#    define C2_PIN_PHASE_0 "undef"
#endif
const char* C2_PIN_PHASE_0_DEFAULT = C2_PIN_PHASE_0;

#ifndef X_PIN_PHASE_1
#    define X_PIN_PHASE_1 "undef"
#endif
const char* X_PIN_PHASE_1_DEFAULT = X_PIN_PHASE_1;
#ifndef X2_PIN_PHASE_1
#    define X2_PIN_PHASE_1 "undef"
#endif
const char* X2_PIN_PHASE_1_DEFAULT = X2_PIN_PHASE_1;
#ifndef Y_PIN_PHASE_1
#    define Y_PIN_PHASE_1 "undef"
#endif
const char* Y_PIN_PHASE_1_DEFAULT = Y_PIN_PHASE_1;
#ifndef Y2_PIN_PHASE_1
#    define Y2_PIN_PHASE_1 "undef"
#endif
const char* Y2_PIN_PHASE_1_DEFAULT = Y2_PIN_PHASE_1;
#ifndef Z_PIN_PHASE_1
#    define Z_PIN_PHASE_1 "undef"
#endif
const char* Z_PIN_PHASE_1_DEFAULT = Z_PIN_PHASE_1;
#ifndef Z2_PIN_PHASE_1
#    define Z2_PIN_PHASE_1 "undef"
#endif
const char* Z2_PIN_PHASE_1_DEFAULT = Z2_PIN_PHASE_1;
#ifndef A_PIN_PHASE_1
#    define A_PIN_PHASE_1 "undef"
#endif
const char* A_PIN_PHASE_1_DEFAULT = A_PIN_PHASE_1;
#ifndef A2_PIN_PHASE_1
#    define A2_PIN_PHASE_1 "undef"
#endif
const char* A2_PIN_PHASE_1_DEFAULT = A2_PIN_PHASE_1;
#ifndef B_PIN_PHASE_1
#    define B_PIN_PHASE_1 "undef"
#endif
const char* B_PIN_PHASE_1_DEFAULT = B_PIN_PHASE_1;
#ifndef B2_PIN_PHASE_1
#    define B2_PIN_PHASE_1 "undef"
#endif
const char* B2_PIN_PHASE_1_DEFAULT = B2_PIN_PHASE_1;
#ifndef C_PIN_PHASE_1
#    define C_PIN_PHASE_1 "undef"
#endif
const char* C_PIN_PHASE_1_DEFAULT = C_PIN_PHASE_1;
#ifndef C2_PIN_PHASE_1
#    define C2_PIN_PHASE_1 "undef"
#endif
const char* C2_PIN_PHASE_1_DEFAULT = C2_PIN_PHASE_1;

#ifndef X_PIN_PHASE_2
#    define X_PIN_PHASE_2 "undef"
#endif
const char* X_PIN_PHASE_2_DEFAULT = X_PIN_PHASE_2;
#ifndef X2_PIN_PHASE_2
#    define X2_PIN_PHASE_2 "undef"
#endif
const char* X2_PIN_PHASE_2_DEFAULT = X2_PIN_PHASE_2;
#ifndef Y_PIN_PHASE_2
#    define Y_PIN_PHASE_2 "undef"
#endif
const char* Y_PIN_PHASE_2_DEFAULT = Y_PIN_PHASE_2;
#ifndef Y2_PIN_PHASE_2
#    define Y2_PIN_PHASE_2 "undef"
#endif
const char* Y2_PIN_PHASE_2_DEFAULT = Y2_PIN_PHASE_2;
#ifndef Z_PIN_PHASE_2
#    define Z_PIN_PHASE_2 "undef"
#endif
const char* Z_PIN_PHASE_2_DEFAULT = Z_PIN_PHASE_2;
#ifndef Z2_PIN_PHASE_2
#    define Z2_PIN_PHASE_2 "undef"
#endif
const char* Z2_PIN_PHASE_2_DEFAULT = Z2_PIN_PHASE_2;
#ifndef A_PIN_PHASE_2
#    define A_PIN_PHASE_2 "undef"
#endif
const char* A_PIN_PHASE_2_DEFAULT = A_PIN_PHASE_2;
#ifndef A2_PIN_PHASE_2
#    define A2_PIN_PHASE_2 "undef"
#endif
const char* A2_PIN_PHASE_2_DEFAULT = A2_PIN_PHASE_2;
#ifndef B_PIN_PHASE_2
#    define B_PIN_PHASE_2 "undef"
#endif
const char* B_PIN_PHASE_2_DEFAULT = B_PIN_PHASE_2;
#ifndef B2_PIN_PHASE_2
#    define B2_PIN_PHASE_2 "undef"
#endif
const char* B2_PIN_PHASE_2_DEFAULT = B2_PIN_PHASE_2;
#ifndef C_PIN_PHASE_2
#    define C_PIN_PHASE_2 "undef"
#endif
const char* C_PIN_PHASE_2_DEFAULT = C_PIN_PHASE_2;
#ifndef C2_PIN_PHASE_2
#    define C2_PIN_PHASE_2 "undef"
#endif
const char* C2_PIN_PHASE_2_DEFAULT = C2_PIN_PHASE_2;

#ifndef X_PIN_PHASE_3
#    define X_PIN_PHASE_3 "undef"
#endif
const char* X_PIN_PHASE_3_DEFAULT = X_PIN_PHASE_3;
#ifndef X2_PIN_PHASE_3
#    define X2_PIN_PHASE_3 "undef"
#endif
const char* X2_PIN_PHASE_3_DEFAULT = X2_PIN_PHASE_3;
#ifndef Y_PIN_PHASE_3
#    define Y_PIN_PHASE_3 "undef"
#endif
const char* Y_PIN_PHASE_3_DEFAULT = Y_PIN_PHASE_3;
#ifndef Y2_PIN_PHASE_3
#    define Y2_PIN_PHASE_3 "undef"
#endif
const char* Y2_PIN_PHASE_3_DEFAULT = Y2_PIN_PHASE_3;
#ifndef Z_PIN_PHASE_3
#    define Z_PIN_PHASE_3 "undef"
#endif
const char* Z_PIN_PHASE_3_DEFAULT = Z_PIN_PHASE_3;
#ifndef Z2_PIN_PHASE_3
#    define Z2_PIN_PHASE_3 "undef"
#endif
const char* Z2_PIN_PHASE_3_DEFAULT = Z2_PIN_PHASE_3;
#ifndef A_PIN_PHASE_3
#    define A_PIN_PHASE_3 "undef"
#endif
const char* A_PIN_PHASE_3_DEFAULT = A_PIN_PHASE_3;
#ifndef A2_PIN_PHASE_3
#    define A2_PIN_PHASE_3 "undef"
#endif
const char* A2_PIN_PHASE_3_DEFAULT = A2_PIN_PHASE_3;
#ifndef B_PIN_PHASE_3
#    define B_PIN_PHASE_3 "undef"
#endif
const char* B_PIN_PHASE_3_DEFAULT = B_PIN_PHASE_3;
#ifndef B2_PIN_PHASE_3
#    define B2_PIN_PHASE_3 "undef"
#endif
const char* B2_PIN_PHASE_3_DEFAULT = B2_PIN_PHASE_3;
#ifndef C_PIN_PHASE_3
#    define C_PIN_PHASE_3 "undef"
#endif
const char* C_PIN_PHASE_3_DEFAULT = C_PIN_PHASE_3;
#ifndef C2_PIN_PHASE_3
#    define C2_PIN_PHASE_3 "undef"
#endif
const char* C2_PIN_PHASE_3_DEFAULT = C2_PIN_PHASE_3;

// We have all the defaults we need at this point. Settings will include Arduino, so we have to get
// rid of the defines we made earlier:
#undef GPIO_NUM_0
#undef GPIO_NUM_1
#undef GPIO_NUM_2
#undef GPIO_NUM_3
#undef GPIO_NUM_4
#undef GPIO_NUM_5
#undef GPIO_NUM_6
#undef GPIO_NUM_7
#undef GPIO_NUM_8
#undef GPIO_NUM_9
#undef GPIO_NUM_10
#undef GPIO_NUM_11
#undef GPIO_NUM_12
#undef GPIO_NUM_13
#undef GPIO_NUM_14
#undef GPIO_NUM_15
#undef GPIO_NUM_16
#undef GPIO_NUM_17
#undef GPIO_NUM_18
#undef GPIO_NUM_19

#undef GPIO_NUM_21
#undef GPIO_NUM_22
#undef GPIO_NUM_23

#undef GPIO_NUM_25
#undef GPIO_NUM_26
#undef GPIO_NUM_27

#undef GPIO_NUM_32
#undef GPIO_NUM_33
#undef GPIO_NUM_34
#undef GPIO_NUM_35
#undef GPIO_NUM_36
#undef GPIO_NUM_37
#undef GPIO_NUM_38
#undef GPIO_NUM_39
#undef I2SO

// We need settings.h for the settings classes
#include "Grbl.h"
#include "Settings.h"
#include "SettingsDefinitions.h"

// Define the pins:

PinSetting* CoolantFloodPin;       // COOLANT_FLOOD_PIN
PinSetting* CoolantMistPin;        // COOLANT_MIST_PIN
PinSetting* ProbePin;              // PROBE_PIN
PinSetting* SDCardDetPin;          // SDCARD_DET_PIN
PinSetting* SteppersDisablePin;    // STEPPERS_DISABLE_PIN
PinSetting* StepperResetPin;       // STEPPER_RESET
PinSetting* ControlSafetyDoorPin;  // CONTROL_SAFETY_DOOR_PIN
PinSetting* ControlResetPin;       // CONTROL_RESET_PIN
PinSetting* ControlFeedHoldPin;    // CONTROL_FEED_HOLD_PIN
PinSetting* ControlCycleStartPin;  // CONTROL_CYCLE_START_PIN
PinSetting* MacroButton0Pin;       // MACRO_BUTTON_0_PIN
PinSetting* MacroButton1Pin;       // MACRO_BUTTON_1_PIN
PinSetting* MacroButton2Pin;       // MACRO_BUTTON_2_PIN
PinSetting* MacroButton3Pin;       // MACRO_BUTTON_3_PIN

PinSetting* DynamixelTXDPin;  // DYNAMIXEL_TXD
PinSetting* DynamixelRXDPin;  // DYNAMIXEL_RXD
PinSetting* DynamixelRTSPin;  // DYNAMIXEL_RTS

PinSetting* UserDigitalPin[4];
PinSetting* UserAnalogPin[4];

PinSetting* LimitPins[MAX_N_AXIS][2];
PinSetting* StepPins[MAX_N_AXIS][2];
PinSetting* DirectionPins[MAX_N_AXIS][2];
PinSetting* DisablePins[MAX_N_AXIS][2];
PinSetting* ClearToSendPins[MAX_N_AXIS][2];
PinSetting* ServoPins[MAX_N_AXIS][2];
PinSetting* StepStickMS3[MAX_N_AXIS][2];
PinSetting* PhasePins[4][MAX_N_AXIS][2];

// Initialize the pin settings
void make_pin_settings() {
    CoolantFloodPin      = new PinSetting("Pins/CoolantFlood", COOLANT_FLOOD_PIN_DEFAULT);
    CoolantMistPin       = new PinSetting("Pins/CoolantMist", COOLANT_MIST_PIN_DEFAULT);
    ProbePin             = new PinSetting("Pins/Probe", PROBE_PIN_DEFAULT);
    SDCardDetPin         = new PinSetting("Pins/SDCardDet", SDCARD_DET_PIN_DEFAULT);
    SteppersDisablePin   = new PinSetting("Pins/SteppersDisable", STEPPERS_DISABLE_PIN_DEFAULT);
    StepperResetPin      = new PinSetting("Pins/SteppersReset", STEPPER_RESET_DEFAULT);
    ControlSafetyDoorPin = new PinSetting("Pins/ControlSafetyDoor", CONTROL_SAFETY_DOOR_PIN_DEFAULT);
    ControlResetPin      = new PinSetting("Pins/ControlReset", CONTROL_RESET_PIN_DEFAULT);
    ControlFeedHoldPin   = new PinSetting("Pins/ControlFeedHold", CONTROL_FEED_HOLD_PIN_DEFAULT);
    ControlCycleStartPin = new PinSetting("Pins/ControlCycleStart", CONTROL_CYCLE_START_PIN_DEFAULT);
    MacroButton0Pin      = new PinSetting("Pins/MacroButton0", MACRO_BUTTON_0_PIN_DEFAULT);
    MacroButton1Pin      = new PinSetting("Pins/MacroButton1", MACRO_BUTTON_1_PIN_DEFAULT);
    MacroButton2Pin      = new PinSetting("Pins/MacroButton2", MACRO_BUTTON_2_PIN_DEFAULT);
    MacroButton3Pin      = new PinSetting("Pins/MacroButton3", MACRO_BUTTON_3_PIN_DEFAULT);
    DynamixelTXDPin      = new PinSetting("Pins/DynamixelTXD", DYNAMIXEL_TXD_DEFAULT);
    DynamixelRXDPin      = new PinSetting("Pins/DynamixelRXD", DYNAMIXEL_RXD_DEFAULT);
    DynamixelRTSPin      = new PinSetting("Pins/DynamixelRTS", DYNAMIXEL_RTS_DEFAULT);

    // User pins:
    UserDigitalPin[0] = new PinSetting("Pins/UserDigital/0", USER_DIGITAL_PIN_0);
    UserAnalogPin[0]  = new PinSetting("Pins/UserAnalog/0", USER_ANALOG_PIN_0);
    UserDigitalPin[1] = new PinSetting("Pins/UserDigital/1", USER_DIGITAL_PIN_1);
    UserAnalogPin[1]  = new PinSetting("Pins/UserAnalog/1", USER_ANALOG_PIN_1);
    UserDigitalPin[2] = new PinSetting("Pins/UserDigital/2", USER_DIGITAL_PIN_2);
    UserAnalogPin[2]  = new PinSetting("Pins/UserAnalog/2", USER_ANALOG_PIN_2);
    UserDigitalPin[3] = new PinSetting("Pins/UserDigital/3", USER_DIGITAL_PIN_3);
    UserAnalogPin[3]  = new PinSetting("Pins/UserAnalog/3", USER_ANALOG_PIN_3);

    // Axis:
    LimitPins[X_AXIS][0] = new PinSetting("Pins/X/Limit", X_LIMIT_PIN_DEFAULT);
    LimitPins[X_AXIS][1] = new PinSetting("Pins/X2/Limit", X2_LIMIT_PIN_DEFAULT);
    LimitPins[Y_AXIS][0] = new PinSetting("Pins/Y/Limit", Y_LIMIT_PIN_DEFAULT);
    LimitPins[Y_AXIS][1] = new PinSetting("Pins/Y2/Limit", Y2_LIMIT_PIN_DEFAULT);
    LimitPins[Z_AXIS][0] = new PinSetting("Pins/Z/Limit", Z_LIMIT_PIN_DEFAULT);
    LimitPins[Z_AXIS][1] = new PinSetting("Pins/Z2/Limit", Z2_LIMIT_PIN_DEFAULT);
    LimitPins[A_AXIS][0] = new PinSetting("Pins/A/Limit", A_LIMIT_PIN_DEFAULT);
    LimitPins[A_AXIS][1] = new PinSetting("Pins/A2/Limit", A2_LIMIT_PIN_DEFAULT);
    LimitPins[B_AXIS][0] = new PinSetting("Pins/B/Limit", B_LIMIT_PIN_DEFAULT);
    LimitPins[B_AXIS][1] = new PinSetting("Pins/B2/Limit", B2_LIMIT_PIN_DEFAULT);
    LimitPins[C_AXIS][0] = new PinSetting("Pins/C/Limit", C_LIMIT_PIN_DEFAULT);
    LimitPins[C_AXIS][1] = new PinSetting("Pins/C2/Limit", C2_LIMIT_PIN_DEFAULT);

    StepPins[X_AXIS][0] = new PinSetting("Pins/X/Step", X_STEP_PIN_DEFAULT);
    StepPins[X_AXIS][1] = new PinSetting("Pins/X2/Step", X2_STEP_PIN_DEFAULT);
    StepPins[Y_AXIS][0] = new PinSetting("Pins/Y/Step", Y_STEP_PIN_DEFAULT);
    StepPins[Y_AXIS][1] = new PinSetting("Pins/Y2/Step", Y2_STEP_PIN_DEFAULT);
    StepPins[Z_AXIS][0] = new PinSetting("Pins/Z/Step", Z_STEP_PIN_DEFAULT);
    StepPins[Z_AXIS][1] = new PinSetting("Pins/Z2/Step", Z2_STEP_PIN_DEFAULT);
    StepPins[A_AXIS][0] = new PinSetting("Pins/A/Step", A_STEP_PIN_DEFAULT);
    StepPins[A_AXIS][1] = new PinSetting("Pins/A2/Step", A2_STEP_PIN_DEFAULT);
    StepPins[B_AXIS][0] = new PinSetting("Pins/B/Step", B_STEP_PIN_DEFAULT);
    StepPins[B_AXIS][1] = new PinSetting("Pins/B2/Step", B2_STEP_PIN_DEFAULT);
    StepPins[C_AXIS][0] = new PinSetting("Pins/C/Step", C_STEP_PIN_DEFAULT);
    StepPins[C_AXIS][1] = new PinSetting("Pins/C2/Step", C2_STEP_PIN_DEFAULT);

    DirectionPins[X_AXIS][0] = new PinSetting("Pins/X/Direction", X_DIRECTION_PIN_DEFAULT);
    DirectionPins[X_AXIS][1] = new PinSetting("Pins/X2/Direction", X2_DIRECTION_PIN_DEFAULT);
    DirectionPins[Y_AXIS][0] = new PinSetting("Pins/Y/Direction", Y_DIRECTION_PIN_DEFAULT);
    DirectionPins[Y_AXIS][1] = new PinSetting("Pins/Y2/Direction", Y2_DIRECTION_PIN_DEFAULT);
    DirectionPins[Z_AXIS][0] = new PinSetting("Pins/Z/Direction", Z_DIRECTION_PIN_DEFAULT);
    DirectionPins[Z_AXIS][1] = new PinSetting("Pins/Z2/Direction", Z2_DIRECTION_PIN_DEFAULT);
    DirectionPins[A_AXIS][0] = new PinSetting("Pins/A/Direction", A_DIRECTION_PIN_DEFAULT);
    DirectionPins[A_AXIS][1] = new PinSetting("Pins/A2/Direction", A2_DIRECTION_PIN_DEFAULT);
    DirectionPins[B_AXIS][0] = new PinSetting("Pins/B/Direction", B_DIRECTION_PIN_DEFAULT);
    DirectionPins[B_AXIS][1] = new PinSetting("Pins/B2/Direction", B2_DIRECTION_PIN_DEFAULT);
    DirectionPins[C_AXIS][0] = new PinSetting("Pins/C/Direction", C_DIRECTION_PIN_DEFAULT);
    DirectionPins[C_AXIS][1] = new PinSetting("Pins/C2/Direction", C2_DIRECTION_PIN_DEFAULT);

    DisablePins[X_AXIS][0] = new PinSetting("Pins/X/Disable", X_DISABLE_PIN_DEFAULT);
    DisablePins[X_AXIS][1] = new PinSetting("Pins/X2/Disable", X2_DISABLE_PIN_DEFAULT);
    DisablePins[Y_AXIS][0] = new PinSetting("Pins/Y/Disable", Y_DISABLE_PIN_DEFAULT);
    DisablePins[Y_AXIS][1] = new PinSetting("Pins/Y2/Disable", Y2_DISABLE_PIN_DEFAULT);
    DisablePins[Z_AXIS][0] = new PinSetting("Pins/Z/Disable", Z_DISABLE_PIN_DEFAULT);
    DisablePins[Z_AXIS][1] = new PinSetting("Pins/Z2/Disable", Z2_DISABLE_PIN_DEFAULT);
    DisablePins[A_AXIS][0] = new PinSetting("Pins/A/Disable", A_DISABLE_PIN_DEFAULT);
    DisablePins[A_AXIS][1] = new PinSetting("Pins/A2/Disable", A2_DISABLE_PIN_DEFAULT);
    DisablePins[B_AXIS][0] = new PinSetting("Pins/B/Disable", B_DISABLE_PIN_DEFAULT);
    DisablePins[B_AXIS][1] = new PinSetting("Pins/B2/Disable", B2_DISABLE_PIN_DEFAULT);
    DisablePins[C_AXIS][0] = new PinSetting("Pins/C/Disable", C_DISABLE_PIN_DEFAULT);
    DisablePins[C_AXIS][1] = new PinSetting("Pins/C2/Disable", C2_DISABLE_PIN_DEFAULT);

    ClearToSendPins[X_AXIS][0] = new PinSetting("Pins/X/ClearToSend", X_CS_PIN_DEFAULT);
    ClearToSendPins[X_AXIS][1] = new PinSetting("Pins/X2/ClearToSend", X2_CS_PIN_DEFAULT);
    ClearToSendPins[Y_AXIS][0] = new PinSetting("Pins/Y/ClearToSend", Y_CS_PIN_DEFAULT);
    ClearToSendPins[Y_AXIS][1] = new PinSetting("Pins/Y2/ClearToSend", Y2_CS_PIN_DEFAULT);
    ClearToSendPins[Z_AXIS][0] = new PinSetting("Pins/Z/ClearToSend", Z_CS_PIN_DEFAULT);
    ClearToSendPins[Z_AXIS][1] = new PinSetting("Pins/Z2/ClearToSend", Z2_CS_PIN_DEFAULT);
    ClearToSendPins[A_AXIS][0] = new PinSetting("Pins/A/ClearToSend", A_CS_PIN_DEFAULT);
    ClearToSendPins[A_AXIS][1] = new PinSetting("Pins/A2/ClearToSend", A2_CS_PIN_DEFAULT);
    ClearToSendPins[B_AXIS][0] = new PinSetting("Pins/B/ClearToSend", B_CS_PIN_DEFAULT);
    ClearToSendPins[B_AXIS][1] = new PinSetting("Pins/B2/ClearToSend", B2_CS_PIN_DEFAULT);
    ClearToSendPins[C_AXIS][0] = new PinSetting("Pins/C/ClearToSend", C_CS_PIN_DEFAULT);
    ClearToSendPins[C_AXIS][1] = new PinSetting("Pins/C2/ClearToSend", C2_CS_PIN_DEFAULT);

    ServoPins[X_AXIS][0] = new PinSetting("Pins/X/ServoPins", X_SERVO_PIN_DEFAULT);
    ServoPins[X_AXIS][1] = new PinSetting("Pins/X2/ServoPins", X2_SERVO_PIN_DEFAULT);
    ServoPins[Y_AXIS][0] = new PinSetting("Pins/Y/ServoPins", Y_SERVO_PIN_DEFAULT);
    ServoPins[Y_AXIS][1] = new PinSetting("Pins/Y2/ServoPins", Y2_SERVO_PIN_DEFAULT);
    ServoPins[Z_AXIS][0] = new PinSetting("Pins/Z/ServoPins", Z_SERVO_PIN_DEFAULT);
    ServoPins[Z_AXIS][1] = new PinSetting("Pins/Z2/ServoPins", Z2_SERVO_PIN_DEFAULT);
    ServoPins[A_AXIS][0] = new PinSetting("Pins/A/ServoPins", A_SERVO_PIN_DEFAULT);
    ServoPins[A_AXIS][1] = new PinSetting("Pins/A2/ServoPins", A2_SERVO_PIN_DEFAULT);
    ServoPins[B_AXIS][0] = new PinSetting("Pins/B/ServoPins", B_SERVO_PIN_DEFAULT);
    ServoPins[B_AXIS][1] = new PinSetting("Pins/B2/ServoPins", B2_SERVO_PIN_DEFAULT);
    ServoPins[C_AXIS][0] = new PinSetting("Pins/C/ServoPins", C_SERVO_PIN_DEFAULT);
    ServoPins[C_AXIS][1] = new PinSetting("Pins/C2/ServoPins", C2_SERVO_PIN_DEFAULT);

    StepStickMS3[X_AXIS][0] = new PinSetting("Pins/X/StepStickMS3", X_STEPPER_MS3_DEFAULT);
    StepStickMS3[X_AXIS][1] = new PinSetting("Pins/X2/StepStickMS3", X2_STEPPER_MS3_DEFAULT);
    StepStickMS3[Y_AXIS][0] = new PinSetting("Pins/Y/StepStickMS3", Y_STEPPER_MS3_DEFAULT);
    StepStickMS3[Y_AXIS][1] = new PinSetting("Pins/Y2/StepStickMS3", Y2_STEPPER_MS3_DEFAULT);
    StepStickMS3[Z_AXIS][0] = new PinSetting("Pins/Z/StepStickMS3", Z_STEPPER_MS3_DEFAULT);
    StepStickMS3[Z_AXIS][1] = new PinSetting("Pins/Z2/StepStickMS3", Z2_STEPPER_MS3_DEFAULT);
    StepStickMS3[A_AXIS][0] = new PinSetting("Pins/A/StepStickMS3", A_STEPPER_MS3_DEFAULT);
    StepStickMS3[A_AXIS][1] = new PinSetting("Pins/A2/StepStickMS3", A2_STEPPER_MS3_DEFAULT);
    StepStickMS3[B_AXIS][0] = new PinSetting("Pins/B/StepStickMS3", B_STEPPER_MS3_DEFAULT);
    StepStickMS3[B_AXIS][1] = new PinSetting("Pins/B2/StepStickMS3", B2_STEPPER_MS3_DEFAULT);
    StepStickMS3[C_AXIS][0] = new PinSetting("Pins/C/StepStickMS3", C_STEPPER_MS3_DEFAULT);
    StepStickMS3[C_AXIS][1] = new PinSetting("Pins/C2/StepStickMS3", C2_STEPPER_MS3_DEFAULT);

    PhasePins[0][X_AXIS][0] = new PinSetting("Pins/X/Phase0", X_PIN_PHASE_0_DEFAULT);
    PhasePins[0][X_AXIS][1] = new PinSetting("Pins/X2/Phase0", X2_PIN_PHASE_0_DEFAULT);
    PhasePins[0][Y_AXIS][0] = new PinSetting("Pins/Y/Phase0", Y_PIN_PHASE_0_DEFAULT);
    PhasePins[0][Y_AXIS][1] = new PinSetting("Pins/Y2/Phase0", Y2_PIN_PHASE_0_DEFAULT);
    PhasePins[0][Z_AXIS][0] = new PinSetting("Pins/Z/Phase0", Z_PIN_PHASE_0_DEFAULT);
    PhasePins[0][Z_AXIS][1] = new PinSetting("Pins/Z2/Phase0", Z2_PIN_PHASE_0_DEFAULT);
    PhasePins[0][A_AXIS][0] = new PinSetting("Pins/A/Phase0", A_PIN_PHASE_0_DEFAULT);
    PhasePins[0][A_AXIS][1] = new PinSetting("Pins/A2/Phase0", A2_PIN_PHASE_0_DEFAULT);
    PhasePins[0][B_AXIS][0] = new PinSetting("Pins/B/Phase0", B_PIN_PHASE_0_DEFAULT);
    PhasePins[0][B_AXIS][1] = new PinSetting("Pins/B2/Phase0", B2_PIN_PHASE_0_DEFAULT);
    PhasePins[0][C_AXIS][0] = new PinSetting("Pins/C/Phase0", C_PIN_PHASE_0_DEFAULT);
    PhasePins[0][C_AXIS][1] = new PinSetting("Pins/C2/Phase0", C2_PIN_PHASE_0_DEFAULT);

    PhasePins[1][X_AXIS][0] = new PinSetting("Pins/X/Phase1", X_PIN_PHASE_1_DEFAULT);
    PhasePins[1][X_AXIS][1] = new PinSetting("Pins/X2/Phase1", X2_PIN_PHASE_1_DEFAULT);
    PhasePins[1][Y_AXIS][0] = new PinSetting("Pins/Y/Phase1", Y_PIN_PHASE_1_DEFAULT);
    PhasePins[1][Y_AXIS][1] = new PinSetting("Pins/Y2/Phase1", Y2_PIN_PHASE_1_DEFAULT);
    PhasePins[1][Z_AXIS][0] = new PinSetting("Pins/Z/Phase1", Z_PIN_PHASE_1_DEFAULT);
    PhasePins[1][Z_AXIS][1] = new PinSetting("Pins/Z2/Phase1", Z2_PIN_PHASE_1_DEFAULT);
    PhasePins[1][A_AXIS][0] = new PinSetting("Pins/A/Phase1", A_PIN_PHASE_1_DEFAULT);
    PhasePins[1][A_AXIS][1] = new PinSetting("Pins/A2/Phase1", A2_PIN_PHASE_1_DEFAULT);
    PhasePins[1][B_AXIS][0] = new PinSetting("Pins/B/Phase1", B_PIN_PHASE_1_DEFAULT);
    PhasePins[1][B_AXIS][1] = new PinSetting("Pins/B2/Phase1", B2_PIN_PHASE_1_DEFAULT);
    PhasePins[1][C_AXIS][0] = new PinSetting("Pins/C/Phase1", C_PIN_PHASE_1_DEFAULT);
    PhasePins[1][C_AXIS][1] = new PinSetting("Pins/C2/Phase1", C2_PIN_PHASE_1_DEFAULT);

    PhasePins[2][X_AXIS][0] = new PinSetting("Pins/X/Phase2", X_PIN_PHASE_2_DEFAULT);
    PhasePins[2][X_AXIS][1] = new PinSetting("Pins/X2/Phase2", X2_PIN_PHASE_2_DEFAULT);
    PhasePins[2][Y_AXIS][0] = new PinSetting("Pins/Y/Phase2", Y_PIN_PHASE_2_DEFAULT);
    PhasePins[2][Y_AXIS][1] = new PinSetting("Pins/Y2/Phase2", Y2_PIN_PHASE_2_DEFAULT);
    PhasePins[2][Z_AXIS][0] = new PinSetting("Pins/Z/Phase2", Z_PIN_PHASE_2_DEFAULT);
    PhasePins[2][Z_AXIS][1] = new PinSetting("Pins/Z2/Phase2", Z2_PIN_PHASE_2_DEFAULT);
    PhasePins[2][A_AXIS][0] = new PinSetting("Pins/A/Phase2", A_PIN_PHASE_2_DEFAULT);
    PhasePins[2][A_AXIS][1] = new PinSetting("Pins/A2/Phase2", A2_PIN_PHASE_2_DEFAULT);
    PhasePins[2][B_AXIS][0] = new PinSetting("Pins/B/Phase2", B_PIN_PHASE_2_DEFAULT);
    PhasePins[2][B_AXIS][1] = new PinSetting("Pins/B2/Phase2", B2_PIN_PHASE_2_DEFAULT);
    PhasePins[2][C_AXIS][0] = new PinSetting("Pins/C/Phase2", C_PIN_PHASE_2_DEFAULT);
    PhasePins[2][C_AXIS][1] = new PinSetting("Pins/C2/Phase2", C2_PIN_PHASE_2_DEFAULT);

    PhasePins[3][X_AXIS][0] = new PinSetting("Pins/X/Phase3", X_PIN_PHASE_3_DEFAULT);
    PhasePins[3][X_AXIS][1] = new PinSetting("Pins/X2/Phase3", X2_PIN_PHASE_3_DEFAULT);
    PhasePins[3][Y_AXIS][0] = new PinSetting("Pins/Y/Phase3", Y_PIN_PHASE_3_DEFAULT);
    PhasePins[3][Y_AXIS][1] = new PinSetting("Pins/Y2/Phase3", Y2_PIN_PHASE_3_DEFAULT);
    PhasePins[3][Z_AXIS][0] = new PinSetting("Pins/Z/Phase3", Z_PIN_PHASE_3_DEFAULT);
    PhasePins[3][Z_AXIS][1] = new PinSetting("Pins/Z2/Phase3", Z2_PIN_PHASE_3_DEFAULT);
    PhasePins[3][A_AXIS][0] = new PinSetting("Pins/A/Phase3", A_PIN_PHASE_3_DEFAULT);
    PhasePins[3][A_AXIS][1] = new PinSetting("Pins/A2/Phase3", A2_PIN_PHASE_3_DEFAULT);
    PhasePins[3][B_AXIS][0] = new PinSetting("Pins/B/Phase3", B_PIN_PHASE_3_DEFAULT);
    PhasePins[3][B_AXIS][1] = new PinSetting("Pins/B2/Phase3", B2_PIN_PHASE_3_DEFAULT);
    PhasePins[3][C_AXIS][0] = new PinSetting("Pins/C/Phase3", C_PIN_PHASE_3_DEFAULT);
    PhasePins[3][C_AXIS][1] = new PinSetting("Pins/C2/Phase3", C2_PIN_PHASE_3_DEFAULT);
}
