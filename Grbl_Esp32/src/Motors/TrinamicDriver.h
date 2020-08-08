#pragma once

/*
    TrinamicDriver.h
    
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

#include "Motor.h"
#include "StandardStepper.h"
#include <TMCStepper.h>  // https://github.com/teemuatlut/TMCStepper

#define TRINAMIC_MODE_STEALTHCHOP 0  // very quiet
#define TRINAMIC_MODE_COOLSTEP 1     // everything runs cooler so higher current possible
#define TRINAMIC_MODE_STALLGUARD 2   // coolstep plus generates stall indication

#define NORMAL_TCOOLTHRS 0xFFFFF  // 20 bit is max
#define NORMAL_THIGH 0

#define TMC2130_RSENSE_DEFAULT 0.11f
#define TMC5160_RSENSE_DEFAULT 0.075f

#define TRINAMIC_SPI_FREQ 100000

#define TRINAMIC_FCLK 12700000.0  // Internal clock Approx (Hz) used to calculate TSTEP from homing rate

// ==== defaults OK to define them in your machine definition ====
#ifndef TRINAMIC_RUN_MODE
#    define TRINAMIC_RUN_MODE TRINAMIC_MODE_COOLSTEP
#endif

#ifndef TRINAMIC_HOMING_MODE
#    define TRINAMIC_HOMING_MODE TRINAMIC_RUN_MODE
#endif

#ifndef TRINAMIC_TOFF_DISABLE
#    define TRINAMIC_TOFF_DISABLE 0
#endif

#ifndef TRINAMIC_TOFF_STEALTHCHOP
#    define TRINAMIC_TOFF_STEALTHCHOP 5
#endif

#ifndef TRINAMIC_TOFF_COOLSTEP
#    define TRINAMIC_TOFF_COOLSTEP 3
#endif

namespace Motors {
    class TrinamicDriver : public StandardStepper {
    public:
        TrinamicDriver(uint8_t  axis_index,
                       uint8_t  step_pin,
                       uint8_t  dir_pin,
                       uint8_t  disable_pin,
                       uint8_t  cs_pin,
                       uint16_t driver_part_number,
                       float    r_sense,
                       int8_t   spi_index);

        void config_message();
        void init();
        void set_mode(bool isHoming);
        void read_settings();
        void trinamic_test_response();
        void trinamic_stepper_enable(bool enable);
        void debug_message();
        void set_homing_mode(uint8_t homing_mask, bool ishoming);
        void set_disable(bool disable);
        bool test();

    private:
        uint32_t calc_tstep(float speed, float percent);

        TMC2130Stepper* tmcstepper;  // all other driver types are subclasses of this one
        uint8_t         _homing_mode;
        uint8_t         cs_pin = UNDEFINED_PIN;  // The chip select pin (can be the same for daisy chain)
        uint16_t        _driver_part_number;     // example: use 2130 for TMC2130
        float           _r_sense;
        int8_t          spi_index;

    protected:
        uint8_t _mode;
        uint8_t _lastMode = 255;
    };
}