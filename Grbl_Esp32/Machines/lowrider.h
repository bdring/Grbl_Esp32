// Pin assignments for the Buildlog.net MPCNC controller
// used in lowrider mode. Low rider has (2) Y and Z and one X motor
// These will not match the silkscreen or schematic descriptions

// !!!!!!!!!!!!!!!!! Warning: Untested !!!!!!!!!!!!!!!!! //
//#define V1P1
#define V1P2  // works for V1.2.1 as well

#ifdef V1P1
#define MACHINE_NAME "MACHINE_LOWRIDER_V1P1"
#else // V1P2
#define MACHINE_NAME "MACHINE_LOWRIDER_V1P2"
#endif

#define USE_GANGED_AXES // allow two motors on an axis

#define X_STEP_PIN      GPIO_NUM_27     // use Z labeled connector
#define X_DIRECTION_PIN GPIO_NUM_33     // use Z labeled connector

#define Y_STEP_PIN      GPIO_NUM_14
#define Y2_STEP_PIN     GPIO_NUM_21     // ganged motor
#define Y_DIRECTION_PIN GPIO_NUM_25
#define Y_AXIS_SQUARING

#define Z_STEP_PIN      GPIO_NUM_12     // use X labeled connector
#define Z2_STEP_PIN     GPIO_NUM_22     // use X labeled connector
#define Z_DIRECTION_PIN GPIO_NUM_26     // use X labeled connector
#define Z_AXIS_SQUARING

// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN GPIO_NUM_13

// Note: if you use PWM rather than relay, you could map GPIO_NUM_2 to mist or flood
//#define USE_SPINDLE_RELAY

#ifdef USE_SPINDLE_RELAY
        #ifdef V1P1
                #define SPINDLE_PWM_PIN GPIO_NUM_17
        #else // V1p2
                #define SPINDLE_PWM_PIN GPIO_NUM_2
        #endif
#else
        #define SPINDLE_PWM_PIN         GPIO_NUM_16
        #define SPINDLE_ENABLE_PIN      GPIO_NUM_32
#endif

// Note: Only uncomment this if USE_SPINDLE_RELAY is commented out.
// Relay can be used for Spindle or Coolant
//#define COOLANT_FLOOD_PIN             GPIO_NUM_17

#define X_LIMIT_PIN                     GPIO_NUM_15
#define Y_LIMIT_PIN                     GPIO_NUM_4

#ifdef V1P1     //v1p1
        #define Z_LIMIT_PIN             GPIO_NUM_2
#else
        #define Z_LIMIT_PIN             GPIO_NUM_17
#endif

#define LIMIT_MASK              B111

#ifdef V1P2
        #ifndef ENABLE_SOFTWARE_DEBOUNCE   // V1P2 does not have R/C filters
                #define ENABLE_SOFTWARE_DEBOUNCE
        #endif
#endif

// The default value in config.h is wrong for this controller
#ifdef INVERT_CONTROL_PIN_MASK
        #undef INVERT_CONTROL_PIN_MASK
#endif

#define INVERT_CONTROL_PIN_MASK   B1110

// Note: check the #define IGNORE_CONTROL_PINS is the way you want in config.h
#define CONTROL_RESET_PIN         GPIO_NUM_34  // needs external pullup
#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_36  // needs external pullup
#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup
