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

//#define TRINAMIC_MODE_STEALTHCHOP 0  // very quiet
//#define TRINAMIC_MODE_COOLSTEP 1     // everything runs cooler so higher current possible
//#define TRINAMIC_MODE_STALLGUARD 2   // coolstep plus generates stall indication

const float TMC2130_RSENSE_DEFAULT = 0.11f;
const float TMC5160_RSENSE_DEFAULT = 0.075f;

const int NORMAL_TCOOLTHRS = 0xFFFFF;  // 20 bit is max
const int NORMAL_THIGH     = 0;

const int TRINAMIC_SPI_FREQ = 100000;

const double TRINAMIC_FCLK = 12700000.0;  // Internal clock Approx (Hz) used to calculate TSTEP from homing rate

// ==== defaults OK to define them in your machine definition ====

#ifndef TRINAMIC_RUN_MODE
#    define TRINAMIC_RUN_MODE TrinamicMode ::StealthChop
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
        TrinamicDriver(uint8_t axis_index, Pin step_pin, Pin dir_pin, Pin disable_pin, Pin cs_pin, MotorType driver_part_number, float r_sense) :
            TrinamicDriver(axis_index, step_pin, dir_pin, disable_pin, cs_pin, driver_part_number, r_sense, get_next_index()) {}

        TrinamicDriver(uint8_t  axis_index,
                       Pin      step_pin,
                       Pin      dir_pin,
                       Pin      disable_pin,
                       Pin      cs_pin,
                       MotorType driver_part_number,
                       float    r_sense,
                       int8_t   spi_index);

        // Overrides for inherited methods
        void init() override;
        void read_settings() override;
        bool set_homing_mode(bool ishoming) override;
        void set_disable(bool disable) override;

        void debug_message();

    private:
        uint32_t calc_tstep(float speed, float percent);

        TMC2130Stepper* tmcstepper;  // all other driver types are subclasses of this one
        TrinamicMode    _homing_mode;
        Pin             _cs_pin = Pin::UNDEFINED;  // The chip select pin (can be the same for daisy chain)
        MotorType        _driver_part_number;       // example: use 2130 for TMC2130
        float           _r_sense;
        int8_t          _spi_index;
        bool            _has_errors;
        bool            _disabled;

        TrinamicMode _mode = TrinamicMode::None;
        bool         test();
        void         set_mode(bool isHoming);
        void         trinamic_test_response();
        void         trinamic_stepper_enable(bool enable);

        bool report_open_load(TMC2130_n ::DRV_STATUS_t status);
        bool report_short_to_ground(TMC2130_n ::DRV_STATUS_t status);
        bool report_over_temp(TMC2130_n ::DRV_STATUS_t status);
        bool report_short_to_ps(TMC2130_n ::DRV_STATUS_t status);

        uint8_t get_next_index();

        // Linked list of Trinamic driver instances, used by the
        // StallGuard reporting task.
        static TrinamicDriver* List;
        TrinamicDriver*        link;
        static void            readSgTask(void*);

    protected:
        void config_message() override;
    };
}
