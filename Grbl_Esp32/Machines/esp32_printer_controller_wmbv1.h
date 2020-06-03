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
// #define CUSTOM_CODE_FILENAME "Custom/my_custom_code.cpp"

// === Number of axes

// You can set the number of axes that the machine supports
// by defining N_AXIS.  If you do not define it, 3 will be
// used.  The value must be at least 3, even if your machine
// has fewer axes.
#define N_AXIS 4


// == Pin Assignments

// Step and direction pins; these must be output-capable pins,
// specifically ESP32 GPIO numbers 0..31
// With the I2S I/O expander enabled, you can specify 128..159 as output pins.
#define X_STEP_PIN              137 /* 128 + 8 x 1 + 1 */
#define X_DIRECTION_PIN         135 /* 128 + 8 x 0 + 7 */
#define Y_STEP_PIN              133 /* 128 + 8 x 0 + 5 */
#define Y_DIRECTION_PIN         132 /* 128 + 8 x 0 + 4 */
#define Z_STEP_PIN              130 /* 128 + 8 x 0 + 2 */
#define Z_DIRECTION_PIN         129 /* 128 + 8 x 0 + 1 */
#define A_STEP_PIN              140 /* 128 + 8 x 1 + 4 */
#define A_DIRECTION_PIN         141 /* 128 x 8 x 1 + 5 */

// The 1 bits in LIMIT_MASK set the axes that have limit switches
// For example, if the Y axis has no limit switches but the
// X, Z, A and B axes do, the LIMIT_MASK value would be B11101
//#define LIMIT_MASK              B0111

//#define X_LIMIT_PIN             GPIO_NUM_34
//#define Y_LIMIT_PIN             GPIO_NUM_35
//#define Z_LIMIT_PIN             GPIO_NUM_32

// Common enable for all steppers.  If it is okay to leave
// your drivers enabled at all times, you can leave
// STEPPERS_DISABLE_PIN undefined and use the pin for something else.
// #define STEPPERS_DISABLE_PIN    GPIO_NUM_13

// Pins for controlling various aspects of the machine.  If your
// machine does not support one of these features, you can leave
// the corresponding pin undefined.

#define SPINDLE_PWM_PIN         GPIO_NUM_2   // labeled SpinPWM
// #define SPINDLE_ENABLE_PIN      GPIO_NUM_22  // labeled SpinEnbl
// #define COOLANT_MIST_PIN        GPIO_NUM_21  // labeled Mist
// #define COOLANT_FLOOD_PIN       GPIO_NUM_25  // labeled Flood
#define PROBE_PIN               GPIO_NUM_16  // labeled BLSW

// Input pins for various functions.  If the corresponding pin is not defined,
// the function will not be available.

// CONTROL_SAFETY_DOOR_PIN shuts off the machine when a door is opened
// or some other unsafe condition exists.
// #define CONTROL_SAFETY_DOOR_PIN GPIO_NUM_35  // labeled Door,  needs external pullup

// RESET, FEED_HOLD, and CYCLE_START can control GCode execution at
// the push of a button.

// #define CONTROL_RESET_PIN       GPIO_NUM_34  // labeled Reset, needs external pullup

// === Homing cycles
// The default homing order is Z first (HOMING_CYCLE_0),
// then X (HOMING_CYCLE_1), and finally Y (HOMING_CYCLE_2)
// For machines that need different homing order, you can
// undefine HOMING_CYCLE_n and redefine it accordingly.
// For example, the following would first home X and Y
// simultaneously, then A and B simultaneously, and Z
// not at all.

// #undef HOMING_CYCLE_0

// #define HOMING_CYCLE_0 ((1<<X_AXIS)|(1<<Y_AXIS))

// #undef HOMING_CYCLE_1
// #define HOMING_CYCLE_1 ((1<<A_AXIS)|(1<<B_AXIS))

// #undef HOMING_CYCLE_2
// #endif

// === Control Pins

// The control pins with names that begin with CONTROL_ are
// ignored by default, to avoid noise problems.  To make them
// work, you must undefine IGNORE_CONTROL_PINS
// #undef IGNORE_CONTROL_PINS

// Grbl_ESP32 use the ESP32's special RMT (IR remote control) hardware
// engine to achieve more precise high step rates than can be done
// in software.  That feature is enabled by default, but there are
// some machines that might not want to use it, such as machines that
// do not use ordinary stepper motors.  To turn it off, do this:
#undef USE_RMT_STEPS

//
// I2S (steppers & other output-only pins)
//
#define USE_I2S_IOEXPANDER
#define I2S_IOEXPANDER_BCK      22
#define I2S_IOEXPANDER_WS       17
#define I2S_IOEXPANDER_DATA     21
#define I2S_STEPPER_STREAM

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
