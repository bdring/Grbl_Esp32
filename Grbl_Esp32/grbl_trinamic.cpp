/*
	grbl_trinamic.cpp - Support for Trinamic Stepper Drivers SPI Mode
	using the TMCStepper library
	
	Part of Grbl_ESP32

	Copyright (c) 2019 Barton Dring for Buildlog.net LLC   	

	Grbl_ESP32 is free software: you can redistribute it and/or modify
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

#ifdef USE_TRINAMIC

// TODO try to use the #define ## method to clean this up
//#define DRIVER(driver, axis) driver##Stepper = TRINAMIC_axis## = driver##Stepper(axis##_CS_PIN, axis##_RSENSE);

#ifdef X_TRINAMIC
	#ifdef X_DRIVER_TMC2130	
		TMC2130Stepper TRINAMIC_X = TMC2130Stepper(X_CS_PIN, X_RSENSE);
	#endif
	#ifdef X_DRIVER_TMC2209
		TMC2209Stepper TRINAMIC_X = TMC2209Stepper(X_CS_PIN, X_RSENSE);
	#endif
	#ifdef X_DRIVER_TMC5160
		TMC5160Stepper TRINAMIC_X = TMC5160Stepper(X_CS_PIN, X_RSENSE);
	#endif
#endif

#ifdef Y_TRINAMIC
	#ifdef Y_DRIVER_TMC2130	
		TMC2130Stepper TRINAMIC_Y = TMC2130Stepper(Y_CS_PIN, Y_RSENSE);
	#endif
	#ifdef Y_DRIVER_TMC2209
		TMC2209Stepper TRINAMIC_Y = TMC2209Stepper(Y_CS_PIN, Y_RSENSE);
	#endif
	#ifdef Y_DRIVER_TMC5160
		TMC5160Stepper TRINAMIC_Y = TMC5160Stepper(Y_CS_PIN, Y_RSENSE);
	#endif
#endif

#ifdef Z_TRINAMIC
	#ifdef Z_DRIVER_TMC2130	
		TMC2130Stepper TRINAMIC_Z = TMC2130Stepper(Z_CS_PIN, Z_RSENSE);
	#endif
	#ifdef Z_DRIVER_TMC2209
		TMC2209Stepper TRINAMIC_Z = TMC2209Stepper(Z_CS_PIN, Z_RSENSE);
	#endif
	#ifdef Z_DRIVER_TMC5160
		TMC5160Stepper TRINAMIC_Z = TMC5160Stepper(Z_CS_PIN, Z_RSENSE);
	#endif
#endif

#ifdef A_TRINAMIC
	#ifdef A_DRIVER_TMC2130	
		TMC2130Stepper TRINAMIC_A = TMC2130Stepper(A_CS_PIN, A_RSENSE);
	#endif
	#ifdef A_DRIVER_TMC2209
		TMC2209Stepper TRINAMIC_A = TMC2209Stepper(A_CS_PIN, A_RSENSE);
	#endif
	#ifdef A_DRIVER_TMC5160
		TMC5160Stepper TRINAMIC_A = TMC5160Stepper(A_CS_PIN, A_RSENSE);
	#endif
#endif

#ifdef B_TRINAMIC
	#ifdef B_DRIVER_TMC2130	
		TMC2130Stepper TRINAMIC_B = TMC2130Stepper(B_CS_PIN, B_RSENSE);
	#endif
	#ifdef B_DRIVER_TMC2209
		TMC2209Stepper TRINAMIC_B = TMC2209Stepper(B_CS_PIN, B_RSENSE);
	#endif
	#ifdef B_DRIVER_TMC5160
		TMC5160Stepper TRINAMIC_B = TMC5160Stepper(B_CS_PIN, B_RSENSE);
	#endif
#endif

#ifdef C_TRINAMIC
	#ifdef C_DRIVER_TMC2130	
		TMC2130Stepper TRINAMIC_c = TMC2130Stepper(C_CS_PIN, C_RSENSE);
	#endif
	#ifdef C_DRIVER_TMC2209
		TMC2209Stepper TRINAMIC_C = TMC2209Stepper(C_CS_PIN, C_RSENSE);
	#endif
	#ifdef C_DRIVER_TMC5160
		TMC5160Stepper TRINAMIC_C = TMC5160Stepper(C_CS_PIN, C_RSENSE);
	#endif
#endif

// TODO ABC Axes

void Trinamic_Init()
{
    grbl_sendf(CLIENT_SERIAL, "[MSG:Using TMCStepper Library Ver 0x%06x]\r\n", TMCSTEPPER_VERSION);
	
	SPI.begin();
	
	#ifdef X_TRINAMIC
		TRINAMIC_X.begin(); // Initiate pins and registries
		TRINAMIC_X.toff(5);
		TRINAMIC_X.microsteps(X_MICROSTEPS);
		#ifdef X_HOLD_CURRENT
			TRINAMIC_X.rms_current(X_RMS_CURRENT, X_HOLD_CURRENT);
		#else
			TRINAMIC_X.rms_current(X_RMS_CURRENT); // default hold current is 0.5 or 50%
		#endif		
		TRINAMIC_X.en_pwm_mode(1);      // Enable extremely quiet stepping
		TRINAMIC_X.pwm_autoscale(1);
	#endif
	
	#ifdef Y_TRINAMIC
		TRINAMIC_Y.begin(); // Initiate pins and registries
		TRINAMIC_Y.toff(5);
		TRINAMIC_Y.microsteps(Y_MICROSTEPS);
		#ifdef Y_HOLD_CURRENT
			TRINAMIC_Y.rms_current(Y_RMS_CURRENT, Y_HOLD_CURRENT);
		#else
			TRINAMIC_Y.rms_current(Y_RMS_CURRENT); // default hold current is 0.5 or 50%
		#endif		
		TRINAMIC_Y.en_pwm_mode(1);      // Enable extremely quiet stepping
		TRINAMIC_Y.pwm_autoscale(1);		
	#endif
	
	#ifdef Z_TRINAMIC
		TRINAMIC_Z.begin(); // Initiate pins and registries
		TRINAMIC_Z.toff(5);
		TRINAMIC_Z.microsteps(Z_MICROSTEPS);
		TRINAMIC_Z.rms_current(Z_RMS_CURRENT);
		TRINAMIC_Z.en_pwm_mode(1);      // Enable extremely quiet stepping
		TRINAMIC_Z.pwm_autoscale(1);
	#endif
	
	#ifdef A_TRINAMIC
		TRINAMIC_A.begin(); // Initiate pins and registries
		TRINAMIC_A.toff(5);
		TRINAMIC_A.microsteps(A_MICROSTEPS);
		TRINAMIC_A.rms_current(A_RMS_CURRENT);
		TRINAMIC_A.en_pwm_mode(1);      // Enable extremely quiet stepping
		TRINAMIC_A.pwm_autoscale(1);
	#endif
	
	#ifdef B_TRINAMIC
		TRINAMIC_B.begin(); // Initiate pins and registries
		TRINAMIC_B.toff(5);
		TRINAMIC_B.microsteps(B_MICROSTEPS);
		TRINAMIC_B.rms_current(B_RMS_CURRENT);
		TRINAMIC_B.en_pwm_mode(1);      // Enable extremely quiet stepping
		TRINAMIC_B.pwm_autoscale(1);
	#endif
	
	#ifdef C_TRINAMIC
		TRINAMIC_C.begin(); // Initiate pins and registries
		TRINAMIC_C.toff(5);
		TRINAMIC_C.microsteps(C_MICROSTEPS);
		TRINAMIC_C.rms_current(C_RMS_CURRENT);
		TRINAMIC_C.en_pwm_mode(1);      // Enable extremely quiet stepping
		TRINAMIC_C.pwm_autoscale(1);
	#endif
	
	// TODO ABC Axes
}

#endif