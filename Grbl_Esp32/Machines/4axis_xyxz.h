/*
 * 4 Achsen CNC Fraese von Jens
 * Infos zur Benutzung einer Spindel mit Relais https://github.com/bdring/Grbl_Esp32/wiki/Spindle-Types
 */

#define MACHINE_NAME            "MACHINE_ESP32 Jens XYZA"

#ifdef N_AXIS
#undef N_AXIS
#endif

/*
 * Moechte ich mit 3 oder 4 Achsen arbeiten?
 * Beides ist eingerichtet.
 */
// ######################################
#define N_AXIS 3
// #######################################

//#define CUSTOM_CODE_FILENAME "Custom/4axis_xyxz.cpp"

/*
 * Spindeltyp definieren, ich nutze ein Relais
 */
#define SPINDLE_TYPE SpindleType::RELAY
#define SPINDLE_OUTPUT_PIN GPIO_NUM_3

/*
 * Zuordnung Schrittmotoren
 * Analog zu den Pins des Boards. Am Besten in KICAD ablesen.
 * Hier wird Version "Fräse 2.6" genutzt
 */
#define STEPPERS_DISABLE_PIN    GPIO_NUM_13 //ok

#define X_STEP_PIN              GPIO_NUM_12
#define X_DIRECTION_PIN         GPIO_NUM_14
#define X_LIMIT_PIN             GPIO_NUM_17

#define Y_STEP_PIN              GPIO_NUM_26
#define Y_DIRECTION_PIN         GPIO_NUM_15
#define Y_LIMIT_PIN             GPIO_NUM_4

#define Z_STEP_PIN              GPIO_NUM_27
#define Z_DIRECTION_PIN         GPIO_NUM_33
#define Z_LIMIT_PIN             GPIO_NUM_16

// Falls die 4. Achse genutzt wird
#if (N_AXIS == 4)
    #define A_STEP_PIN          GPIO_NUM_25
    #define A_DIRECTION_PIN     GPIO_NUM_22
    #define A_LIMIT_PIN         GPIO_NUM_21

    /* Zuordnung Endschalter
     * The 1 bits in LIMIT_MASK set the axes that have limit switches
     * X, Y, Z, A do, the LIMIT_MASK value would be B1111
     */
    #define LIMIT_MASK          B1111
#else
    // bei 3 Achsen
    #define LIMIT_MASK          B111
#endif

/*
 * Control pins
 */
#define PROBE_PIN               GPIO_NUM_35 //ok
//#define CONTROL_SAFETY_DOOR_PIN GPIO_NUM_36  // needs external pullup
#define CONTROL_RESET_PIN       GPIO_NUM_34  // needs external pullup
#define CONTROL_FEED_HOLD_PIN   GPIO_NUM_36  // needs external pullup
#define CONTROL_CYCLE_START_PIN GPIO_NUM_39  // needs external pullup

#define COOLANT_FLOOD_PIN       GPIO_NUM_2

/* Normally Grbl_ESP32 ignores tool changes.
 * It just tracks the current tool number.
 * If you put #define USE_TOOL_CHANGE in you header file,
 * it will call a function void user_tool_change(uint8_t new_tool) when it sees the M6 gcode command.
 */


//#define USE_MACHINE_INIT
//#define USE_TOOL_CHANGE