#pragma once
// clang-format off
/*
    Esp32_2130_XYYZ.h
	based onL=:
		6_pack_trinamic_V1.h
*/
#define MACHINE_NAME            "Esp32_2130_XYYZ"
#ifdef N_AXIS
        #undef N_AXIS
#endif
#define N_AXIS 3
// === Special Features
// I2S (steppers & other output-only pins)
#define USE_I2S_OUT
#define USE_I2S_STEPS
//#define DEFAULT_STEPPER ST_I2S_STATIC
#define I2S_OUT_BCK      GPIO_NUM_22
#define I2S_OUT_WS       GPIO_NUM_17
#define I2S_OUT_DATA     GPIO_NUM_21
#define TRINAMIC_RUN_MODE           Motors::TrinamicMode::CoolStep
#define TRINAMIC_HOMING_MODE        Motors::TrinamicMode::StallGuard
#define X_TRINAMIC_DRIVER       2130
#define X_DISABLE_PIN           I2SO(0)
#define X_DIRECTION_PIN         I2SO(1)
#define X_STEP_PIN              I2SO(2)
#define X_CS_PIN                I2SO(3)
#define X_RSENSE                TMC2130_RSENSE_DEFAULT
#define Y_TRINAMIC_DRIVER       2130
#define Y_DIRECTION_PIN         I2SO(4)
#define Y_STEP_PIN              I2SO(5)
#define Y_DISABLE_PIN           I2SO(7)
#define Y_CS_PIN                I2SO(6)
#define Y_RSENSE                X_RSENSE
#define Y2_TRINAMIC_DRIVER       2130
#define Y2_DISABLE_PIN           I2SO(8)
#define Y2_DIRECTION_PIN         I2SO(9)
#define Y2_STEP_PIN              I2SO(10)
#define Y2_CS_PIN                I2SO(11)
#define Y2_RSENSE                X_RSENSE
#define Z_TRINAMIC_DRIVER       2130
#define Z_DIRECTION_PIN         I2SO(12)
#define Z_STEP_PIN              I2SO(13)
#define Z_DISABLE_PIN           I2SO(15)
#define Z_CS_PIN                I2SO(14)
#define Z_RSENSE                X_RSENSE
//#define A_TRINAMIC_DRIVER       2130
//#define A_DISABLE_PIN           I2SO(16)
//#define A_DIRECTION_PIN         I2SO(17)
//#define A_STEP_PIN              I2SO(18)
//#define A_CS_PIN                I2SO(19)
//#define A_RSENSE                X_RSENSE
/*
    Socket I/O reference
    The list of modules is here...
    https://github.com/bdring/6-Pack_CNC_Controller/wiki/CNC-I-O-Module-List
    Click on each module to get example for using the modules in the sockets
*/
// Socket #1
// DO NOT USE -- DUE TO STALLGUARD BEING USED
// #1 GPIO_NUM_33 // (Sg1)
// #2 GPIO_NUM_32 // (Sg2)
// #3 GPIO_NUM_35  // (Sg3) (input only)
// #4 GPIO_NUM_34  // (Sg4) (input only)
// Socket #2
// #1 GPIO_NUM_2
// #2 GPIO_NUM_25
// #3 GPIO_NUM_39  // (Sg5) (input only)
// #4 GPIO_NUM_36  // (Sg6) (input only)
// 4x Switch Input module  in socket #2
//#define COOLANT_MIST_PIN        GPIO_NUM_2
//#define PROBE_PIN               GPIO_NUM_25
// Socket #3
// #1 GPIO_NUM_26
// #2 GPIO_NUM_4
// #3 GPIO_NUM_16
// #4 GPIO_NUM_27
// 0-10v CNC Module in Socket #3
#define SPINDLE_TYPE            SpindleType::_10V
#define SPINDLE_OUTPUT_PIN      GPIO_NUM_26
#define SPINDLE_ENABLE_PIN      GPIO_NUM_4
#define SPINDLE_DIR_PIN         GPIO_NUM_16
// Socket #4
// #1 GPIO_NUM_14
// #2 GPIO_NUM_13
// #3 GPIO_NUM_15
// #4 GPIO_NUM_12
// Socket #5
// #1 I2SO(24)  (output only)
// #2 I2SO(25)  (output only)
// #3 I2SO(26)  (output only)
// #4 I2SO(27)  (output only)

#define X_LIMIT_PIN GPIO_NUM_33

// === Default settings
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE
// === Custom Settings
#define DEFAULT_HOMING_SQUARED_AXES bit(Y_AXIS)