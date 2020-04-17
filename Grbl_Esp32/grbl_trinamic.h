/*
	grbl_trinamic.h - Support for TMC2130 Stepper Drivers SPI Mode
	Part of Grbl_ESP32

	Copyright (c) 2019 Barton Dring for Buildlog.net LLC

	GrblESP32 is free software: you can redistribute it and/or modify
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
#define TRINAMIC_DEFAULT_TOFF 5


#ifndef GRBL_TRINAMIC_h
#define GRBL_TRINAMIC_h

#include "grbl.h"

// the default current sense resistors used on most driver modules
#define TMC2130_RSENSE_DEFAULT  0.11f
#define TMC5160_RSENSE_DEFAULT  0.075f

#ifdef USE_TRINAMIC
    #include <TMCStepper.h> // https://github.com/teemuatlut/TMCStepper
    void Trinamic_Init();
    void trinamic_change_settings();
    void trinamic_test_response(uint8_t result, const char* axis);
    void trinamic_stepper_enable(bool enable);
    #ifdef USE_MACHINE_TRINAMIC_INIT
        void machine_trinamic_setup();
    #endif

    uint8_t get_next_trinamic_driver_index();
#endif

#endif // GRBL_TRIAMINIC_h
