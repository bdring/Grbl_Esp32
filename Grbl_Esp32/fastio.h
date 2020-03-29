/*
    fastio.h
    Part of Grbl_ESP32

    Header for digital out wrapper that support I2S expander feature

    2020    - Michiyasu Odaki

    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Grbl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef fastio_h
#define fastio_h

#include <Arduino.h>

/**
 * Utility functions
 */

#ifdef USE_I2S_IOEXPANDER
    //
    // enable I2S I/O expander pin mapping.
    //
    #include "i2s_ioexpander.h"
    #define IS_I2S_EXPANDER_PIN(IO) (((IO) & ~0x7F))
    #define I2S_EXPANDER_PIN_INDEX(IO) ((IO) & 0x7F)
    // Read a pin wrapper
    #define READ(IO)                do{ IS_I2S_EXPANDER_PIN(IO) ?\
                                        i2s_state(I2S_EXPANDER_PIN_INDEX(IO)) : digitalRead(IO);\
                                    }while(0)
    // Write to a pin wrapper
    #define WRITE(IO, v)            do{ IS_I2S_EXPANDER_PIN(IO) ?\
                                        i2s_write(I2S_EXPANDER_PIN_INDEX(IO), v) : digitalWrite(IO, v);\
                                    }while(0)
    #define SET_INPUT(IO)           do{ if (!IS_I2S_EXPANDER_PIN(IO))\
                                        pinMode(IO, INPUT);\
                                    }while(0)
    // Set pin as input with pullup wrapper
    #define SET_INPUT_PULLUP(IO)    do{ if (!IS_I2S_EXPANDER_PIN(IO))\
                                        pinMode(IO, INPUT_PULLUP);\
                                    }while(0)    // Set pin as output wrapper
    #define SET_OUTPUT(IO)          do{ if (!IS_I2S_EXPANDER_PIN(IO))\
                                       pinMode(IO, OUTPUT);\
                                    }while(0)
    // Set pin as PWM
    #define SET_PWM(IO)             SET_OUTPUT(IO)
    // Set pin as output and init
    #define OUT_WRITE(IO,V)         do{ SET_OUTPUT(IO); WRITE(IO,V); }while(0)
    // PWM outputs
    #define PWM_PIN(P)              (P < 34 || P > 127) // NOTE Pins >= 34 are input only on ESP32, so they can't be used for output.
#else
    //
    // default implimentation
    //
    // Read a pin wrapper
    #define READ(IO)                digitalRead(IO)
    // Write to a pin wrapper
    #define WRITE(IO, v)            digitalWrite(IO, v)
    // Set pin as input wrapper
    #define SET_INPUT(IO)           pinMode(IO, INPUT)
    // Set pin as input with pullup wrapper
    #define SET_INPUT_PULLUP(IO)    pinMode(IO, INPUT_PULLUP)
    // Set pin as output wrapper
    #define SET_OUTPUT(IO)          pinMode(IO, OUTPUT)
    // Set pin as PWM
    #define SET_PWM(IO)             pinMode(IO, OUTPUT)
    // Set pin as output and init
    #define OUT_WRITE(IO,V)         do{ pinMode(IO, OUTPUT); WRITE(IO,V); }while(0)
    // PWM outputs
    #define PWM_PIN(P)              (P < 34) // NOTE Pins >= 34 are input only on ESP32, so they can't be used for output.
#endif

// digitalRead/Write wrappers
#define extDigitalRead(IO)      digitalRead(IO)
#define extDigitalWrite(IO,V)   digitalWrite(IO,V)


// Toggle pin value
#define TOGGLE(IO)              WRITE(IO, !READ(IO))

#endif
