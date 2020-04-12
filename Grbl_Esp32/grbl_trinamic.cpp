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
/*
	The drivers can use SPI daisy chaining to allow the use of a single CS_PIN.
	The PCB must be designed for this, with SDO pins connected to the
	next driver's SDI pin and the final SDO going back to the CPU.

	To set this up, #define TRINAMIC_DAISY_CHAIN in your machine definition
	file (Machines/something.h).

	Set the CS_PIN definition for every axis to the same pin, like this...
	#define X_CS_PIN GPIO_NUM_17
	#define Y_CS_PIN GPIO_NUM_17
	...etc.

	Indexes are assigned to each axis in daisy chain mode as shown below.
	This assumes your first SPI driver axis is X and there are no gaps until
	the last SPI driver.

*/


// TODO try to use the #define ## method to clean this up
//#define DRIVER(driver, axis) driver##Stepper = TRINAMIC_axis## = driver##Stepper(axis##_CS_PIN, axis##_RSENSE);

#ifdef X_TRINAMIC
    #ifdef X_DRIVER_TMC2130
        TMC2130Stepper TRINAMIC_X = TMC2130Stepper(X_CS_PIN, X_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef X_DRIVER_TMC2209
        TMC2209Stepper TRINAMIC_X = TMC2209Stepper(X_CS_PIN, X_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef X_DRIVER_TMC5160
        TMC5160Stepper TRINAMIC_X = TMC5160Stepper(X_CS_PIN, X_RSENSE, get_next_trinamic_driver_index());
    #endif
#endif

#ifdef X2_TRINAMIC
    #ifdef X2_DRIVER_TMC2130
        TMC2130Stepper TRINAMIC_X2 = TMC2130Stepper(X2_CS_PIN, X2_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef X2_DRIVER_TMC2209
        TMC2209Stepper TRINAMIC_X2 = TMC2209Stepper(X2_CS_PIN, X2_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef X2_DRIVER_TMC5160
        TMC5160Stepper TRINAMIC_X2 = TMC5160Stepper(X2_CS_PIN, X2_RSENSE, get_next_trinamic_driver_index());
    #endif
#endif

#ifdef Y_TRINAMIC
    #ifdef Y_DRIVER_TMC2130
        TMC2130Stepper TRINAMIC_Y = TMC2130Stepper(Y_CS_PIN, Y_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef Y_DRIVER_TMC2209
        TMC2209Stepper TRINAMIC_Y = TMC2209Stepper(Y_CS_PIN, Y_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef Y_DRIVER_TMC5160
        TMC5160Stepper TRINAMIC_Y = TMC5160Stepper(Y_CS_PIN, Y_RSENSE, get_next_trinamic_driver_index());
    #endif
#endif
#ifdef Y2_TRINAMIC
    #ifdef Y2_DRIVER_TMC2130
        TMC2130Stepper TRINAMIC_Y2 = TMC2130Stepper(Y2_CS_PIN, Y2_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef Y2_DRIVER_TMC2209
        TMC2209Stepper TRINAMIC_Y2 = TMC2209Stepper(Y2_CS_PIN, Y2_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef Y2_DRIVER_TMC5160
        TMC5160Stepper TRINAMIC_Y2 = TMC5160Stepper(Y2_CS_PIN, Y2_RSENSE, get_next_trinamic_driver_index());
    #endif
#endif


#ifdef Z_TRINAMIC
    #ifdef Z_DRIVER_TMC2130
        TMC2130Stepper TRINAMIC_Z = TMC2130Stepper(Z_CS_PIN, Z_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef Z_DRIVER_TMC2209
        TMC2209Stepper TRINAMIC_Z = TMC2209Stepper(Z_CS_PIN, Z_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef Z_DRIVER_TMC5160
        TMC5160Stepper TRINAMIC_Z = TMC5160Stepper(Z_CS_PIN, Z_RSENSE, get_next_trinamic_driver_index());
    #endif
#endif
#ifdef Z2_TRINAMIC
    #ifdef Z2_DRIVER_TMC2130
        TMC2130Stepper TRINAMIC_Z2 = TMC2130Stepper(Z2_CS_PIN, Z2_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef Z2_DRIVER_TMC2209
        TMC2209Stepper TRINAMIC_Z2 = TMC2209Stepper(Z2_CS_PIN, Z2_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef Z2_DRIVER_TMC5160
        TMC5160Stepper TRINAMIC_Z2 = TMC5160Stepper(Z2_CS_PIN, Z2_RSENSE, get_next_trinamic_driver_index());
    #endif
#endif

#ifdef A_TRINAMIC
    #ifdef A_DRIVER_TMC2130
        TMC2130Stepper TRINAMIC_A = TMC2130Stepper(A_CS_PIN, A_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef A_DRIVER_TMC2209
        TMC2209Stepper TRINAMIC_A = TMC2209Stepper(A_CS_PIN, A_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef A_DRIVER_TMC5160
        TMC5160Stepper TRINAMIC_A = TMC5160Stepper(A_CS_PIN, A_RSENSE, get_next_trinamic_driver_index());
    #endif
#endif
#ifdef A2_TRINAMIC
    #ifdef A2_DRIVER_TMC2130
        TMC2130Stepper TRINAMIC_A2 = TMC2130Stepper(A2_CS_PIN, A2_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef A2_DRIVER_TMC2209
        TMC2209Stepper TRINAMIC_A2 = TMC2209Stepper(A2_CS_PIN, A_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef A2_DRIVER_TMC5160
        TMC5160Stepper TRINAMIC_A2 = TMC5160Stepper(A2_CS_PIN, A2_RSENSE, get_next_trinamic_driver_index());
    #endif
#endif

#ifdef B_TRINAMIC
    #ifdef B_DRIVER_TMC2130
        TMC2130Stepper TRINAMIC_B = TMC2130Stepper(B_CS_PIN, B_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef B_DRIVER_TMC2209
        TMC2209Stepper TRINAMIC_B = TMC2209Stepper(B_CS_PIN, B_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef B_DRIVER_TMC5160
        TMC5160Stepper TRINAMIC_B = TMC5160Stepper(B_CS_PIN, B_RSENSE, get_next_trinamic_driver_index());
    #endif
#endif
#ifdef B2_TRINAMIC
    #ifdef B2_DRIVER_TMC2130
        TMC2130Stepper TRINAMIC_B2 = TMC2130Stepper(B2_CS_PIN, B2_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef B2_DRIVER_TMC2209
        TMC2209Stepper TRINAMIC_B2 = TMC2209Stepper(B2_CS_PIN, B2_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef B2_DRIVER_TMC5160
        TMC5160Stepper TRINAMIC_B2 = TMC5160Stepper(B2_CS_PIN, B2_RSENSE, get_next_trinamic_driver_index());
    #endif
#endif

#ifdef C_TRINAMIC
    #ifdef C_DRIVER_TMC2130
        TMC2130Stepper TRINAMIC_c = TMC2130Stepper(C_CS_PIN, C_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef C_DRIVER_TMC2209
        TMC2209Stepper TRINAMIC_C = TMC2209Stepper(C_CS_PIN, C_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef C_DRIVER_TMC5160
        TMC5160Stepper TRINAMIC_C = TMC5160Stepper(C_CS_PIN, C_RSENSE, get_next_trinamic_driver_index());
    #endif
#endif
#ifdef C2_TRINAMIC
    #ifdef C2_DRIVER_TMC2130
        TMC2130Stepper TRINAMIC_C2 = TMC2130Stepper(C2_CS_PIN, C2_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef C2_DRIVER_TMC2209
        TMC2209Stepper TRINAMIC_C2 = TMC2209Stepper(C2_CS_PIN, C2_RSENSE, get_next_trinamic_driver_index());
    #endif
    #ifdef C2_DRIVER_TMC5160
        TMC5160Stepper TRINAMIC_C2 = TMC5160Stepper(C2_CS_PIN, C2_RSENSE, get_next_trinamic_driver_index());
    #endif
#endif

void Trinamic_Init() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "TMCStepper Init using Library Ver 0x%06x", TMCSTEPPER_VERSION);
    SPI.begin();
#ifdef USE_MACHINE_TRINAMIC_INIT
    machine_trinamic_setup();
    return;
#endif
#ifdef X_TRINAMIC    
    TRINAMIC_X.begin(); // Initiate pins and registries
    trinamic_test_response(TRINAMIC_X.test_connection(), "X");    
    TRINAMIC_X.toff(TRINAMIC_DEFAULT_TOFF);
    TRINAMIC_X.microsteps(settings.microsteps[X_AXIS]);
    TRINAMIC_X.rms_current(settings.current[X_AXIS] * 1000.0, settings.hold_current[X_AXIS] / 100.0);
    TRINAMIC_X.en_pwm_mode(1);      // Enable extremely quiet stepping
    TRINAMIC_X.pwm_autoscale(1);
#endif
#ifdef X2_TRINAMIC    
    TRINAMIC_X2.begin(); // Initiate pins and registries
    trinamic_test_response(TRINAMIC_X2.test_connection(), "X2");    
    TRINAMIC_X2.toff(TRINAMIC_DEFAULT_TOFF);
    TRINAMIC_X2.microsteps(settings.microsteps[X_AXIS]);
    TRINAMIC_X2.rms_current(settings.current[X_AXIS] * 1000.0, settings.hold_current[X_AXIS] / 100.0);
    TRINAMIC_X2.en_pwm_mode(1);    // Enable extremely quiet stepping
    TRINAMIC_X2.pwm_autoscale(1);
#endif

#ifdef Y_TRINAMIC
    TRINAMIC_Y.begin(); // Initiate pins and registries
    trinamic_test_response(TRINAMIC_Y.test_connection(), "Y");
    TRINAMIC_Y.toff(TRINAMIC_DEFAULT_TOFF);
    TRINAMIC_Y.microsteps(settings.microsteps[Y_AXIS]);
    TRINAMIC_Y.rms_current(settings.current[Y_AXIS] * 1000.0, settings.hold_current[Y_AXIS] / 100.0);
    TRINAMIC_Y.en_pwm_mode(1);      // Enable extremely quiet stepping
    TRINAMIC_Y.pwm_autoscale(1);
#endif
#ifdef Y2_TRINAMIC
    TRINAMIC_Y2.begin(); // Initiate pins and registries
    trinamic_test_response(TRINAMIC_Y2.test_connection(), "Y2");
    TRINAMIC_Y2.toff(TRINAMIC_DEFAULT_TOFF);
    TRINAMIC_Y2.microsteps(settings.microsteps[Y_AXIS]);
    TRINAMIC_Y2.rms_current(settings.current[Y_AXIS] * 1000.0, settings.hold_current[Y_AXIS] / 100.0);
    TRINAMIC_Y2.en_pwm_mode(1);      // Enable extremely quiet stepping
    TRINAMIC_Y2.pwm_autoscale(1);
#endif

#ifdef Z_TRINAMIC
    TRINAMIC_Z.begin(); // Initiate pins and registries
    trinamic_test_response(TRINAMIC_Z.test_connection(), "Z");
    TRINAMIC_Z.toff(TRINAMIC_DEFAULT_TOFF);
    TRINAMIC_Z.microsteps(settings.microsteps[Z_AXIS]);
    TRINAMIC_Z.rms_current(settings.current[Z_AXIS] * 1000.0, settings.hold_current[Z_AXIS] / 100.0);
    TRINAMIC_Z.en_pwm_mode(1);      // Enable extremely quiet stepping
    TRINAMIC_Z.pwm_autoscale(1);
#endif
#ifdef Z2_TRINAMIC
    TRINAMIC_Z2.begin(); // Initiate pins and registries
    trinamic_test_response(TRINAMIC_Z2.test_connection(), "Z2");
    TRINAMIC_Z2.toff(TRINAMIC_DEFAULT_TOFF);
    TRINAMIC_Z2.microsteps(settings.microsteps[Z_AXIS]);
    TRINAMIC_Z2.rms_current(settings.current[Z_AXIS] * 1000.0, settings.hold_current[Z_AXIS] / 100.0);
    TRINAMIC_Z2.en_pwm_mode(1);      // Enable extremely quiet stepping
    TRINAMIC_Z2.pwm_autoscale(1);
#endif

#ifdef A_TRINAMIC
    TRINAMIC_A.begin(); // Initiate pins and registries
    trinamic_test_response(TRINAMIC_A.test_connection(), "A");
    TRINAMIC_A.toff(TRINAMIC_DEFAULT_TOFF);
    TRINAMIC_A.microsteps(settings.microsteps[A_AXIS]);
    TRINAMIC_A.rms_current(settings.current[A_AXIS] * 1000.0, settings.hold_current[A_AXIS] / 100.0);
    TRINAMIC_A.en_pwm_mode(1);      // Enable extremely quiet stepping
    TRINAMIC_A.pwm_autoscale(1);
#endif
#ifdef A2_TRINAMIC
    TRINAMIC_A2.begin(); // Initiate pins and registries
    trinamic_test_response(TRINAMIC_A2.test_connection(), "A2");
    TRINAMIC_A2.toff(TRINAMIC_DEFAULT_TOFF);
    TRINAMIC_A2.microsteps(settings.microsteps[A_AXIS]);
    TRINAMIC_A2.rms_current(settings.current[A_AXIS] * 1000.0, settings.hold_current[A_AXIS] / 100.0);
    TRINAMIC_A2.en_pwm_mode(1);      // Enable extremely quiet stepping
    TRINAMIC_A2.pwm_autoscale(1);
#endif

#ifdef B_TRINAMIC
    TRINAMIC_B.begin(); // Initiate pins and registries
    trinamic_test_response(TRINAMIC_B.test_connection(), "B");
    TRINAMIC_B.toff(TRINAMIC_DEFAULT_TOFF);
    TRINAMIC_B.microsteps(settings.microsteps[B_AXIS]);
    TRINAMIC_B.rms_current(settings.current[B_AXIS] * 1000.0, settings.hold_current[B_AXIS] / 100.0);
    TRINAMIC_B.en_pwm_mode(1);      // Enable extremely quiet stepping
    TRINAMIC_B.pwm_autoscale(1);
#endif
#ifdef B2_TRINAMIC
    TRINAMIC_B2.begin(); // Initiate pins and registries
    trinamic_test_response(TRINAMIC_B2.test_connection(), "B2");
    TRINAMIC_B2.toff(TRINAMIC_DEFAULT_TOFF);
    TRINAMIC_B2.microsteps(settings.microsteps[B_AXIS]);
    TRINAMIC_B2.rms_current(settings.current[B_AXIS] * 1000.0, settings.hold_current[B_AXIS] / 100.0);
    TRINAMIC_B2.en_pwm_mode(1);      // Enable extremely quiet stepping
    TRINAMIC_B2.pwm_autoscale(1);
#endif

#ifdef C_TRINAMIC
    TRINAMIC_C.begin(); // Initiate pins and registries
    trinamic_test_response(TRINAMIC_C.test_connection(), "C");
    TRINAMIC_C.toff(TRINAMIC_DEFAULT_TOFF);
    TRINAMIC_C.microsteps(settings.microsteps[C_AXIS]);
    TRINAMIC_C.rms_current(settings.current[C_AXIS] * 1000.0, settings.hold_current[C_AXIS] / 100.0);
    TRINAMIC_C.en_pwm_mode(1);      // Enable extremely quiet stepping
    TRINAMIC_C.pwm_autoscale(1);
#endif
#ifdef C2_TRINAMIC
    TRINAMIC_C2.begin(); // Initiate pins and registries
    trinamic_test_response(TRINAMIC_C2.test_connection(), "C2");
    TRINAMIC_C2.toff(TRINAMIC_DEFAULT_TOFF);
    TRINAMIC_C2.microsteps(settings.microsteps[C_AXIS]);
    TRINAMIC_C2.rms_current(settings.current[C_AXIS] * 1000.0, settings.hold_current[C_AXIS] / 100.0);
    TRINAMIC_C2.en_pwm_mode(1);      // Enable extremely quiet stepping
    TRINAMIC_C2.pwm_autoscale(1);
#endif
}

// Call this function called whenever $$ settings that affect the drivers are changed
void trinamic_change_settings() {
#ifdef X_TRINAMIC
    TRINAMIC_X.microsteps(settings.microsteps[X_AXIS]);
    TRINAMIC_X.rms_current(settings.current[X_AXIS] * 1000.0, settings.hold_current[X_AXIS] / 100.0);
#endif
#ifdef X2_TRINAMIC
    TRINAMIC_X2.microsteps(settings.microsteps[X_AXIS]);
    TRINAMIC_X2.rms_current(settings.current[X_AXIS] * 1000.0, settings.hold_current[X_AXIS] / 100.0);
#endif

#ifdef Y_TRINAMIC
    TRINAMIC_Y.microsteps(settings.microsteps[Y_AXIS]);
    TRINAMIC_Y.rms_current(settings.current[Y_AXIS] * 1000.0, settings.hold_current[Y_AXIS] / 100.0);
#endif
#ifdef Y2_TRINAMIC
    TRINAMIC_Y2.microsteps(settings.microsteps[Y_AXIS]);
    TRINAMIC_Y2.rms_current(settings.current[Y_AXIS] * 1000.0, settings.hold_current[Y_AXIS] / 100.0);
#endif


#ifdef Z_TRINAMIC
    TRINAMIC_Z.microsteps(settings.microsteps[Z_AXIS]);
    TRINAMIC_Z.rms_current(settings.current[Z_AXIS] * 1000.0, settings.hold_current[Z_AXIS] / 100.0);
#endif
#ifdef Z2_TRINAMIC
    TRINAMIC_Z2.microsteps(settings.microsteps[Z_AXIS]);
    TRINAMIC_Z2.rms_current(settings.current[Z_AXIS] * 1000.0, settings.hold_current[Z_AXIS] / 100.0);
#endif

#ifdef A_TRINAMIC
    TRINAMIC_A.microsteps(settings.microsteps[A_AXIS]);
    TRINAMIC_A.rms_current(settings.current[A_AXIS] * 1000.0, settings.hold_current[A_AXIS] / 100.0);
#endif
#ifdef A2_TRINAMIC
    TRINAMIC_A2.microsteps(settings.microsteps[A_AXIS]);
    TRINAMIC_A2.rms_current(settings.current[A_AXIS] * 1000.0, settings.hold_current[A_AXIS] / 100.0);
#endif

#ifdef B_TRINAMIC
    TRINAMIC_B.microsteps(settings.microsteps[B_AXIS]);
    TTRINAMIC_B.rms_current(settings.current[B_AXIS] * 1000.0, settings.hold_current[B_AXIS] / 100.0);
#endif
#ifdef B2_TRINAMIC
    TRINAMIC_B2.microsteps(settings.microsteps[B_AXIS]);
    TTRINAMIC_B2.rms_current(settings.current[B_AXIS] * 1000.0, settings.hold_current[B_AXIS] / 100.0);
#endif

#ifdef C_TRINAMIC
    TRINAMIC_C.microsteps(settings.microsteps[C_AXIS]);
    TRINAMIC_C.rms_current(settings.current[C_AXIS] * 1000.0, settings.hold_current[C_AXIS] / 100.0);
#endif
#ifdef C2_TRINAMIC
    TRINAMIC_C2.microsteps(settings.microsteps[C_AXIS]);
    TRINAMIC_C2.rms_current(settings.current[C_AXIS] * 1000.0, settings.hold_current[C_AXIS] / 100.0);
#endif

}

// Display the response of the attempt to connect to a Trinamic driver
void trinamic_test_response(uint8_t result, const char* axis) {
    if (result) {
        switch (result) {
        case 1:
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%s Trinamic driver test failed. Check connection", axis);
            break;
        case 2:
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%s Trinamic driver test failed. Check motor power", axis);
            break;
        }
    } else
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%s Trinamic driver test passed", axis);
}

void trinamic_stepper_enable(bool enable) {
    // Trinamic_Init() has already enabled the drivers in case #define USE_TRINAMIC_ENABLE is not used
    // so the previous_state is set accordingly
    static bool previous_state = true;
    uint8_t toff;
    if (enable == previous_state)
        return;
    previous_state = enable;
    if (enable)
        toff = TRINAMIC_DEFAULT_TOFF;
    else
        toff = 0; // diables driver
#ifdef X_TRINAMIC
    TRINAMIC_X.toff(toff);
#endif
#ifdef X2_TRINAMIC
    TRINAMIC_X2.toff(toff);
#endif

#ifdef Y_TRINAMIC
    TRINAMIC_Y.toff(toff);
#endif
#ifdef Y2_TRINAMIC
    TRINAMIC_Y2.toff(toff);
#endif

#ifdef Z_TRINAMIC
    TRINAMIC_Z.toff(toff);
#endif
#ifdef Z2_TRINAMIC
    TRINAMIC_Z2.toff(toff);
#endif

#ifdef A_TRINAMIC
    TRINAMIC_A.toff(toff);
#endif
#ifdef A2_TRINAMIC
    TRINAMIC_A2.toff(toff);
#endif

#ifdef B_TRINAMIC
    TRINAMIC_B.toff(toff);
#endif
#ifdef B2_TRINAMIC
    TRINAMIC_B2.toff(toff);
#endif

#ifdef C_TRINAMIC
    TRINAMIC_C.toff(toff);
#endif
#ifdef C2_TRINAMIC
    TRINAMIC_C2.toff(toff);
#endif

}

// returns the next spi index. We cannot preassign to axes because ganged (X2 type axes) might
// need to be inserted into the order of axes.
uint8_t get_next_trinamic_driver_index() {
    static uint8_t index = 1; // they start at 1
    #ifndef TRINAMIC_DAISY_CHAIN
        return -1;
    #else
        return index++;
    #endif
}

#endif
