#pragma once
// clang-format off

/*
    6_pack_5160_ATC_Carve.h

    2020-11-11 B. Dring
   
*/
#define MACHINE_NAME            "6 Pack ATC-Carve"
#define CUSTOM_CODE_FILENAME    "Custom/atc.cpp"

#define N_AXIS 3

// === Special Features

// I2S (steppers & other output-only pins)
#define USE_I2S_OUT
#define USE_I2S_STEPS
//#define DEFAULT_STEPPER ST_I2S_STATIC
// === Default settings
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

#define USE_STEPSTICK   // makes sure MS1,2,3 !reset and !sleep are set

#define I2S_OUT_BCK             GPIO_NUM_22
#define I2S_OUT_WS              GPIO_NUM_17
#define I2S_OUT_DATA            GPIO_NUM_21


#define TRINAMIC_RUN_MODE       Motors::TrinamicMode::StealthChop
#define TRINAMIC_HOMING_MODE    Motors::TrinamicMode::CoolStep

// Motor Socket #1
#define X_TRINAMIC_DRIVER       5160
#define X_DISABLE_PIN           I2SO(0)
#define X_DIRECTION_PIN         I2SO(1)
#define X_STEP_PIN              I2SO(2)
#define X_CS_PIN                I2SO(3)
#define X_RSENSE                TMC5160_RSENSE_DEFAULT

// Motor Socket #2
#define Y_TRINAMIC_DRIVER       2130
#define Y_DIRECTION_PIN         I2SO(4)
#define Y_STEP_PIN              I2SO(5)
#define Y_DISABLE_PIN           I2SO(7)
#define Y_CS_PIN                I2SO(6)
#define Y_RSENSE                TMC2130_RSENSE_DEFAULT

// Motor Socket #3
#define Y2_TRINAMIC_DRIVER      2130
#define Y2_DISABLE_PIN          I2SO(8)
#define Y2_DIRECTION_PIN        I2SO(9)
#define Y2_STEP_PIN             I2SO(10)
#define Y2_CS_PIN               I2SO(11)
#define Y2_RSENSE               TMC2130_RSENSE_DEFAULT

// Motor Socket #4
#define Z_TRINAMIC_DRIVER       2130
#define Z_DIRECTION_PIN         I2SO(12)
#define Z_STEP_PIN              I2SO(13)
#define Z_DISABLE_PIN           I2SO(15)
#define Z_CS_PIN                I2SO(14)
#define Z_RSENSE                TMC2130_RSENSE_DEFAULT

/*
    Socket I/O reference
    The list of modules is here...
    https://github.com/bdring/6-Pack_CNC_Controller/wiki/CNC-I-O-Module-List
    
    Pin Mapping
    https://github.com/bdring/6-Pack_CNC_Controller/wiki/Socket-Pin-Number-Mapping

*/


// 4x Input Module in Socket #1
// https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-Switch-Input-module
#define X_LIMIT_PIN                 GPIO_NUM_33
#define Y_LIMIT_PIN                 GPIO_NUM_32
#define Y2_LIMIT_PIN                GPIO_NUM_35
#define Z_LIMIT_PIN                 GPIO_NUM_34

// Socket #3
#define PROBE_PIN                   GPIO_NUM_26
#define PROBE2_PIN                  GPIO_NUM_4
#define MACRO_BUTTON_0_PIN          GPIO_NUM_39  // Manual ATC Switch

// 5V output CNC module in socket #4
// https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-5V-Buffered-Output-Module
#define SPINDLE_TYPE                SpindleType::PWM
#define SPINDLE_OUTPUT_PIN          GPIO_NUM_12
#define ATC_RELEASE_PIN             GPIO_NUM_13 // optional 
#define COOLANT_MIST_PIN            GPIO_NUM_15 // optional
#define COOLANT_FLOOD_PIN           GPIO_NUM_14

#define DEFAULT_SPINDLE_DELAY_SPINUP    5.0
#define DEFAULT_SPINDLE_DELAY_SPINDOWN  5.0

// ================= Setting Defaults ==========================
#define DEFAULT_X_STEPS_PER_MM      160
#define DEFAULT_Y_STEPS_PER_MM      160
#define DEFAULT_Z_STEPS_PER_MM      377.952

#define DEFAULT_X_MAX_RATE          2000
#define DEFAULT_Y_MAX_RATE          2000
#define DEFAULT_Z_MAX_RATE          2000

#define DEFAULT_X_HOMING_MPOS       0
#define DEFAULT_X_MAX_TRAVEL        140

#define DEFAULT_Y_HOMING_MPOS       0
#define DEFAULT_Y_MAX_TRAVEL        140

#define DEFAULT_Z_HOMING_MPOS       0
#define DEFAULT_Z_MAX_TRAVEL        100

#define DEFAULT_X_MICROSTEPS        32
#define DEFAULT_Y_MICROSTEPS        32
#define DEFAULT_Z_MICROSTEPS        32

#define DEFAULT_HOMING_DIR_MASK     (bit(X_AXIS) | bit(Y_AXIS))

#define  DEFAULT_HOMING_ENABLE 1
