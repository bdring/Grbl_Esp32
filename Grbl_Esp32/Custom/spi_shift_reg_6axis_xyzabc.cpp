/*
	custom_code_template.cpp (copy and use your machine name)
	Part of Grbl_ESP32

	copyright (c) 2020 -	Bart Dring. This file was intended for use on the ESP32

  ...add your date and name here.

	CPU. Do not use this with Grbl for atMega328P

	Grbl_ESP32 is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Grbl_ESP32 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Grbl.  If not, see <http://www.gnu.org/licenses/>.

	=======================================================================

This is a template for user-defined C++ code functions.  Grbl can be
configured to call some optional functions, enabled by #define statements
in the machine definition .h file.  Implement the functions thus enabled
herein.  The possible functions are listed and described below.

To use this file, copy it to a name of your own choosing, and also copy
Machines/template.h to a similar name.

Example:
Machines/my_machine.h
Custom/my_machine.cpp

Edit machine.h to include your Machines/my_machine.h file

Edit Machines/my_machine.h according to the instructions therein.

Fill in the function definitions below for the functions that you have
enabled with USE_ defines in Machines/my_machine.h

===============================================================================

*/

#ifdef USE_MACHINE_INIT
/*
machine_init() is called when Grbl_ESP32 first starts. You can use it to do any
special things your machine needs at startup.
*/
void machine_init() {
  // Enable steppers
  grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Machine Init");
  I2S_IOEXP_OUT_WRITE(X_ENABLE_PIN, LOW);
  I2S_IOEXP_OUT_WRITE(Y_ENABLE_PIN, LOW);
  I2S_IOEXP_OUT_WRITE(Z_ENABLE_PIN, LOW);
  I2S_IOEXP_OUT_WRITE(A_ENABLE_PIN, LOW);
  I2S_IOEXP_OUT_WRITE(B_ENABLE_PIN, LOW);
  I2S_IOEXP_OUT_WRITE(C_ENABLE_PIN, LOW);

/*

    // ==== Genaral I/O testing of PCB ==============

    pinMode(GPIO_NUM_12, OUTPUT);
    digitalWrite(GPIO_NUM_12, HIGH);

    pinMode(GPIO_NUM_13, OUTPUT);
    digitalWrite(GPIO_NUM_13, HIGH);

    pinMode(GPIO_NUM_14, OUTPUT);
    digitalWrite(GPIO_NUM_14, HIGH);

    pinMode(GPIO_NUM_15, OUTPUT);
    digitalWrite(GPIO_NUM_15, HIGH);

    pinMode(GPIO_NUM_27, OUTPUT);
    digitalWrite(GPIO_NUM_27, HIGH);

    I2S_IOEXP_OUT_WRITE(GPIO_NUM_I2S_IOEXP_24, HIGH);
    I2S_IOEXP_OUT_WRITE(GPIO_NUM_I2S_IOEXP_25, HIGH);
    I2S_IOEXP_OUT_WRITE(GPIO_NUM_I2S_IOEXP_26, HIGH);
    I2S_IOEXP_OUT_WRITE(GPIO_NUM_I2S_IOEXP_27, HIGH);
    I2S_IOEXP_OUT_WRITE(GPIO_NUM_I2S_IOEXP_28, HIGH);
    I2S_IOEXP_OUT_WRITE(GPIO_NUM_I2S_IOEXP_29, HIGH);
    I2S_IOEXP_OUT_WRITE(GPIO_NUM_I2S_IOEXP_30, HIGH);
    I2S_IOEXP_OUT_WRITE(GPIO_NUM_I2S_IOEXP_31, HIGH);
  */

#ifndef USE_TRINAMIC
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Set Microstepping Pins");

    pinMode(STEPPER_MS1, OUTPUT);
    digitalWrite(STEPPER_MS1, HIGH);

    pinMode(STEPPER_MS2, OUTPUT);
    digitalWrite(STEPPER_MS2, HIGH);

    pinMode(STEPPER_X_MS3, OUTPUT);
    digitalWrite(STEPPER_X_MS3, HIGH);

    pinMode(STEPPER_Y_MS3, OUTPUT);
    digitalWrite(STEPPER_Y_MS3, HIGH);

    pinMode(STEPPER_Z_MS3, OUTPUT);
    digitalWrite(STEPPER_Z_MS3, HIGH);

    pinMode(STEPPER_A_MS3, OUTPUT);
    digitalWrite(STEPPER_A_MS3, HIGH);

    pinMode(STEPPER_B_MS3, OUTPUT);
    digitalWrite(STEPPER_B_MS3, HIGH);

    pinMode(STEPPER_C_MS3, OUTPUT);
    digitalWrite(STEPPER_C_MS3, HIGH);

    // !RESET pin on steppers  (MISO On Schematic)
    pinMode(STEPPER_RESET, OUTPUT);
    digitalWrite(STEPPER_RESET, HIGH);

    // Note !SLEEP is set via jumper

#endif

}
#endif

#ifdef USE_MACHINE_TRINAMIC_INIT
/*
  machine_triaminic_setup() replaces the normal setup of trinamic
  drivers with your own code.  For example, you could setup StallGuard
*/
void machine_trinamic_setup() {
}
#endif

// If you add any additional functions specific to your machine that
// require calls from common code, guard their calls in the common code with
// #ifdef USE_WHATEVER and add function prototypes (also guarded) to grbl.h
