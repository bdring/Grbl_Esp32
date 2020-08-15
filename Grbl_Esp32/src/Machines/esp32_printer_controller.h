#pragma once
// clang-format off

/*
    esp32_printer_controller.h
    Part of Grbl_ESP32
    Template for a machine configuration file.
    2020    - Mitch Bradley
    2020    - Michiyasu Odaki
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

// This contains a long list of things that might possibly be
// configured.  Most machines - especially simple cartesian machines
// that use stepper motors - will only need to define a few of the
// options herein, often just the pin assignments.

// Pin assignments depend on how the ESP32 is connected to
// the external machine.  Typically the ESP32 module plugs into
// an adapter board that wires specific ESP32 GPIO pins to
// other connectors on the board, such as Pololu sockets for
// stepper drivers or connectors for external drivers, limit
// pins, spindle control, etc.  This file describes how those
// GPIO pins are wired to those other connectors.

// Some machines might choose to use an adapter board in a
// non-standard way, for example a 3-axis board might have axes
// labeled XYZ, but the machine might have only 2 axes one of which is
// driven by two ganged motors.  In that case, you would need
// a custom version of this file that assigns the pins differently
// from the adapter board labels.

// In addition to pin assignments, many other aspects of Grbl
// can be configured, such as spindle speeds, special motor
// types like servos and unipolars, homing order, default values
// for $$ settings, etc.  A detailed list of such options is
// given below.

// Furthermore, it is possible to implement special complex
// behavior in custom C++ code, for non-Cartesian machines,
// unusual homing cycles, etc.  See the Special Features section
// below for additional instructions.

// 3D printer controller using ESP32 processor
// https://github.com/MitchBradley/Esp32PrinterController

// === Machine Name
// Change TEMPLATE to some name of your own choosing.  That name
// will be shown in a Grbl startup message to identify your
// configuration.

#define MACHINE_NAME            "ESP32_PRINTER_CONTROLLER"

// If your machine requires custom code as described below in
// Special Features, you must copy Custom/custom_code_template.cpp
// to a new name like Custom/my_custom_code.cpp, implement the
// functions therein, and enable its use by defining:
#define CUSTOM_CODE_FILENAME "Custom/esp32_printer_controller.cpp"

// === Number of axes

// You can set the number of axes that the machine supports
// by defining N_AXIS.  If you do not define it, 3 will be
// used.  The value must be at least 3, even if your machine
// has fewer axes.
#define N_AXIS 3


// == Pin Assignments

// Step and direction pins; these must be output-capable pins,
// specifically ESP32 GPIO numbers 0..31
// With the I2S I/O expander enabled, you can specify 128..159 as output pins.
#define X_STEP_PIN              I2SO(9)
#define X_DIRECTION_PIN         I2SO(7)
#define Y_STEP_PIN              I2SO(5)
#define Y_DIRECTION_PIN         I2SO(4)
#define Z_STEP_PIN              I2SO(2)
#define Z_DIRECTION_PIN         I2SO(1)
#define A_STEP_PIN              I2SO(12)
#define A_DIRECTION_PIN         I2SO(13)

#define X_LIMIT_PIN             GPIO_NUM_34
//#define Y_LIMIT_PIN             GPIO_NUM_35
//#define Z_LIMIT_PIN             GPIO_NUM_32

// Common enable for all steppers.  If it is okay to leave
// your drivers enabled at all times, you can leave
// STEPPERS_DISABLE_PIN undefined and use the pin for something else.
// #define STEPPERS_DISABLE_PIN    GPIO_NUM_13

// Pins for controlling various aspects of the machine.  If your
// machine does not support one of these features, you can leave
// the corresponding pin undefined.

#define SPINDLE_TYPE SPINDLE_TYPE_NONE
// #define SPINDLE_PWM_PIN         GPIO_NUM_2   // labeled SpinPWM
// #define SPINDLE_ENABLE_PIN      GPIO_NUM_22  // labeled SpinEnbl
// #define COOLANT_MIST_PIN        GPIO_NUM_21  // labeled Mist
// #define COOLANT_FLOOD_PIN       GPIO_NUM_25  // labeled Flood
// #define PROBE_PIN               GPIO_NUM_32  // labeled Probe

// Input pins for various functions.  If the corresponding pin is not defined,
// the function will not be available.

// CONTROL_SAFETY_DOOR_PIN shuts off the machine when a door is opened
// or some other unsafe condition exists.
// #define CONTROL_SAFETY_DOOR_PIN GPIO_NUM_35  // labeled Door,  needs external pullup

// RESET, FEED_HOLD, and CYCLE_START can control GCode execution at
// the push of a button.

// #define CONTROL_FEED_HOLD_PIN   GPIO_NUM_36  // labeled Hold,  needs external pullup
// #define CONTROL_CYCLE_START_PIN GPIO_NUM_39  // labeled Start, needs external pullup

// === Ganging
// If you need to use two motors on one axis, you can "gang" the motors by
// defining a second pin to control the other motor on the axis.  For example:

// #define Y2_STEP_PIN             GPIO_NUM_27  /* labeled Z */
// #define Y2_DIRECTION_PIN        GPIO_NUM_33  /* labeled Z */

// === Servos
// To use a servo motor on an axis, do not define step and direction
// pins for that axis, but instead include a block like this:
//#define USE_SERVO_AXES

//#define SERVO_Z_PIN             GPIO_NUM_15  // It cannot be used when JTAG debugging
//#define SERVO_Z_RANGE_MIN       0.0
//#define SERVO_Z_RANGE_MAX       5.0
//#define SERVO_Z_HOMING_TYPE     SERVO_HOMING_TARGET // during homing it will instantly move to a target value
//#define SERVO_Z_HOME_POS        SERVO_Z_RANGE_MAX // move to max during homing
//#define SERVO_Z_MPOS            false           // will not use mpos, uses work coordinates

// === Homing cycles
// The default homing order is Z first (HOMING_CYCLE_0),
// then X (HOMING_CYCLE_1), and finally Y (HOMING_CYCLE_2)
// For machines that need different homing order, you can
// undefine HOMING_CYCLE_n and redefine it accordingly.
// For example, the following would first home X and Y
// simultaneously, then A and B simultaneously, and Z
// not at all.

// redefine some stuff from config.h
#ifdef HOMING_CYCLE_0
    #undef HOMING_CYCLE_0
#endif
#define HOMING_CYCLE_0 bit(X_AXIS) // this 'bot only homes the X axis
#ifdef HOMING_CYCLE_1
    #undef HOMING_CYCLE_1
#endif
#ifdef HOMING_CYCLE_2
    #undef HOMING_CYCLE_2
#endif

// === Default settings
// Grbl has many run-time settings that the user can changed by
// commands like $110=2000 .  Their values are stored in EEPROM
// so they persist after the controller has been powered down.
// Those settings have default values that are used if the user
// has not altered them, or if the settings are explicitly reset
// to the default values wth $RST=$.
//
// The default values are established in defaults.h, but you
// can override any one of them by definining it here, for example:

//#define DEFAULT_INVERT_LIMIT_PINS 1
//#define DEFAULT_REPORT_INCHES 1
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

#define DEFAULT_HOMING_ENABLE 1
#define DEFAULT_HOMING_DIR_MASK 0 // move positive dir Z, negative X,Y
#define DEFAULT_HOMING_FEED_RATE 200.0 // mm/min
#define DEFAULT_HOMING_SEEK_RATE 1000.0 // mm/min
#define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
#define DEFAULT_HOMING_PULLOFF 3.0 // mm

#define DEFAULT_HARD_LIMIT_ENABLE 1
// === Control Pins

// The control pins with names that begin with CONTROL_ are
// ignored by default, to avoid noise problems.  To make them
// work, you must undefine IGNORE_CONTROL_PINS
#undef IGNORE_CONTROL_PINS

// If some of the control pin switches are normally closed
// (the default is normally open), you can invert some of them
// with INVERT_CONTROL_PIN_MASK.  The bits in the mask are
// Cycle Start, Feed Hold, Reset, Safety Door.  To use a
// normally open switch on Reset, you would say
// #define INVERT_CONTROL_PIN_MASK B1101

// If your control pins do not have adequate hardware signal
// conditioning, you can define these to use software to
// reduce false triggering.
// #define ENABLE_CONTROL_SW_DEBOUNCE // Default disabled. Uncomment to enable.
// #define CONTROL_SW_DEBOUNCE_PERIOD 32 // in milliseconds default 32 microseconds
#ifndef ENABLE_CONTROL_SW_DEBOUNCE
    #define ENABLE_CONTROL_SW_DEBOUNCE
#endif

#ifdef CONTROL_SW_DEBOUNCE_PERIOD
    #undef CONTROL_SW_DEBOUNCE_PERIOD
#endif
#define CONTROL_SW_DEBOUNCE_PERIOD 100 // really long debounce

#ifdef INVERT_CONTROL_PIN_MASK
    #undef INVERT_CONTROL_PIN_MASK
#endif
#define INVERT_CONTROL_PIN_MASK B11111111

//
// I2S (steppers & other output-only pins)
//
#define USE_I2S_OUT
#define USE_I2S_STEPS
//#define DEFAULT_STEPPER ST_I2S_STATIC
#define I2S_OUT_BCK      GPIO_NUM_22
#define I2S_OUT_WS       GPIO_NUM_17
#define I2S_OUT_DATA     GPIO_NUM_21
#define I2S_OUT_NUM_BITS 16

// === Special Features
// Grbl_ESP32 can support non-Cartesian machines and some other
// scenarios that cannot be handled by choosing from a set of
// predefined selections.  Instead they require machine-specific
// C++ code functions.  There are callouts in the core code for
// such code, guarded by ifdefs that enable calling the individual
// functions.  custom_code_template.cpp describes the functions
// that you can implement.  The ifdef guards are described below:
//
#define USE_MACHINE_INIT

// USE_CUSTOM_HOMING enables the user_defined_homing() function
// that can implement an arbitrary homing sequence.
// #define USE_CUSTOM_HOMING

// USE_KINEMATICS enables the functions inverse_kinematics(),
// kinematics_pre_homing(), and kinematics_post_homing(),
// so non-Cartesian machines can be implemented.
// #define USE_KINEMATICS

// USE_FWD_KINEMATIC enables the forward_kinematics() function
// that converts motor positions in non-Cartesian coordinate
// systems back to Cartesian form, for status reports.
//#define USE_FWD_KINEMATIC

// USE_TOOL_CHANGE enables the user_tool_change() function
// that implements custom tool change procedures.
// #define USE_TOOL_CHANGE

// Any one of MACRO_BUTTON_0_PIN, MACRO_BUTTON_1_PIN, and MACRO_BUTTON_2_PIN
// enables the user_defined_macro(number) function which
// implements custom behavior at the press of a button
// #define MACRO_BUTTON_0_PIN

// USE_M30 enables the user_m30() function which implements
// custom behavior when a GCode programs stops at the end
// #define USE_M30

// USE_TRIAMINIC enables a suite of functions that are defined
// in grbl_triaminic.cpp, allowing the use of Triaminic stepper
// drivers that require software configuration at startup.
// There are several options that control the details of such
// drivers; inspect the code in grbl_triaminic.cpp to see them.
// #define USE_TRIAMINIC
// #define X_TRIAMINIC
// #define X_DRIVER_TMC2209
// #define TRIAMINIC_DAISY_CHAIN

// USE_MACHINE_TRINAMIC_INIT enables the machine_triaminic_setup()
// function that replaces the normal setup of Triaminic drivers.
// It could, for, example, setup StallGuard or other special modes.
// #define USE_MACHINE_TRINAMIC_INIT


// === Grbl behavior options
// There are quite a few options that control aspects of Grbl that
// are not specific to particular machines.  They are listed and
// described in config.h after it includes the file machine.h.
// Normally you would not need to change them, but if you do,
// it will be necessary to make the change in config.h
