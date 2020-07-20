/*
    TrinamicDriverClass.h
    
    Part of Grbl_ESP32

    2020 -	Bart Dring
    
    Grbl is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    Grbl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#define TRINAMIC_MODE_STEALTHCHOP   0   // very quiet
#define TRINAMIC_MODE_COOLSTEP      1   // everything runs cooler so higher current possible
#define TRINAMIC_MODE_STALLGUARD    2   // coolstep plus generates stall indication

#define NORMAL_TCOOLTHRS 		0xFFFFF // 20 bit is max
#define NORMAL_THIGH 			0

#define TMC2130_RSENSE_DEFAULT  0.11f
#define TMC5160_RSENSE_DEFAULT  0.075f

#define TRINAMIC_SPI_FREQ 100000

#define TRINAMIC_FCLK       12700000.0 // Internal clock Approx (Hz) used to calculate TSTEP from homing rate

// ==== defaults OK to define them in your machine definition ====
#ifndef TRINAMIC_RUN_MODE
    #define TRINAMIC_RUN_MODE           TRINAMIC_MODE_COOLSTEP
#endif

#ifndef TRINAMIC_HOMING_MODE
    #define TRINAMIC_HOMING_MODE        TRINAMIC_RUN_MODE
#endif


#ifndef TRINAMIC_TOFF_DISABLE
    #define TRINAMIC_TOFF_DISABLE       0
#endif

#ifndef TRINAMIC_TOFF_STEALTHCHOP
    #define TRINAMIC_TOFF_STEALTHCHOP   5
#endif

#ifndef TRINAMIC_TOFF_COOLSTEP
    #define TRINAMIC_TOFF_COOLSTEP      3
#endif



#ifndef TRINAMICDRIVERCLASS_H
#define TRINAMICDRIVERCLASS_H

#include "MotorClass.h"
#include <TMCStepper.h> // https://github.com/teemuatlut/TMCStepper

#endif
