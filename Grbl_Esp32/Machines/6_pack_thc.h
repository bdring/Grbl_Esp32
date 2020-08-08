/*
    6_pack_thc.h

    Covers all V1 versions V1p0, V1p1, etc

    Part of Grbl_ESP32
    Pin assignments for the ESP32 I2S 6-axis board
    2018    - Bart Dring
    2020    - Mitch Bradley
    2020    - Michiyasu Odaki
	2020    - William Curry 
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
	This 6-Pack Torch Height Control Custom File Was Created By: William Curry
*/
#define MACHINE_NAME            "6 Pack Controller with Plasma Torch Height Control"
#define CUSTOM_CODE_FILENAME "Custom/torchHeightControl.cpp"//define the torch height control custom code file
// tells grbl we have some special functions to call
#define USE_MACHINE_INIT
#ifdef N_AXIS
        #undef N_AXIS
#endif
#define N_AXIS 3
//Enable the SD Card
#ifndef ENABLE_SD_CARD
    #define ENABLE_SD_CARD
#endif
///Additional THC Setting, these can be displayed in G Code Sender by Typing $S or $+ to show numbers like shown below
#define SHOW_EXTENDED_SETTINGS
#define DEFAULT_THC_DEBUG 1 //$300=1// Boolean //If true this will print torch height debug information out
#define DEFAULT_THC_DEBUG_PRINT_MILLIS 5000 //$301=1000//milliseconds // debug print time in milliseconds
#define DEFAULT_THC_TARGET_VOLTAGE 0 //$302=0// Volts // default target voltage (a real value would be 100 volts)
#define DEFAULT_THC_ARC_DELAY_TIME 500 //$303=750//milliseconds // Time for Arc to Start before running THC
#define DEFAULT_THC_VOLTAGE_FILTER_VALUE 0.98 //$304=0.98//ND// Torch Voltage Filter Time Constant
#define DEFAULT_THC_ITER_FREQ 5 //$305=5//milliseconds// Torch Height Control Time Between Calls, this will directly effect ...
// ... the rate at which the Z axis moves, the machine must be restarted after changing this for it to take effect

// === Special Features

// I2S (steppers & other output-only pins)
#define USE_I2S_OUT
// Define USE_I2S_OUT_STREAM if buffering is used.
// (there will be a delay between the specified I/O operation and the actual I/O execution)
#define USE_I2S_OUT_STREAM
#undef USE_RMT_STEPS
#define STEP_PULSE_DELAY 10 //Mircoseconds to delay between setting direction pin and setting step pin high
//#define USE_STEPSTICK   //Bill comment makes sure MS1,2,3 !reset and !sleep are set
#ifdef ENABLE_WIFI
 #undef ENABLE_WIFI ///Turn off wifi since this blocks the ADC Channel 2 Pins from Reading in Voltage Which is needed for THC
#endif

#define I2S_OUT_BCK      GPIO_NUM_22
#define I2S_OUT_WS       GPIO_NUM_17
#define I2S_OUT_DATA     GPIO_NUM_21

///Axis Data 
#define USE_GANGED_AXES // allow two motors on an axis

#define STEPPER_Z_MS3           I2SO(3)   // Z_CS
#define STEPPER_Y_MS3           I2SO(6)   // Y_CS
#define STEPPER_X_MS3           I2SO(11)  // X_CS
#define STEPPER_Y2_MS3           I2SO(14)  // Y2/A_CS
//#define STEPPER_B_MS3           I2SO(19)  // B_CS
//#define STEPPER_C_MS3           I2SO(22)  // C_CS
#define STEPPER_RESET           GPIO_NUM_19
//Terminal 1
#define Z_DISABLE_PIN           I2SO(0)
#define Z_DIRECTION_PIN         I2SO(1)
#define Z_STEP_PIN              I2SO(2)

//Y Is a Ganged Motor
//#define Y_AXIS_SQUARING
//Terminal 2
#define Y_DIRECTION_PIN         I2SO(4)
#define Y_STEP_PIN              I2SO(5)
#define Y_DISABLE_PIN           I2SO(7)
//Terminal 4
#define Y2_DIRECTION_PIN         I2SO(12) //Motor A Dir
#define Y2_STEP_PIN              I2SO(13) //Motor A Step
#define Y2_DISABLE_PIN           I2SO(15) //Motor A Disable

///Terminal 3
#define X_DISABLE_PIN           I2SO(8)
#define X_DIRECTION_PIN         I2SO(9)
#define X_STEP_PIN              I2SO(10)
/* Bill Comment out these axis'

#define B_DISABLE_PIN           I2SO(16)
#define B_DIRECTION_PIN         I2SO(17)
#define B_STEP_PIN              I2SO(18)

#define C_DIRECTION_PIN         I2SO(20)
#define C_STEP_PIN              I2SO(21)
#define C_DISABLE_PIN           I2SO(23)
*/
///CNC Socket # 1 Opto Isolated
#define X_LIMIT_PIN             GPIO_NUM_33
#define Y_LIMIT_PIN             GPIO_NUM_32
#define Z_LIMIT_PIN             GPIO_NUM_35
#define PROBE_PIN               GPIO_NUM_34
//#define A_LIMIT_PIN             GPIO_NUM_34
//#define B_LIMIT_PIN             GPIO_NUM_39
//#define C_LIMIT_PIN             GPIO_NUM_36

///CNC Socket #2 THC Voltage
#define THC_VOLTAGE_PIN         GPIO_NUM_36
///Resistance Values needed to determine arc voltage i.e. Vout = (Vs*R2)/(R1+R2)
#define VOLTAGE_DIVIDER_R1      470 ///470K Ohms
#define VOLTAGE_DIVIDER_R2      7.6 ///7.6K Ohms


///CNC Socket # 3 Spindle Relay
#define SPINDLE_TYPE            SPINDLE_TYPE_RELAY
#define SPINDLE_OUTPUT_PIN       GPIO_NUM_26

///CNC Socket # 4 Plasma Torch Relay  AKA Coolant Flood Pin 
#define COOLANT_FLOOD_PIN       GPIO_NUM_14

void THCSyncTask(void* pvParameters); //Task called for Torch height controlled thats defined in torchHeightControl.cpp
void THCVoltageTask(void* pvParameters); //Task called for Torch height controlled thats defined in torchHeightControl.cpp
// === Default settings
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE
