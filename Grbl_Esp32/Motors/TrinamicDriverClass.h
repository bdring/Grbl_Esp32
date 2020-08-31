#define TRINAMIC_RUN_MODE_STEALTHCHOP   0   // very quiet
#define TRINAMIC_RUN_MODE_COOLSTEP      1   // everything runs cooler so higher current possible
#define TRINAMIC_RUN_MODE_STALLGUARD    2   // everything runs cooler so higher current possible

#define TRINAMIC_HOMING_NONE        0
#define TRINAMIC_HOMING_STALLGUARD  1

#define NORMAL_TCOOLTHRS 		0xFFFFF // 20 bit is max
#define NORMAL_THIGH 			0

#define TMC2208_RSENSE_DEFAULT  0.11f
#define TMC2209_RSENSE_DEFAULT  0.11f
#define TMC2130_RSENSE_DEFAULT  0.11f
#define TMC5160_RSENSE_DEFAULT  0.075f

// ============ defaults =================
#ifndef TRINAMIC_RUN_MODE
    #define TRINAMIC_RUN_MODE           TRINAMIC_RUN_MODE_COOLSTEP
#endif

#ifndef TRINAMIC_HOMING_MODE
    #define TRINAMIC_HOMING_MODE        TRINAMIC_HOMING_NONE
#endif


#ifndef TRINAMICDRIVERCLASS_H
#define TRINAMICDRIVERCLASS_H

#include "MotorClass.h"
#include <TMCStepper.h> // https://github.com/teemuatlut/TMCStepper




#endif