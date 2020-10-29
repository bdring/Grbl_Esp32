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
#include "Config.h"

// Include the file that loads the machine-specific config file.
// machine.h must be edited to choose the desired file.
#include "Machine.h"

// machine_common.h contains settings that do not change
#include "MachineCommon.h"

// Update missing definitions with defaults:
#include "Defaults.h"

// Set defaults to all the macro's:
#ifndef COOLANT_FLOOD_PIN
#    define COOLANT_FLOOD_PIN UNDEFINED_PIN
#endif
const char* COOLANT_FLOOD_PIN_DEFAULT = COOLANT_FLOOD_PIN;
#ifndef COOLANT_MIST_PIN
#    define COOLANT_MIST_PIN UNDEFINED_PIN
#endif
const char* COOLANT_MIST_PIN_DEFAULT = COOLANT_MIST_PIN;
#ifndef PROBE_PIN
#    define PROBE_PIN UNDEFINED_PIN
#endif
const char* PROBE_PIN_DEFAULT = PROBE_PIN;
#ifndef SDCARD_DET_PIN
#    define SDCARD_DET_PIN UNDEFINED_PIN
#endif
const char* SDCARD_DET_PIN_DEFAULT = SDCARD_DET_PIN;
#ifndef STEPPERS_DISABLE_PIN
#    define STEPPERS_DISABLE_PIN UNDEFINED_PIN
#endif
const char* STEPPERS_DISABLE_PIN_DEFAULT = STEPPERS_DISABLE_PIN;
#ifndef STEPPER_RESET
#    define STEPPER_RESET UNDEFINED_PIN
#endif
const char* STEPPER_RESET_DEFAULT = STEPPER_RESET;
#ifndef CONTROL_SAFETY_DOOR_PIN
#    define CONTROL_SAFETY_DOOR_PIN UNDEFINED_PIN
#endif
const char* CONTROL_SAFETY_DOOR_PIN_DEFAULT = CONTROL_SAFETY_DOOR_PIN;
#ifndef CONTROL_RESET_PIN
#    define CONTROL_RESET_PIN UNDEFINED_PIN
#endif
const char* CONTROL_RESET_PIN_DEFAULT = CONTROL_RESET_PIN;
#ifndef CONTROL_FEED_HOLD_PIN
#    define CONTROL_FEED_HOLD_PIN UNDEFINED_PIN
#endif
const char* CONTROL_FEED_HOLD_PIN_DEFAULT = CONTROL_FEED_HOLD_PIN;
#ifndef CONTROL_CYCLE_START_PIN
#    define CONTROL_CYCLE_START_PIN UNDEFINED_PIN
#endif
const char* CONTROL_CYCLE_START_PIN_DEFAULT = CONTROL_CYCLE_START_PIN;
#ifndef MACRO_BUTTON_0_PIN
#    define MACRO_BUTTON_0_PIN UNDEFINED_PIN
#endif
const char* MACRO_BUTTON_0_PIN_DEFAULT = MACRO_BUTTON_0_PIN;
#ifndef MACRO_BUTTON_1_PIN
#    define MACRO_BUTTON_1_PIN UNDEFINED_PIN
#endif
const char* MACRO_BUTTON_1_PIN_DEFAULT = MACRO_BUTTON_1_PIN;
#ifndef MACRO_BUTTON_2_PIN
#    define MACRO_BUTTON_2_PIN UNDEFINED_PIN
#endif
const char* MACRO_BUTTON_2_PIN_DEFAULT = MACRO_BUTTON_2_PIN;
#ifndef MACRO_BUTTON_3_PIN
#    define MACRO_BUTTON_3_PIN UNDEFINED_PIN
#endif
const char* MACRO_BUTTON_3_PIN_DEFAULT = MACRO_BUTTON_3_PIN;
#ifndef DYNAMIXEL_TXD
#    define DYNAMIXEL_TXD UNDEFINED_PIN
#endif
const char* DYNAMIXEL_TXD_DEFAULT = DYNAMIXEL_TXD;
#ifndef DYNAMIXEL_RXD
#    define DYNAMIXEL_RXD UNDEFINED_PIN
#endif
const char* DYNAMIXEL_RXD_DEFAULT = DYNAMIXEL_RXD;
#ifndef DYNAMIXEL_RTS
#    define DYNAMIXEL_RTS UNDEFINED_PIN
#endif
const char* DYNAMIXEL_RTS_DEFAULT = DYNAMIXEL_RTS;

#ifndef USER_DIGITAL_PIN_0
#    define USER_DIGITAL_PIN_0 UNDEFINED_PIN
#endif
const char* USER_DIGITAL_PIN_0_DEFAULT = USER_DIGITAL_PIN_0;

#ifndef USER_DIGITAL_PIN_1
#    define USER_DIGITAL_PIN_1 UNDEFINED_PIN
#endif
const char* USER_DIGITAL_PIN_1_DEFAULT = USER_DIGITAL_PIN_1;

#ifndef USER_DIGITAL_PIN_2
#    define USER_DIGITAL_PIN_2 UNDEFINED_PIN
#endif
const char* USER_DIGITAL_PIN_2_DEFAULT = USER_DIGITAL_PIN_2;

#ifndef USER_DIGITAL_PIN_3
#    define USER_DIGITAL_PIN_3 UNDEFINED_PIN
#endif
const char* USER_DIGITAL_PIN_3_DEFAULT = USER_DIGITAL_PIN_3;

#ifndef USER_ANALOG_PIN_0
#    define USER_ANALOG_PIN_0 UNDEFINED_PIN
#endif
const char* USER_ANALOG_PIN_0_DEFAULT = USER_ANALOG_PIN_0;

#ifndef USER_ANALOG_PIN_1
#    define USER_ANALOG_PIN_1 UNDEFINED_PIN
#endif
const char* USER_ANALOG_PIN_1_DEFAULT = USER_ANALOG_PIN_1;

#ifndef USER_ANALOG_PIN_2
#    define USER_ANALOG_PIN_2 UNDEFINED_PIN
#endif
const char* USER_ANALOG_PIN_2_DEFAULT = USER_ANALOG_PIN_2;

#ifndef USER_ANALOG_PIN_3
#    define USER_ANALOG_PIN_3 UNDEFINED_PIN
#endif
const char* USER_ANALOG_PIN_3_DEFAULT = USER_ANALOG_PIN_3;

#ifndef SPINDLE_OUTPUT_PIN
#    define SPINDLE_OUTPUT_PIN UNDEFINED_PIN
#endif
const char* SPINDLE_OUTPUT_PIN_DEFAULT = SPINDLE_OUTPUT_PIN;

#ifndef SPINDLE_ENABLE_PIN
#    define SPINDLE_ENABLE_PIN UNDEFINED_PIN
#endif
const char* SPINDLE_ENABLE_PIN_DEFAULT = SPINDLE_ENABLE_PIN;

#ifndef SPINDLE_DIRECTION_PIN
#    define SPINDLE_DIRECTION_PIN UNDEFINED_PIN
#endif
const char* SPINDLE_DIRECTION_PIN_DEFAULT = SPINDLE_DIRECTION_PIN;

#ifndef SPINDLE_FORWARD_PIN
#    define SPINDLE_FORWARD_PIN UNDEFINED_PIN
#endif
const char* SPINDLE_FORWARD_PIN_DEFAULT = SPINDLE_FORWARD_PIN;

#ifndef SPINDLE_REVERSE_PIN
#    define SPINDLE_REVERSE_PIN UNDEFINED_PIN
#endif
const char* SPINDLE_REVERSE_PIN_DEFAULT = SPINDLE_REVERSE_PIN;

#ifndef VFD_RS485_TXD_PIN
#    define VFD_RS485_TXD_PIN UNDEFINED_PIN
#endif
const char* VFD_RS485_TXD_PIN_DEFAULT = VFD_RS485_TXD_PIN;

#ifndef VFD_RS485_RXD_PIN
#    define VFD_RS485_RXD_PIN UNDEFINED_PIN
#endif
const char* VFD_RS485_RXD_PIN_DEFAULT = VFD_RS485_RXD_PIN;

#ifndef VFD_RS485_RTS_PIN
#    define VFD_RS485_RTS_PIN UNDEFINED_PIN
#endif
const char* VFD_RS485_RTS_PIN_DEFAULT = VFD_RS485_RTS_PIN;

#ifndef X_LIMIT_PIN
#    define X_LIMIT_PIN UNDEFINED_PIN
#endif
const char* X_LIMIT_PIN_DEFAULT = X_LIMIT_PIN;
#ifndef X2_LIMIT_PIN
#    define X2_LIMIT_PIN UNDEFINED_PIN
#endif
const char* X2_LIMIT_PIN_DEFAULT = X2_LIMIT_PIN;
#ifndef Y_LIMIT_PIN
#    define Y_LIMIT_PIN UNDEFINED_PIN
#endif
const char* Y_LIMIT_PIN_DEFAULT = Y_LIMIT_PIN;
#ifndef Y2_LIMIT_PIN
#    define Y2_LIMIT_PIN UNDEFINED_PIN
#endif
const char* Y2_LIMIT_PIN_DEFAULT = Y2_LIMIT_PIN;
#ifndef Z_LIMIT_PIN
#    define Z_LIMIT_PIN UNDEFINED_PIN
#endif
const char* Z_LIMIT_PIN_DEFAULT = Z_LIMIT_PIN;
#ifndef Z2_LIMIT_PIN
#    define Z2_LIMIT_PIN UNDEFINED_PIN
#endif
const char* Z2_LIMIT_PIN_DEFAULT = Z2_LIMIT_PIN;
#ifndef A_LIMIT_PIN
#    define A_LIMIT_PIN UNDEFINED_PIN
#endif
const char* A_LIMIT_PIN_DEFAULT = A_LIMIT_PIN;
#ifndef A2_LIMIT_PIN
#    define A2_LIMIT_PIN UNDEFINED_PIN
#endif
const char* A2_LIMIT_PIN_DEFAULT = A2_LIMIT_PIN;
#ifndef B_LIMIT_PIN
#    define B_LIMIT_PIN UNDEFINED_PIN
#endif
const char* B_LIMIT_PIN_DEFAULT = B_LIMIT_PIN;
#ifndef B2_LIMIT_PIN
#    define B2_LIMIT_PIN UNDEFINED_PIN
#endif
const char* B2_LIMIT_PIN_DEFAULT = B2_LIMIT_PIN;
#ifndef C_LIMIT_PIN
#    define C_LIMIT_PIN UNDEFINED_PIN
#endif
const char* C_LIMIT_PIN_DEFAULT = C_LIMIT_PIN;
#ifndef C2_LIMIT_PIN
#    define C2_LIMIT_PIN UNDEFINED_PIN
#endif
const char* C2_LIMIT_PIN_DEFAULT = C2_LIMIT_PIN;

#ifndef X_STEP_PIN
#    define X_STEP_PIN UNDEFINED_PIN
#endif
const char* X_STEP_PIN_DEFAULT = X_STEP_PIN;
#ifndef X2_STEP_PIN
#    define X2_STEP_PIN UNDEFINED_PIN
#endif
const char* X2_STEP_PIN_DEFAULT = X2_STEP_PIN;
#ifndef Y_STEP_PIN
#    define Y_STEP_PIN UNDEFINED_PIN
#endif
const char* Y_STEP_PIN_DEFAULT = Y_STEP_PIN;
#ifndef Y2_STEP_PIN
#    define Y2_STEP_PIN UNDEFINED_PIN
#endif
const char* Y2_STEP_PIN_DEFAULT = Y2_STEP_PIN;
#ifndef Z_STEP_PIN
#    define Z_STEP_PIN UNDEFINED_PIN
#endif
const char* Z_STEP_PIN_DEFAULT = Z_STEP_PIN;
#ifndef Z2_STEP_PIN
#    define Z2_STEP_PIN UNDEFINED_PIN
#endif
const char* Z2_STEP_PIN_DEFAULT = Z2_STEP_PIN;
#ifndef A_STEP_PIN
#    define A_STEP_PIN UNDEFINED_PIN
#endif
const char* A_STEP_PIN_DEFAULT = A_STEP_PIN;
#ifndef A2_STEP_PIN
#    define A2_STEP_PIN UNDEFINED_PIN
#endif
const char* A2_STEP_PIN_DEFAULT = A2_STEP_PIN;
#ifndef B_STEP_PIN
#    define B_STEP_PIN UNDEFINED_PIN
#endif
const char* B_STEP_PIN_DEFAULT = B_STEP_PIN;
#ifndef B2_STEP_PIN
#    define B2_STEP_PIN UNDEFINED_PIN
#endif
const char* B2_STEP_PIN_DEFAULT = B2_STEP_PIN;
#ifndef C_STEP_PIN
#    define C_STEP_PIN UNDEFINED_PIN
#endif
const char* C_STEP_PIN_DEFAULT = C_STEP_PIN;
#ifndef C2_STEP_PIN
#    define C2_STEP_PIN UNDEFINED_PIN
#endif
const char* C2_STEP_PIN_DEFAULT = C2_STEP_PIN;

#ifndef X_DIRECTION_PIN
#    define X_DIRECTION_PIN UNDEFINED_PIN
#endif
const char* X_DIRECTION_PIN_DEFAULT = X_DIRECTION_PIN;
#ifndef X2_DIRECTION_PIN
#    define X2_DIRECTION_PIN UNDEFINED_PIN
#endif
const char* X2_DIRECTION_PIN_DEFAULT = X2_DIRECTION_PIN;
#ifndef Y_DIRECTION_PIN
#    define Y_DIRECTION_PIN UNDEFINED_PIN
#endif
const char* Y_DIRECTION_PIN_DEFAULT = Y_DIRECTION_PIN;
#ifndef Y2_DIRECTION_PIN
#    define Y2_DIRECTION_PIN UNDEFINED_PIN
#endif
const char* Y2_DIRECTION_PIN_DEFAULT = Y2_DIRECTION_PIN;
#ifndef Z_DIRECTION_PIN
#    define Z_DIRECTION_PIN UNDEFINED_PIN
#endif
const char* Z_DIRECTION_PIN_DEFAULT = Z_DIRECTION_PIN;
#ifndef Z2_DIRECTION_PIN
#    define Z2_DIRECTION_PIN UNDEFINED_PIN
#endif
const char* Z2_DIRECTION_PIN_DEFAULT = Z2_DIRECTION_PIN;
#ifndef A_DIRECTION_PIN
#    define A_DIRECTION_PIN UNDEFINED_PIN
#endif
const char* A_DIRECTION_PIN_DEFAULT = A_DIRECTION_PIN;
#ifndef A2_DIRECTION_PIN
#    define A2_DIRECTION_PIN UNDEFINED_PIN
#endif
const char* A2_DIRECTION_PIN_DEFAULT = A2_DIRECTION_PIN;
#ifndef B_DIRECTION_PIN
#    define B_DIRECTION_PIN UNDEFINED_PIN
#endif
const char* B_DIRECTION_PIN_DEFAULT = B_DIRECTION_PIN;
#ifndef B2_DIRECTION_PIN
#    define B2_DIRECTION_PIN UNDEFINED_PIN
#endif
const char* B2_DIRECTION_PIN_DEFAULT = B2_DIRECTION_PIN;
#ifndef C_DIRECTION_PIN
#    define C_DIRECTION_PIN UNDEFINED_PIN
#endif
const char* C_DIRECTION_PIN_DEFAULT = C_DIRECTION_PIN;
#ifndef C2_DIRECTION_PIN
#    define C2_DIRECTION_PIN UNDEFINED_PIN
#endif
const char* C2_DIRECTION_PIN_DEFAULT = C2_DIRECTION_PIN;

#ifndef X_DISABLE_PIN
#    define X_DISABLE_PIN UNDEFINED_PIN
#endif
const char* X_DISABLE_PIN_DEFAULT = X_DISABLE_PIN;
#ifndef X2_DISABLE_PIN
#    define X2_DISABLE_PIN UNDEFINED_PIN
#endif
const char* X2_DISABLE_PIN_DEFAULT = X2_DISABLE_PIN;
#ifndef Y_DISABLE_PIN
#    define Y_DISABLE_PIN UNDEFINED_PIN
#endif
const char* Y_DISABLE_PIN_DEFAULT = Y_DISABLE_PIN;
#ifndef Y2_DISABLE_PIN
#    define Y2_DISABLE_PIN UNDEFINED_PIN
#endif
const char* Y2_DISABLE_PIN_DEFAULT = Y2_DISABLE_PIN;
#ifndef Z_DISABLE_PIN
#    define Z_DISABLE_PIN UNDEFINED_PIN
#endif
const char* Z_DISABLE_PIN_DEFAULT = Z_DISABLE_PIN;
#ifndef Z2_DISABLE_PIN
#    define Z2_DISABLE_PIN UNDEFINED_PIN
#endif
const char* Z2_DISABLE_PIN_DEFAULT = Z2_DISABLE_PIN;
#ifndef A_DISABLE_PIN
#    define A_DISABLE_PIN UNDEFINED_PIN
#endif
const char* A_DISABLE_PIN_DEFAULT = A_DISABLE_PIN;
#ifndef A2_DISABLE_PIN
#    define A2_DISABLE_PIN UNDEFINED_PIN
#endif
const char* A2_DISABLE_PIN_DEFAULT = A2_DISABLE_PIN;
#ifndef B_DISABLE_PIN
#    define B_DISABLE_PIN UNDEFINED_PIN
#endif
const char* B_DISABLE_PIN_DEFAULT = B_DISABLE_PIN;
#ifndef B2_DISABLE_PIN
#    define B2_DISABLE_PIN UNDEFINED_PIN
#endif
const char* B2_DISABLE_PIN_DEFAULT = B2_DISABLE_PIN;
#ifndef C_DISABLE_PIN
#    define C_DISABLE_PIN UNDEFINED_PIN
#endif
const char* C_DISABLE_PIN_DEFAULT = C_DISABLE_PIN;
#ifndef C2_DISABLE_PIN
#    define C2_DISABLE_PIN UNDEFINED_PIN
#endif
const char* C2_DISABLE_PIN_DEFAULT = C2_DISABLE_PIN;

#ifndef X_CS_PIN
#    define X_CS_PIN UNDEFINED_PIN
#endif
const char* X_CS_PIN_DEFAULT = X_CS_PIN;
#ifndef X2_CS_PIN
#    define X2_CS_PIN UNDEFINED_PIN
#endif
const char* X2_CS_PIN_DEFAULT = X2_CS_PIN;
#ifndef Y_CS_PIN
#    define Y_CS_PIN UNDEFINED_PIN
#endif
const char* Y_CS_PIN_DEFAULT = Y_CS_PIN;
#ifndef Y2_CS_PIN
#    define Y2_CS_PIN UNDEFINED_PIN
#endif
const char* Y2_CS_PIN_DEFAULT = Y2_CS_PIN;
#ifndef Z_CS_PIN
#    define Z_CS_PIN UNDEFINED_PIN
#endif
const char* Z_CS_PIN_DEFAULT = Z_CS_PIN;
#ifndef Z2_CS_PIN
#    define Z2_CS_PIN UNDEFINED_PIN
#endif
const char* Z2_CS_PIN_DEFAULT = Z2_CS_PIN;
#ifndef A_CS_PIN
#    define A_CS_PIN UNDEFINED_PIN
#endif
const char* A_CS_PIN_DEFAULT = A_CS_PIN;
#ifndef A2_CS_PIN
#    define A2_CS_PIN UNDEFINED_PIN
#endif
const char* A2_CS_PIN_DEFAULT = A2_CS_PIN;
#ifndef B_CS_PIN
#    define B_CS_PIN UNDEFINED_PIN
#endif
const char* B_CS_PIN_DEFAULT = B_CS_PIN;
#ifndef B2_CS_PIN
#    define B2_CS_PIN UNDEFINED_PIN
#endif
const char* B2_CS_PIN_DEFAULT = B2_CS_PIN;
#ifndef C_CS_PIN
#    define C_CS_PIN UNDEFINED_PIN
#endif
const char* C_CS_PIN_DEFAULT = C_CS_PIN;
#ifndef C2_CS_PIN
#    define C2_CS_PIN UNDEFINED_PIN
#endif
const char* C2_CS_PIN_DEFAULT = C2_CS_PIN;

#ifndef X_SERVO_PIN
#    define X_SERVO_PIN UNDEFINED_PIN
#endif
const char* X_SERVO_PIN_DEFAULT = X_SERVO_PIN;
#ifndef X2_SERVO_PIN
#    define X2_SERVO_PIN UNDEFINED_PIN
#endif
const char* X2_SERVO_PIN_DEFAULT = X2_SERVO_PIN;
#ifndef Y_SERVO_PIN
#    define Y_SERVO_PIN UNDEFINED_PIN
#endif
const char* Y_SERVO_PIN_DEFAULT = Y_SERVO_PIN;
#ifndef Y2_SERVO_PIN
#    define Y2_SERVO_PIN UNDEFINED_PIN
#endif
const char* Y2_SERVO_PIN_DEFAULT = Y2_SERVO_PIN;
#ifndef Z_SERVO_PIN
#    define Z_SERVO_PIN UNDEFINED_PIN
#endif
const char* Z_SERVO_PIN_DEFAULT = Z_SERVO_PIN;
#ifndef Z2_SERVO_PIN
#    define Z2_SERVO_PIN UNDEFINED_PIN
#endif
const char* Z2_SERVO_PIN_DEFAULT = Z2_SERVO_PIN;
#ifndef A_SERVO_PIN
#    define A_SERVO_PIN UNDEFINED_PIN
#endif
const char* A_SERVO_PIN_DEFAULT = A_SERVO_PIN;
#ifndef A2_SERVO_PIN
#    define A2_SERVO_PIN UNDEFINED_PIN
#endif
const char* A2_SERVO_PIN_DEFAULT = A2_SERVO_PIN;
#ifndef B_SERVO_PIN
#    define B_SERVO_PIN UNDEFINED_PIN
#endif
const char* B_SERVO_PIN_DEFAULT = B_SERVO_PIN;
#ifndef B2_SERVO_PIN
#    define B2_SERVO_PIN UNDEFINED_PIN
#endif
const char* B2_SERVO_PIN_DEFAULT = B2_SERVO_PIN;
#ifndef C_SERVO_PIN
#    define C_SERVO_PIN UNDEFINED_PIN
#endif
const char* C_SERVO_PIN_DEFAULT = C_SERVO_PIN;
#ifndef C2_SERVO_PIN
#    define C2_SERVO_PIN UNDEFINED_PIN
#endif
const char* C2_SERVO_PIN_DEFAULT = C2_SERVO_PIN;

#ifndef X_STEPPER_MS3
#    define X_STEPPER_MS3 UNDEFINED_PIN
#endif
const char* X_STEPPER_MS3_DEFAULT = X_STEPPER_MS3;
#ifndef X2_STEPPER_MS3
#    define X2_STEPPER_MS3 UNDEFINED_PIN
#endif
const char* X2_STEPPER_MS3_DEFAULT = X2_STEPPER_MS3;
#ifndef Y_STEPPER_MS3
#    define Y_STEPPER_MS3 UNDEFINED_PIN
#endif
const char* Y_STEPPER_MS3_DEFAULT = Y_STEPPER_MS3;
#ifndef Y2_STEPPER_MS3
#    define Y2_STEPPER_MS3 UNDEFINED_PIN
#endif
const char* Y2_STEPPER_MS3_DEFAULT = Y2_STEPPER_MS3;
#ifndef Z_STEPPER_MS3
#    define Z_STEPPER_MS3 UNDEFINED_PIN
#endif
const char* Z_STEPPER_MS3_DEFAULT = Z_STEPPER_MS3;
#ifndef Z2_STEPPER_MS3
#    define Z2_STEPPER_MS3 UNDEFINED_PIN
#endif
const char* Z2_STEPPER_MS3_DEFAULT = Z2_STEPPER_MS3;
#ifndef A_STEPPER_MS3
#    define A_STEPPER_MS3 UNDEFINED_PIN
#endif
const char* A_STEPPER_MS3_DEFAULT = A_STEPPER_MS3;
#ifndef A2_STEPPER_MS3
#    define A2_STEPPER_MS3 UNDEFINED_PIN
#endif
const char* A2_STEPPER_MS3_DEFAULT = A2_STEPPER_MS3;
#ifndef B_STEPPER_MS3
#    define B_STEPPER_MS3 UNDEFINED_PIN
#endif
const char* B_STEPPER_MS3_DEFAULT = B_STEPPER_MS3;
#ifndef B2_STEPPER_MS3
#    define B2_STEPPER_MS3 UNDEFINED_PIN
#endif
const char* B2_STEPPER_MS3_DEFAULT = B2_STEPPER_MS3;
#ifndef C_STEPPER_MS3
#    define C_STEPPER_MS3 UNDEFINED_PIN
#endif
const char* C_STEPPER_MS3_DEFAULT = C_STEPPER_MS3;
#ifndef C2_STEPPER_MS3
#    define C2_STEPPER_MS3 UNDEFINED_PIN
#endif
const char* C2_STEPPER_MS3_DEFAULT = C2_STEPPER_MS3;

#ifndef X_PIN_PHASE_0
#    define X_PIN_PHASE_0 UNDEFINED_PIN
#endif
const char* X_PIN_PHASE_0_DEFAULT = X_PIN_PHASE_0;
#ifndef X2_PIN_PHASE_0
#    define X2_PIN_PHASE_0 UNDEFINED_PIN
#endif
const char* X2_PIN_PHASE_0_DEFAULT = X2_PIN_PHASE_0;
#ifndef Y_PIN_PHASE_0
#    define Y_PIN_PHASE_0 UNDEFINED_PIN
#endif
const char* Y_PIN_PHASE_0_DEFAULT = Y_PIN_PHASE_0;
#ifndef Y2_PIN_PHASE_0
#    define Y2_PIN_PHASE_0 UNDEFINED_PIN
#endif
const char* Y2_PIN_PHASE_0_DEFAULT = Y2_PIN_PHASE_0;
#ifndef Z_PIN_PHASE_0
#    define Z_PIN_PHASE_0 UNDEFINED_PIN
#endif
const char* Z_PIN_PHASE_0_DEFAULT = Z_PIN_PHASE_0;
#ifndef Z2_PIN_PHASE_0
#    define Z2_PIN_PHASE_0 UNDEFINED_PIN
#endif
const char* Z2_PIN_PHASE_0_DEFAULT = Z2_PIN_PHASE_0;
#ifndef A_PIN_PHASE_0
#    define A_PIN_PHASE_0 UNDEFINED_PIN
#endif
const char* A_PIN_PHASE_0_DEFAULT = A_PIN_PHASE_0;
#ifndef A2_PIN_PHASE_0
#    define A2_PIN_PHASE_0 UNDEFINED_PIN
#endif
const char* A2_PIN_PHASE_0_DEFAULT = A2_PIN_PHASE_0;
#ifndef B_PIN_PHASE_0
#    define B_PIN_PHASE_0 UNDEFINED_PIN
#endif
const char* B_PIN_PHASE_0_DEFAULT = B_PIN_PHASE_0;
#ifndef B2_PIN_PHASE_0
#    define B2_PIN_PHASE_0 UNDEFINED_PIN
#endif
const char* B2_PIN_PHASE_0_DEFAULT = B2_PIN_PHASE_0;
#ifndef C_PIN_PHASE_0
#    define C_PIN_PHASE_0 UNDEFINED_PIN
#endif
const char* C_PIN_PHASE_0_DEFAULT = C_PIN_PHASE_0;
#ifndef C2_PIN_PHASE_0
#    define C2_PIN_PHASE_0 UNDEFINED_PIN
#endif
const char* C2_PIN_PHASE_0_DEFAULT = C2_PIN_PHASE_0;

#ifndef X_PIN_PHASE_1
#    define X_PIN_PHASE_1 UNDEFINED_PIN
#endif
const char* X_PIN_PHASE_1_DEFAULT = X_PIN_PHASE_1;
#ifndef X2_PIN_PHASE_1
#    define X2_PIN_PHASE_1 UNDEFINED_PIN
#endif
const char* X2_PIN_PHASE_1_DEFAULT = X2_PIN_PHASE_1;
#ifndef Y_PIN_PHASE_1
#    define Y_PIN_PHASE_1 UNDEFINED_PIN
#endif
const char* Y_PIN_PHASE_1_DEFAULT = Y_PIN_PHASE_1;
#ifndef Y2_PIN_PHASE_1
#    define Y2_PIN_PHASE_1 UNDEFINED_PIN
#endif
const char* Y2_PIN_PHASE_1_DEFAULT = Y2_PIN_PHASE_1;
#ifndef Z_PIN_PHASE_1
#    define Z_PIN_PHASE_1 UNDEFINED_PIN
#endif
const char* Z_PIN_PHASE_1_DEFAULT = Z_PIN_PHASE_1;
#ifndef Z2_PIN_PHASE_1
#    define Z2_PIN_PHASE_1 UNDEFINED_PIN
#endif
const char* Z2_PIN_PHASE_1_DEFAULT = Z2_PIN_PHASE_1;
#ifndef A_PIN_PHASE_1
#    define A_PIN_PHASE_1 UNDEFINED_PIN
#endif
const char* A_PIN_PHASE_1_DEFAULT = A_PIN_PHASE_1;
#ifndef A2_PIN_PHASE_1
#    define A2_PIN_PHASE_1 UNDEFINED_PIN
#endif
const char* A2_PIN_PHASE_1_DEFAULT = A2_PIN_PHASE_1;
#ifndef B_PIN_PHASE_1
#    define B_PIN_PHASE_1 UNDEFINED_PIN
#endif
const char* B_PIN_PHASE_1_DEFAULT = B_PIN_PHASE_1;
#ifndef B2_PIN_PHASE_1
#    define B2_PIN_PHASE_1 UNDEFINED_PIN
#endif
const char* B2_PIN_PHASE_1_DEFAULT = B2_PIN_PHASE_1;
#ifndef C_PIN_PHASE_1
#    define C_PIN_PHASE_1 UNDEFINED_PIN
#endif
const char* C_PIN_PHASE_1_DEFAULT = C_PIN_PHASE_1;
#ifndef C2_PIN_PHASE_1
#    define C2_PIN_PHASE_1 UNDEFINED_PIN
#endif
const char* C2_PIN_PHASE_1_DEFAULT = C2_PIN_PHASE_1;

#ifndef X_PIN_PHASE_2
#    define X_PIN_PHASE_2 UNDEFINED_PIN
#endif
const char* X_PIN_PHASE_2_DEFAULT = X_PIN_PHASE_2;
#ifndef X2_PIN_PHASE_2
#    define X2_PIN_PHASE_2 UNDEFINED_PIN
#endif
const char* X2_PIN_PHASE_2_DEFAULT = X2_PIN_PHASE_2;
#ifndef Y_PIN_PHASE_2
#    define Y_PIN_PHASE_2 UNDEFINED_PIN
#endif
const char* Y_PIN_PHASE_2_DEFAULT = Y_PIN_PHASE_2;
#ifndef Y2_PIN_PHASE_2
#    define Y2_PIN_PHASE_2 UNDEFINED_PIN
#endif
const char* Y2_PIN_PHASE_2_DEFAULT = Y2_PIN_PHASE_2;
#ifndef Z_PIN_PHASE_2
#    define Z_PIN_PHASE_2 UNDEFINED_PIN
#endif
const char* Z_PIN_PHASE_2_DEFAULT = Z_PIN_PHASE_2;
#ifndef Z2_PIN_PHASE_2
#    define Z2_PIN_PHASE_2 UNDEFINED_PIN
#endif
const char* Z2_PIN_PHASE_2_DEFAULT = Z2_PIN_PHASE_2;
#ifndef A_PIN_PHASE_2
#    define A_PIN_PHASE_2 UNDEFINED_PIN
#endif
const char* A_PIN_PHASE_2_DEFAULT = A_PIN_PHASE_2;
#ifndef A2_PIN_PHASE_2
#    define A2_PIN_PHASE_2 UNDEFINED_PIN
#endif
const char* A2_PIN_PHASE_2_DEFAULT = A2_PIN_PHASE_2;
#ifndef B_PIN_PHASE_2
#    define B_PIN_PHASE_2 UNDEFINED_PIN
#endif
const char* B_PIN_PHASE_2_DEFAULT = B_PIN_PHASE_2;
#ifndef B2_PIN_PHASE_2
#    define B2_PIN_PHASE_2 UNDEFINED_PIN
#endif
const char* B2_PIN_PHASE_2_DEFAULT = B2_PIN_PHASE_2;
#ifndef C_PIN_PHASE_2
#    define C_PIN_PHASE_2 UNDEFINED_PIN
#endif
const char* C_PIN_PHASE_2_DEFAULT = C_PIN_PHASE_2;
#ifndef C2_PIN_PHASE_2
#    define C2_PIN_PHASE_2 UNDEFINED_PIN
#endif
const char* C2_PIN_PHASE_2_DEFAULT = C2_PIN_PHASE_2;

#ifndef X_PIN_PHASE_3
#    define X_PIN_PHASE_3 UNDEFINED_PIN
#endif
const char* X_PIN_PHASE_3_DEFAULT = X_PIN_PHASE_3;
#ifndef X2_PIN_PHASE_3
#    define X2_PIN_PHASE_3 UNDEFINED_PIN
#endif
const char* X2_PIN_PHASE_3_DEFAULT = X2_PIN_PHASE_3;
#ifndef Y_PIN_PHASE_3
#    define Y_PIN_PHASE_3 UNDEFINED_PIN
#endif
const char* Y_PIN_PHASE_3_DEFAULT = Y_PIN_PHASE_3;
#ifndef Y2_PIN_PHASE_3
#    define Y2_PIN_PHASE_3 UNDEFINED_PIN
#endif
const char* Y2_PIN_PHASE_3_DEFAULT = Y2_PIN_PHASE_3;
#ifndef Z_PIN_PHASE_3
#    define Z_PIN_PHASE_3 UNDEFINED_PIN
#endif
const char* Z_PIN_PHASE_3_DEFAULT = Z_PIN_PHASE_3;
#ifndef Z2_PIN_PHASE_3
#    define Z2_PIN_PHASE_3 UNDEFINED_PIN
#endif
const char* Z2_PIN_PHASE_3_DEFAULT = Z2_PIN_PHASE_3;
#ifndef A_PIN_PHASE_3
#    define A_PIN_PHASE_3 UNDEFINED_PIN
#endif
const char* A_PIN_PHASE_3_DEFAULT = A_PIN_PHASE_3;
#ifndef A2_PIN_PHASE_3
#    define A2_PIN_PHASE_3 UNDEFINED_PIN
#endif
const char* A2_PIN_PHASE_3_DEFAULT = A2_PIN_PHASE_3;
#ifndef B_PIN_PHASE_3
#    define B_PIN_PHASE_3 UNDEFINED_PIN
#endif
const char* B_PIN_PHASE_3_DEFAULT = B_PIN_PHASE_3;
#ifndef B2_PIN_PHASE_3
#    define B2_PIN_PHASE_3 UNDEFINED_PIN
#endif
const char* B2_PIN_PHASE_3_DEFAULT = B2_PIN_PHASE_3;
#ifndef C_PIN_PHASE_3
#    define C_PIN_PHASE_3 UNDEFINED_PIN
#endif
const char* C_PIN_PHASE_3_DEFAULT = C_PIN_PHASE_3;
#ifndef C2_PIN_PHASE_3
#    define C2_PIN_PHASE_3 UNDEFINED_PIN
#endif
const char* C2_PIN_PHASE_3_DEFAULT = C2_PIN_PHASE_3;

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

PinSetting* SpindleOutputPin;
PinSetting* SpindleEnablePin;
PinSetting* SpindleDirectionPin;
PinSetting* SpindleForwardPin;
PinSetting* SpindleReversePin;

PinSetting* VFDRS485TXDPin;
PinSetting* VFDRS485RXDPin;
PinSetting* VFDRS485RTSPin;

PinSetting* LimitPins[MAX_N_AXIS][2];
PinSetting* StepPins[MAX_N_AXIS][2];
PinSetting* DirectionPins[MAX_N_AXIS][2];
PinSetting* DisablePins[MAX_N_AXIS][2];
PinSetting* ClearToSendPins[MAX_N_AXIS][2];
PinSetting* ServoPins[MAX_N_AXIS][2];
PinSetting* StepStickMS3[MAX_N_AXIS][2];
PinSetting* PhasePins[4][MAX_N_AXIS][2];

#include "Pin.h"

// Initialize the pin settings
void make_pin_settings() {
    CoolantFloodPin      = new PinSetting("Coolant/Flood/Pin", COOLANT_FLOOD_PIN_DEFAULT);
    CoolantMistPin       = new PinSetting("Coolant/Mist/Pin", COOLANT_MIST_PIN_DEFAULT);
    ProbePin             = new PinSetting("Probe/Pin", PROBE_PIN_DEFAULT);
    SDCardDetPin         = new PinSetting("SDCardDet/Pin", SDCARD_DET_PIN_DEFAULT);
    SteppersDisablePin   = new PinSetting("Steppers/Disable/Pin", STEPPERS_DISABLE_PIN_DEFAULT);
    StepperResetPin      = new PinSetting("Steppers/Reset/Pin", STEPPER_RESET_DEFAULT);
    ControlSafetyDoorPin = new PinSetting("Control/SafetyDoor/Pin", CONTROL_SAFETY_DOOR_PIN_DEFAULT);
    ControlResetPin      = new PinSetting("Control/Reset/Pin", CONTROL_RESET_PIN_DEFAULT);
    ControlFeedHoldPin   = new PinSetting("Control/FeedHold/Pin", CONTROL_FEED_HOLD_PIN_DEFAULT);
    ControlCycleStartPin = new PinSetting("Control/CycleStart/Pin", CONTROL_CYCLE_START_PIN_DEFAULT);
    MacroButton0Pin      = new PinSetting("Macro/0/Pin", MACRO_BUTTON_0_PIN_DEFAULT);
    MacroButton1Pin      = new PinSetting("Macro/1/Pin", MACRO_BUTTON_1_PIN_DEFAULT);
    MacroButton2Pin      = new PinSetting("Macro/2/Pin", MACRO_BUTTON_2_PIN_DEFAULT);
    MacroButton3Pin      = new PinSetting("Macro/3/Pin", MACRO_BUTTON_3_PIN_DEFAULT);

    // XXX Move to Dynamixel class
    DynamixelTXDPin = new PinSetting("Dynamixel/TXD/Pin", DYNAMIXEL_TXD_DEFAULT);
    DynamixelRXDPin = new PinSetting("Dynamixel/RXD/Pin", DYNAMIXEL_RXD_DEFAULT);
    DynamixelRTSPin = new PinSetting("Dynamixel/RTS/Pin", DYNAMIXEL_RTS_DEFAULT);

    // User pins:
    UserDigitalPin[0] = new PinSetting("UserDigital/0/Pin", USER_DIGITAL_PIN_0_DEFAULT);
    UserAnalogPin[0]  = new PinSetting("UserAnalog/0/Pin", USER_ANALOG_PIN_0_DEFAULT);
    UserDigitalPin[1] = new PinSetting("UserDigital/1/Pin", USER_DIGITAL_PIN_1_DEFAULT);
    UserAnalogPin[1]  = new PinSetting("UserAnalog/1/Pin", USER_ANALOG_PIN_1_DEFAULT);
    UserDigitalPin[2] = new PinSetting("UserDigital/2/Pin", USER_DIGITAL_PIN_2_DEFAULT);
    UserAnalogPin[2]  = new PinSetting("UserAnalog/2/Pin", USER_ANALOG_PIN_2_DEFAULT);
    UserDigitalPin[3] = new PinSetting("UserDigital/3/Pin", USER_DIGITAL_PIN_3_DEFAULT);
    UserAnalogPin[3]  = new PinSetting("UserAnalog/3/Pin", USER_ANALOG_PIN_3_DEFAULT);

    // Spindles:
    SpindleOutputPin    = new PinSetting("Spindle/Output/Pin", SPINDLE_OUTPUT_PIN_DEFAULT);
    SpindleEnablePin    = new PinSetting("Spindle/Enable/Pin", SPINDLE_ENABLE_PIN_DEFAULT);
    SpindleDirectionPin = new PinSetting("Spindle/Direction/Pin", SPINDLE_DIRECTION_PIN_DEFAULT);
    SpindleForwardPin   = new PinSetting("Spindle/Forward/Pin", SPINDLE_FORWARD_PIN_DEFAULT);
    SpindleReversePin   = new PinSetting("Spindle/Reverse/Pin", SPINDLE_REVERSE_PIN_DEFAULT);
    // XXX Move to VFD class
    VFDRS485TXDPin = new PinSetting("Spindle/VFD/TxD/Pin", VFD_RS485_TXD_PIN_DEFAULT);  // VFD_RS485_TXD_PIN
    VFDRS485RXDPin = new PinSetting("Spindle/VFD/RxD/Pin", VFD_RS485_RXD_PIN_DEFAULT);  // VFD_RS485_RXD_PIN
    VFDRS485RTSPin = new PinSetting("Spindle/VFD/RTS/Pin", VFD_RS485_RTS_PIN_DEFAULT);  // VFD_RS485_RTS_PIN

    // Axis:
    LimitPins[X_AXIS][0] = new PinSetting("X/Limit/Pin", X_LIMIT_PIN_DEFAULT);
    LimitPins[X_AXIS][1] = new PinSetting("X2/Limit/Pin", X2_LIMIT_PIN_DEFAULT);
    LimitPins[Y_AXIS][0] = new PinSetting("Y/Limit/Pin", Y_LIMIT_PIN_DEFAULT);
    LimitPins[Y_AXIS][1] = new PinSetting("Y2/Limit/Pin", Y2_LIMIT_PIN_DEFAULT);
    LimitPins[Z_AXIS][0] = new PinSetting("Z/Limit/Pin", Z_LIMIT_PIN_DEFAULT);
    LimitPins[Z_AXIS][1] = new PinSetting("Z2/Limit/Pin", Z2_LIMIT_PIN_DEFAULT);
    LimitPins[A_AXIS][0] = new PinSetting("A/Limit/Pin", A_LIMIT_PIN_DEFAULT);
    LimitPins[A_AXIS][1] = new PinSetting("A2/Limit/Pin", A2_LIMIT_PIN_DEFAULT);
    LimitPins[B_AXIS][0] = new PinSetting("B/Limit/Pin", B_LIMIT_PIN_DEFAULT);
    LimitPins[B_AXIS][1] = new PinSetting("B2/Limit/Pin", B2_LIMIT_PIN_DEFAULT);
    LimitPins[C_AXIS][0] = new PinSetting("C/Limit/Pin", C_LIMIT_PIN_DEFAULT);
    LimitPins[C_AXIS][1] = new PinSetting("C2/Limit/Pin", C2_LIMIT_PIN_DEFAULT);

    // Move to Standard Stepper Class
    StepPins[X_AXIS][0] = new PinSetting("X/Step/Pin", X_STEP_PIN_DEFAULT);
    StepPins[X_AXIS][1] = new PinSetting("X2/Step/Pin", X2_STEP_PIN_DEFAULT);
    StepPins[Y_AXIS][0] = new PinSetting("Y/Step/Pin", Y_STEP_PIN_DEFAULT);
    StepPins[Y_AXIS][1] = new PinSetting("Y2/Step/Pin", Y2_STEP_PIN_DEFAULT);
    StepPins[Z_AXIS][0] = new PinSetting("Z/Step/Pin", Z_STEP_PIN_DEFAULT);
    StepPins[Z_AXIS][1] = new PinSetting("Z2/Step/Pin", Z2_STEP_PIN_DEFAULT);
    StepPins[A_AXIS][0] = new PinSetting("A/Step/Pin", A_STEP_PIN_DEFAULT);
    StepPins[A_AXIS][1] = new PinSetting("A2/Step/Pin", A2_STEP_PIN_DEFAULT);
    StepPins[B_AXIS][0] = new PinSetting("B/Step/Pin", B_STEP_PIN_DEFAULT);
    StepPins[B_AXIS][1] = new PinSetting("B2/Step/Pin", B2_STEP_PIN_DEFAULT);
    StepPins[C_AXIS][0] = new PinSetting("C/Step/Pin", C_STEP_PIN_DEFAULT);
    StepPins[C_AXIS][1] = new PinSetting("C2/Step/Pin", C2_STEP_PIN_DEFAULT);

    DirectionPins[X_AXIS][0] = new PinSetting("X/Direction/Pin", X_DIRECTION_PIN_DEFAULT);
    DirectionPins[X_AXIS][1] = new PinSetting("X2/Direction/Pin", X2_DIRECTION_PIN_DEFAULT);
    DirectionPins[Y_AXIS][0] = new PinSetting("Y/Direction/Pin", Y_DIRECTION_PIN_DEFAULT);
    DirectionPins[Y_AXIS][1] = new PinSetting("Y2/Direction/Pin", Y2_DIRECTION_PIN_DEFAULT);
    DirectionPins[Z_AXIS][0] = new PinSetting("Z/Direction/Pin", Z_DIRECTION_PIN_DEFAULT);
    DirectionPins[Z_AXIS][1] = new PinSetting("Z2/Direction/Pin", Z2_DIRECTION_PIN_DEFAULT);
    DirectionPins[A_AXIS][0] = new PinSetting("A/Direction/Pin", A_DIRECTION_PIN_DEFAULT);
    DirectionPins[A_AXIS][1] = new PinSetting("A2/Direction/Pin", A2_DIRECTION_PIN_DEFAULT);
    DirectionPins[B_AXIS][0] = new PinSetting("B/Direction/Pin", B_DIRECTION_PIN_DEFAULT);
    DirectionPins[B_AXIS][1] = new PinSetting("B2/Direction/Pin", B2_DIRECTION_PIN_DEFAULT);
    DirectionPins[C_AXIS][0] = new PinSetting("C/Direction/Pin", C_DIRECTION_PIN_DEFAULT);
    DirectionPins[C_AXIS][1] = new PinSetting("C2/Direction/Pin", C2_DIRECTION_PIN_DEFAULT);

    DisablePins[X_AXIS][0] = new PinSetting("X/Disable/Pin", X_DISABLE_PIN_DEFAULT);
    DisablePins[X_AXIS][1] = new PinSetting("X2/Disable/Pin", X2_DISABLE_PIN_DEFAULT);
    DisablePins[Y_AXIS][0] = new PinSetting("Y/Disable/Pin", Y_DISABLE_PIN_DEFAULT);
    DisablePins[Y_AXIS][1] = new PinSetting("Y2/Disable/Pin", Y2_DISABLE_PIN_DEFAULT);
    DisablePins[Z_AXIS][0] = new PinSetting("Z/Disable/Pin", Z_DISABLE_PIN_DEFAULT);
    DisablePins[Z_AXIS][1] = new PinSetting("Z2/Disable/Pin", Z2_DISABLE_PIN_DEFAULT);
    DisablePins[A_AXIS][0] = new PinSetting("A/Disable/Pin", A_DISABLE_PIN_DEFAULT);
    DisablePins[A_AXIS][1] = new PinSetting("A2/Disable/Pin", A2_DISABLE_PIN_DEFAULT);
    DisablePins[B_AXIS][0] = new PinSetting("B/Disable/Pin", B_DISABLE_PIN_DEFAULT);
    DisablePins[B_AXIS][1] = new PinSetting("B2/Disable/Pin", B2_DISABLE_PIN_DEFAULT);
    DisablePins[C_AXIS][0] = new PinSetting("C/Disable/Pin", C_DISABLE_PIN_DEFAULT);
    DisablePins[C_AXIS][1] = new PinSetting("C2/Disable/Pin", C2_DISABLE_PIN_DEFAULT);

    // XXX Move to Trinamic class
    ClearToSendPins[X_AXIS][0] = new PinSetting("X/Trinamic/CTS/Pin", X_CS_PIN_DEFAULT);
    ClearToSendPins[X_AXIS][1] = new PinSetting("X2/Trinamic/CTS/Pin", X2_CS_PIN_DEFAULT);
    ClearToSendPins[Y_AXIS][0] = new PinSetting("Y/Trinamic/CTS/Pin", Y_CS_PIN_DEFAULT);
    ClearToSendPins[Y_AXIS][1] = new PinSetting("Y2/Trinamic/CTS/Pin", Y2_CS_PIN_DEFAULT);
    ClearToSendPins[Z_AXIS][0] = new PinSetting("Z/Trinamic/CTS/Pin", Z_CS_PIN_DEFAULT);
    ClearToSendPins[Z_AXIS][1] = new PinSetting("Z2/Trinamic/CTS/Pin", Z2_CS_PIN_DEFAULT);
    ClearToSendPins[A_AXIS][0] = new PinSetting("A/Trinamic/CTS/Pin", A_CS_PIN_DEFAULT);
    ClearToSendPins[A_AXIS][1] = new PinSetting("A2/Trinamic/CTS/Pin", A2_CS_PIN_DEFAULT);
    ClearToSendPins[B_AXIS][0] = new PinSetting("B/Trinamic/CTS/Pin", B_CS_PIN_DEFAULT);
    ClearToSendPins[B_AXIS][1] = new PinSetting("B2/Trinamic/CTS/Pin", B2_CS_PIN_DEFAULT);
    ClearToSendPins[C_AXIS][0] = new PinSetting("C/Trinamic/CTS/Pin", C_CS_PIN_DEFAULT);
    ClearToSendPins[C_AXIS][1] = new PinSetting("C2/Trinamic/CTS/Pin", C2_CS_PIN_DEFAULT);

    // XXX Move to Servo class
    ServoPins[X_AXIS][0] = new PinSetting("X/Servo/Pin", X_SERVO_PIN_DEFAULT);
    ServoPins[X_AXIS][1] = new PinSetting("X2/Servo/Pin", X2_SERVO_PIN_DEFAULT);
    ServoPins[Y_AXIS][0] = new PinSetting("Y/Servo/Pin", Y_SERVO_PIN_DEFAULT);
    ServoPins[Y_AXIS][1] = new PinSetting("Y2/Servo/Pin", Y2_SERVO_PIN_DEFAULT);
    ServoPins[Z_AXIS][0] = new PinSetting("Z/Servo/Pin", Z_SERVO_PIN_DEFAULT);
    ServoPins[Z_AXIS][1] = new PinSetting("Z2/Servo/Pin", Z2_SERVO_PIN_DEFAULT);
    ServoPins[A_AXIS][0] = new PinSetting("A/Servo/Pin", A_SERVO_PIN_DEFAULT);
    ServoPins[A_AXIS][1] = new PinSetting("A2/Servo/Pin", A2_SERVO_PIN_DEFAULT);
    ServoPins[B_AXIS][0] = new PinSetting("B/Servo/Pin", B_SERVO_PIN_DEFAULT);
    ServoPins[B_AXIS][1] = new PinSetting("B2/Servo/Pin", B2_SERVO_PIN_DEFAULT);
    ServoPins[C_AXIS][0] = new PinSetting("C/Servo/Pin", C_SERVO_PIN_DEFAULT);
    ServoPins[C_AXIS][1] = new PinSetting("C2/Servo/Pin", C2_SERVO_PIN_DEFAULT);

    // XXX Move to StandardStepper class or StepStick class when it appears
    StepStickMS3[X_AXIS][0] = new PinSetting("X/StepStick/MS3/Pin", X_STEPPER_MS3_DEFAULT);
    StepStickMS3[X_AXIS][1] = new PinSetting("X2/StepStick/MS3/Pin", X2_STEPPER_MS3_DEFAULT);
    StepStickMS3[Y_AXIS][0] = new PinSetting("Y/StepStick/MS3/Pin", Y_STEPPER_MS3_DEFAULT);
    StepStickMS3[Y_AXIS][1] = new PinSetting("Y2/StepStick/MS3/Pin", Y2_STEPPER_MS3_DEFAULT);
    StepStickMS3[Z_AXIS][0] = new PinSetting("Z/StepStick/MS3/Pin", Z_STEPPER_MS3_DEFAULT);
    StepStickMS3[Z_AXIS][1] = new PinSetting("Z2/StepStick/MS3/Pin", Z2_STEPPER_MS3_DEFAULT);
    StepStickMS3[A_AXIS][0] = new PinSetting("A/StepStick/MS3/Pin", A_STEPPER_MS3_DEFAULT);
    StepStickMS3[A_AXIS][1] = new PinSetting("A2/StepStick/MS3/Pin", A2_STEPPER_MS3_DEFAULT);
    StepStickMS3[B_AXIS][0] = new PinSetting("B/StepStick/MS3/Pin", B_STEPPER_MS3_DEFAULT);
    StepStickMS3[B_AXIS][1] = new PinSetting("B2/StepStick/MS3/Pin", B2_STEPPER_MS3_DEFAULT);
    StepStickMS3[C_AXIS][0] = new PinSetting("C/StepStick/MS3/Pin", C_STEPPER_MS3_DEFAULT);
    StepStickMS3[C_AXIS][1] = new PinSetting("C2/StepStick/MS3/Pin", C2_STEPPER_MS3_DEFAULT);

    // XXX Move to Unipolar Motor class
    PhasePins[0][X_AXIS][0] = new PinSetting("X/Unipolar/P0/Pin", X_PIN_PHASE_0_DEFAULT);
    PhasePins[0][X_AXIS][1] = new PinSetting("X2/Unipolar/P0/Pin", X2_PIN_PHASE_0_DEFAULT);
    PhasePins[0][Y_AXIS][0] = new PinSetting("Y/Unipolar/P0/Pin", Y_PIN_PHASE_0_DEFAULT);
    PhasePins[0][Y_AXIS][1] = new PinSetting("Y2/Unipolar/P0/Pin", Y2_PIN_PHASE_0_DEFAULT);
    PhasePins[0][Z_AXIS][0] = new PinSetting("Z/Unipolar/P0/Pin", Z_PIN_PHASE_0_DEFAULT);
    PhasePins[0][Z_AXIS][1] = new PinSetting("Z2/Unipolar/P0/Pin", Z2_PIN_PHASE_0_DEFAULT);
    PhasePins[0][A_AXIS][0] = new PinSetting("A/Unipolar/P0/Pin", A_PIN_PHASE_0_DEFAULT);
    PhasePins[0][A_AXIS][1] = new PinSetting("A2/Unipolar/P0/Pin", A2_PIN_PHASE_0_DEFAULT);
    PhasePins[0][B_AXIS][0] = new PinSetting("B/Unipolar/P0/Pin", B_PIN_PHASE_0_DEFAULT);
    PhasePins[0][B_AXIS][1] = new PinSetting("B2/Unipolar/P0/Pin", B2_PIN_PHASE_0_DEFAULT);
    PhasePins[0][C_AXIS][0] = new PinSetting("C/Unipolar/P0/Pin", C_PIN_PHASE_0_DEFAULT);
    PhasePins[0][C_AXIS][1] = new PinSetting("C2/Unipolar/P0/Pin", C2_PIN_PHASE_0_DEFAULT);

    PhasePins[1][X_AXIS][0] = new PinSetting("X/Unipolar/P1/Pin", X_PIN_PHASE_1_DEFAULT);
    PhasePins[1][X_AXIS][1] = new PinSetting("X2/Unipolar/P1/Pin", X2_PIN_PHASE_1_DEFAULT);
    PhasePins[1][Y_AXIS][0] = new PinSetting("Y/Unipolar/P1/Pin", Y_PIN_PHASE_1_DEFAULT);
    PhasePins[1][Y_AXIS][1] = new PinSetting("Y2/Unipolar/P1/Pin", Y2_PIN_PHASE_1_DEFAULT);
    PhasePins[1][Z_AXIS][0] = new PinSetting("Z/Unipolar/P1/Pin", Z_PIN_PHASE_1_DEFAULT);
    PhasePins[1][Z_AXIS][1] = new PinSetting("Z2/Unipolar/P1/Pin", Z2_PIN_PHASE_1_DEFAULT);
    PhasePins[1][A_AXIS][0] = new PinSetting("A/Unipolar/P1/Pin", A_PIN_PHASE_1_DEFAULT);
    PhasePins[1][A_AXIS][1] = new PinSetting("A2/Unipolar/P1/Pin", A2_PIN_PHASE_1_DEFAULT);
    PhasePins[1][B_AXIS][0] = new PinSetting("B/Unipolar/P1/Pin", B_PIN_PHASE_1_DEFAULT);
    PhasePins[1][B_AXIS][1] = new PinSetting("B2/Unipolar/P1/Pin", B2_PIN_PHASE_1_DEFAULT);
    PhasePins[1][C_AXIS][0] = new PinSetting("C/Unipolar/P1/Pin", C_PIN_PHASE_1_DEFAULT);
    PhasePins[1][C_AXIS][1] = new PinSetting("C2/Unipolar/P1/Pin", C2_PIN_PHASE_1_DEFAULT);

    PhasePins[2][X_AXIS][0] = new PinSetting("X/Unipolar/P2/Pin", X_PIN_PHASE_2_DEFAULT);
    PhasePins[2][X_AXIS][1] = new PinSetting("X2/Unipolar/P2/Pin", X2_PIN_PHASE_2_DEFAULT);
    PhasePins[2][Y_AXIS][0] = new PinSetting("Y/Unipolar/P2/Pin", Y_PIN_PHASE_2_DEFAULT);
    PhasePins[2][Y_AXIS][1] = new PinSetting("Y2/Unipolar/P2/Pin", Y2_PIN_PHASE_2_DEFAULT);
    PhasePins[2][Z_AXIS][0] = new PinSetting("Z/Unipolar/P2/Pin", Z_PIN_PHASE_2_DEFAULT);
    PhasePins[2][Z_AXIS][1] = new PinSetting("Z2/Unipolar/P2/Pin", Z2_PIN_PHASE_2_DEFAULT);
    PhasePins[2][A_AXIS][0] = new PinSetting("A/Unipolar/P2/Pin", A_PIN_PHASE_2_DEFAULT);
    PhasePins[2][A_AXIS][1] = new PinSetting("A2/Unipolar/P2/Pin", A2_PIN_PHASE_2_DEFAULT);
    PhasePins[2][B_AXIS][0] = new PinSetting("B/Unipolar/P2/Pin", B_PIN_PHASE_2_DEFAULT);
    PhasePins[2][B_AXIS][1] = new PinSetting("B2/Unipolar/P2/Pin", B2_PIN_PHASE_2_DEFAULT);
    PhasePins[2][C_AXIS][0] = new PinSetting("C/Unipolar/P2/Pin", C_PIN_PHASE_2_DEFAULT);
    PhasePins[2][C_AXIS][1] = new PinSetting("C2/Unipolar/P2/Pin", C2_PIN_PHASE_2_DEFAULT);

    PhasePins[3][X_AXIS][0] = new PinSetting("X/Unipolar/P3/Pin", X_PIN_PHASE_3_DEFAULT);
    PhasePins[3][X_AXIS][1] = new PinSetting("X2/Unipolar/P3/Pin", X2_PIN_PHASE_3_DEFAULT);
    PhasePins[3][Y_AXIS][0] = new PinSetting("Y/Unipolar/P3/Pin", Y_PIN_PHASE_3_DEFAULT);
    PhasePins[3][Y_AXIS][1] = new PinSetting("Y2/Unipolar/P3/Pin", Y2_PIN_PHASE_3_DEFAULT);
    PhasePins[3][Z_AXIS][0] = new PinSetting("Z/Unipolar/P3/Pin", Z_PIN_PHASE_3_DEFAULT);
    PhasePins[3][Z_AXIS][1] = new PinSetting("Z2/Unipolar/P3/Pin", Z2_PIN_PHASE_3_DEFAULT);
    PhasePins[3][A_AXIS][0] = new PinSetting("A/Unipolar/P3/Pin", A_PIN_PHASE_3_DEFAULT);
    PhasePins[3][A_AXIS][1] = new PinSetting("A2/Unipolar/P3/Pin", A2_PIN_PHASE_3_DEFAULT);
    PhasePins[3][B_AXIS][0] = new PinSetting("B/Unipolar/P3/Pin", B_PIN_PHASE_3_DEFAULT);
    PhasePins[3][B_AXIS][1] = new PinSetting("B2/Unipolar/P3/Pin", B2_PIN_PHASE_3_DEFAULT);
    PhasePins[3][C_AXIS][0] = new PinSetting("C/Unipolar/P3/Pin", C_PIN_PHASE_3_DEFAULT);
    PhasePins[3][C_AXIS][1] = new PinSetting("C2/Unipolar/P3/Pin", C2_PIN_PHASE_3_DEFAULT);
}
