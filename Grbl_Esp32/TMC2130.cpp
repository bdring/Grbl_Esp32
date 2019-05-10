
/*
	TMC2130.cpp - Support for TMC2130 Stepper Drivers SPI Mode
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

#include "grbl.h"

#ifdef X_CS_PIN
	TMC2130Stepper TMC2130_X = TMC2130Stepper(X_CS_PIN);
#endif

#ifdef Y_CS_PIN
	TMC2130Stepper TMC2130_Y = TMC2130Stepper(Y_CS_PIN);
#endif

#ifdef Z_CS_PIN
	TMC2130Stepper TMC2130_Z = TMC2130Stepper(Z_CS_PIN);
#endif

void TMC2130_Init()
{
		#ifdef X_CS_PIN
			TMC2130_X.begin(); // Initiate pins and registries
			TMC2130_X.microsteps(32);
			TMC2130_X.setCurrent(200, 0.11, 0.5);
			TMC2130_X.stealthChop(1); // Enable extremely quiet stepping			
		#endif	
		
		#ifdef Y_CS_PIN
			TMC2130_Y.begin(); // Initiate pins and registries
			TMC2130_Y.microsteps(32);
			TMC2130_Y.setCurrent(200, 0.11, 0.5);
			TMC2130_Y.stealthChop(1); // Enable extremely quiet stepping
		#endif
		
		#ifdef Z_CS_PIN
			TMC2130_Z.begin(); // Initiate pins and registries
			TMC2130_Z.microsteps(32);
			TMC2130_Z.setCurrent(200, 0.11, 0.5);
			TMC2130_Z.stealthChop(1); // Enable extremely quiet stepping
		#endif
}