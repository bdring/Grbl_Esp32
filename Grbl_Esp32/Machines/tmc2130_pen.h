// Pin assignments (or lack thereof) for a pen plotter
// using Triaminic TMC2130 drivers.

// Select a version to match your PCB
//#define MACHINE_V1 // version 1 PCB
#define MACHINE_V2 // version 2 PCB

#ifdef MACHINE_V1
        #define MACHINE_NAME    "ESP32_TMC2130_PEN V1"
        #define X_LIMIT_PIN     GPIO_NUM_2
#else
        #define MACHINE_NAME    "ESP32_TMC2130_PEN V2"
        #define X_LIMIT_PIN     GPIO_NUM_32
#endif

#define USE_TRINAMIC // Using at least 1 trinamic driver

#define X_STEP_PIN              GPIO_NUM_12
#define X_DIRECTION_PIN         GPIO_NUM_26
#define X_TRINAMIC              // using SPI control
#define X_DRIVER_TMC2130        // Which Driver Type?
#define X_CS_PIN                GPIO_NUM_17  //chip select
#define X_RSENSE                0.11f   // .11 Ohm

#define Y_STEP_PIN              GPIO_NUM_14
#define Y_DIRECTION_PIN         GPIO_NUM_25
#define Y_TRINAMIC              // using SPI control
#define Y_DRIVER_TMC2130        // Which Driver Type?
#define Y_CS_PIN                GPIO_NUM_16  //chip select
#define Y_RSENSE                0.11f   // .11 Ohm

// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN GPIO_NUM_13

#ifndef USE_SERVO_AXES  // may be set in config.h
        #define USE_SERVO_AXES
#endif

#define SERVO_Z_PIN                     GPIO_NUM_27
#define SERVO_Z_CHANNEL_NUM             5
#define SERVO_Z_RANGE_MIN               0.0
#define SERVO_Z_RANGE_MAX               5.0
#define SERVO_Z_HOMING_TYPE             SERVO_HOMING_TARGET // during homing it will instantly move to a target value
#define SERVO_Z_HOME_POS                SERVO_Z_RANGE_MAX // move to max during homing
#define SERVO_Z_MPOS                    false           // will not use mpos, uses work coordinates


// Comment out servo pin and uncomment spindle pwm pin to use the servo PWM to control a spindle
/*
#define SPINDLE_PWM_PIN    GPIO_NUM_27
*/

// #define X_LIMIT_PIN          See version section
#define Y_LIMIT_PIN             GPIO_NUM_4
#define LIMIT_MASK              B11
