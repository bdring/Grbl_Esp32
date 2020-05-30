

#define SOLENOID_PULSE_FREQ 5000 // 50Hz ...This is a standard analog servo value. Digital ones can repeat faster

#define SOLENOID_PULSE_RES_BITS 10 // bits of resolution of PWM (16 is max)

#ifndef SOLENOID_MIN_PULSE
    #define SOLENOID_MIN_PULSE    0
#endif

#ifndef SOLENOID_MAX_PULSE
    #define SOLENOID_MAX_PULSE    1024
#endif

#ifndef SOLENOID_HOLD_DELAY
    #define SOLENOID_HOLD_DELAY   10 // task counts
#endif

#ifndef SOLENOID_HOLD_PERCENTAGE
    #define SOLENOID_HOLD_PERCENTAGE  60.0 // percent of SOLENOID_MAX_PULSE
#endif

#ifndef SOLENOID_ALLOW_DISABLE
    #define SOLENOID_ALLOW_DISABLE  false
#endif