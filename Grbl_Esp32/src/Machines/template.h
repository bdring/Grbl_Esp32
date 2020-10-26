#pragma once
// clang-format off

/*
    template.h
    Part of Grbl_ESP32

    Template for a machine configuration file.

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

// === Machine Name
// Change TEMPLATE to some name of your own choosing.  That name
// will be shown in a Grbl startup message to identify your
// configuration.

#define MACHINE_NAME            "TEMPLATE"

// If your machine requires custom code as described below in
// Special Features, you must copy Custom/custom_code_template.cpp
// to a new name like Custom/my_custom_code.cpp, implement the
// functions therein, and enable its use by defining:
// #define CUSTOM_CODE_FILENAME "Custom/my_custom_code.cpp"

// === Number of axes

// You can set the number of axes that the machine supports
// by defining N_AXIS.  If you do not define it, 3 will be
// used.  The value must be at least 3, even if your machine
// has fewer axes.
// #define N_AXIS 4


// == Pin Assignments

// Step and direction pins; these must be output-capable pins,
// specifically ESP32 GPIO numbers 0..31
// #define X_STEP_PIN              GPIO_NUM_12
// #define X_DIRECTION_PIN         GPIO_NUM_14
// #define Y_STEP_PIN              GPIO_NUM_26
// #define Y_DIRECTION_PIN         GPIO_NUM_15
// #define Z_STEP_PIN              GPIO_NUM_27
// #define Z_DIRECTION_PIN         GPIO_NUM_33

// #define X_LIMIT_PIN             GPIO_NUM_17
// #define Y_LIMIT_PIN             GPIO_NUM_4
// #define Z_LIMIT_PIN             GPIO_NUM_16

// Common enable for all steppers.  If it is okay to leave
// your drivers enabled at all times, you can leave
// STEPPERS_DISABLE_PIN undefined and use the pin for something else.
// #define STEPPERS_DISABLE_PIN    GPIO_NUM_13

// Pins for controlling various aspects of the machine.  If your
// machine does not support one of these features, you can leave
// the corresponding pin undefined.

// #define SPINDLE_OUTPUT_PIN         GPIO_NUM_2   // labeled SpinPWM
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

// #define CONTROL_RESET_PIN       GPIO_NUM_34  // labeled Reset, needs external pullup
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

// #define SERVO_Z_PIN             GPIO_NUM_22

// === Homing cycles
// Set them using $Homing/Cycle0= optionally up to $Homing/Cycle5=

// === Default settings
// Grbl has many run-time settings that the user can changed by
// commands like $110=2000 .  Their values are stored in non-volatile
// storage so they persist after the controller has been powered down.
// Those settings have default values that are used if the user
// has not altered them, or if the settings are explicitly reset
// to the default values wth $RST=$.
//
// The default values are established in defaults.h, but you
// can override any one of them by definining it here, for example:

//#define DEFAULT_INVERT_LIMIT_PINS 1
//#define DEFAULT_REPORT_INCHES 1

// === Control Pins

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


// Grbl_ESP32 use the ESP32's special RMT (IR remote control) hardware
// engine to achieve more precise high step rates than can be done
// in software.  That feature is enabled by default, but there are
// some machines that might not want to use it, such as machines that
// do not use ordinary stepper motors.  To turn it off, do this:
// #undef USE_RMT_STEPS

// === Special Features
// Grbl_ESP32 can support non-Cartesian machines and some other
// scenarios that cannot be handled by choosing from a set of
// predefined selections.  Instead they require machine-specific
// C++ code functions.  There are callouts in the core code for
// such code, guarded by ifdefs that enable calling the individual
// functions.  custom_code_template.cpp describes the functions
// that you can implement.  The ifdef guards are described below:
//
// USE_CUSTOM_HOMING enables the user_defined_homing(uint8_t cycle_mask) function
// that can implement an arbitrary homing sequence.
// #define USE_CUSTOM_HOMING

// USE_KINEMATICS enables the functions inverse_kinematics(),
// kinematics_pre_homing(), and kinematics_post_homing(),
// so non-Cartesian machines can be implemented.
// #define USE_KINEMATICS

// USE_FWD_KINEMATICS enables the forward_kinematics() function
// that converts motor positions in non-Cartesian coordinate
// systems back to Cartesian form, for status reports.
//#define USE_FWD_KINEMATICS

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
