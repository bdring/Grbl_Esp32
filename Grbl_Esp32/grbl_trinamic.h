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

#ifndef GRBL_TRINAMIC_h
	#define GRBL_TRINAMIC_h

#include "grbl.h"

#ifdef USE_TRINAMIC
	#include <TMCStepper.h> // https://github.com/teemuatlut/TMCStepper
	void Trinamic_Init();
	void trinamic_change_settings();
#endif

#endif