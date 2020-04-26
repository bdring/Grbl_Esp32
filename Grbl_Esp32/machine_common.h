#ifndef _machine_common_h
#define _machine_common_h

#ifndef SPINDLE_TYPE 
    #define SPINDLE_TYPE SPINDLE_TYPE_PWM
#endif

// Grbl setting that are common to all machines
// It should not be necessary to change anything herein

#ifndef GRBL_SPI_FREQ
    // You can override these by defining them in a board file.
    // To override, you must set all of them
    //-1 means use the default board pin
    #define GRBL_SPI_SS -1
    #define GRBL_SPI_MOSI -1
    #define GRBL_SPI_MISO -1
    #define GRBL_SPI_SCK -1
    #define GRBL_SPI_FREQ  4000000
#endif

// ESP32 CPU Settings
#define F_TIMERS	80000000     // a reference to the speed of ESP32 timers
#define F_STEPPER_TIMER 20000000     // frequency of step pulse timer
#define STEPPER_OFF_TIMER_PRESCALE 8 // gives a frequency of 10MHz
#define STEPPER_OFF_PERIOD_uSEC  3   // each tick is

#define STEP_PULSE_MIN 2   // uSeconds
#define STEP_PULSE_MAX 10  // uSeconds

// =============== Don't change or comment these out ======================
// They are for legacy purposes and will not affect your I/O

#define X_STEP_BIT 0
#define Y_STEP_BIT 1
#define Z_STEP_BIT 2
#define A_STEP_BIT 3
#define B_STEP_BIT 4
#define C_STEP_BIT 5
#define STEP_MASK B111111

#define X_DIRECTION_BIT 0
#define Y_DIRECTION_BIT 1
#define Z_DIRECTION_BIT 2
#define A_DIRECTION_BIT 3
#define B_DIRECTION_BIT 4
#define C_DIRECTION_BIT 5

#define X_LIMIT_BIT 0
#define Y_LIMIT_BIT 1
#define Z_LIMIT_BIT 2
#define A_LIMIT_BIT 3
#define B_LIMIT_BIT 4
#define C_LIMIT_BIT 5

#define PROBE_MASK 1

#define CONTROL_MASK B1111

#endif // _machine_common_h
