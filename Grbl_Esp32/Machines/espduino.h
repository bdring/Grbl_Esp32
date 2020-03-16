// Pin assignments for ESPDUINO-32 Boards and Protoneer V3 boards
// Note: Probe pin is mapped, but will require a 10k external pullup to 3.3V to work.

// Rebooting...See this issue https://github.com/bdring/Grbl_Esp32/issues/314
// !!!! Experimental Untested !!!!!

#define MACHINE_NAME "MACHINE_ESPDUINO_32"

#define X_STEP_PIN              GPIO_NUM_26
#define X_DIRECTION_PIN         GPIO_NUM_16

#define Y_STEP_PIN              GPIO_NUM_25
#define Y_DIRECTION_PIN         GPIO_NUM_27

#define Z_STEP_PIN              GPIO_NUM_17
#define Z_DIRECTION_PIN         GPIO_NUM_14

// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN    GPIO_NUM_12

#define SPINDLE_PWM_PIN         GPIO_NUM_19
#define SPINDLE_DIR_PIN         GPIO_NUM_18

#define COOLANT_FLOOD_PIN       GPIO_NUM_34
#define COOLANT_MIST_PIN        GPIO_NUM_36

#define X_LIMIT_PIN             GPIO_NUM_13
#define Y_LIMIT_PIN             GPIO_NUM_5
#define Z_LIMIT_PIN             GPIO_NUM_19
#define LIMIT_MASK              B111

#define PROBE_PIN               GPIO_NUM_39

// comment out #define IGNORE_CONTROL_PINS in config.h to use control pins
#define CONTROL_RESET_PIN       GPIO_NUM_2
#define CONTROL_FEED_HOLD_PIN   GPIO_NUM_4
#define CONTROL_CYCLE_START_PIN GPIO_NUM_35 // ESP32 needs external pullup
