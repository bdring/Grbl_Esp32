/*
	Motors.cpp
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
	TODO
		Make sure public/private/protected is cleaned up.
		Only a few Unipolar axes have been setup in init()
		Get rid of Z_SERVO, just reply on Z_SERVO_PIN
		Deal with custom machine ... machine_trinamic_setup();
		Class is ready to deal with non SPI pins, but they have not been needed yet.
			It would be nice in the config message though
	Testing
		Done (success)
			3 Axis (3 Standard Steppers)
			MPCNC (ganged with shared direction pin)
			TMC2130 Pen Laser (trinamics, stallguard tuning)
			Unipolar
		TODO
			4 Axis SPI (Daisy Chain, Ganged with unique direction pins)
	Reference
		TMC2130 Datasheet https://www.trinamic.com/fileadmin/assets/Products/ICs_Documents/TMC2130_datasheet.pdf
*/

#include "Motors.h"

#include "Motor.h"
#include "../Grbl.h"

#include "NullMotor.h"
#include "StandardStepper.h"
#include "UnipolarMotor.h"
#include "RcServo.h"
#include "Dynamixel2.h"
#include "TrinamicDriver.h"
#include "TrinamicUartDriver.h"

Motors::Motor* myMotor[MAX_AXES][MAX_GANGED];  // number of axes (normal and ganged)
void           init_motors() {
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Init Motors");

    auto n_axis = number_axis->get();

    if (n_axis >= 1) {
#ifdef X_TRINAMIC_DRIVER
#    if (X_TRINAMIC_DRIVER == 2130 || X_TRINAMIC_DRIVER == 5160)
        {
            myMotor[X_AXIS][0] =
                new Motors::TrinamicDriver(X_AXIS, X_STEP_PIN, X_DIRECTION_PIN, X_DISABLE_PIN, X_CS_PIN, X_TRINAMIC_DRIVER, X_RSENSE);
        }
#    elif (X_TRINAMIC_DRIVER == 2208 || X_TRINAMIC_DRIVER == 2209)
        {
            myMotor[X_AXIS][0] = new Motors::TrinamicUartDriver(
                X_AXIS, X_STEP_PIN, X_DIRECTION_PIN, X_DISABLE_PIN, X_TRINAMIC_DRIVER, X_RSENSE, X_DRIVER_ADDRESS);
        }
#    else
#        error X Axis undefined motor p/n
#    endif
#elif defined(X_SERVO_PIN)
        myMotor[X_AXIS][0] = new Motors::RcServo(X_AXIS, X_SERVO_PIN);
#elif defined(X_UNIPOLAR)
        myMotor[X_AXIS][0] = new Motors::UnipolarMotor(X_AXIS, X_PIN_PHASE_0, X_PIN_PHASE_1, X_PIN_PHASE_2, X_PIN_PHASE_3);
#elif defined(X_STEP_PIN)
        myMotor[X_AXIS][0] = new Motors::StandardStepper(X_AXIS, X_STEP_PIN, X_DIRECTION_PIN, X_DISABLE_PIN);
#elif defined(X_DYNAMIXEL_ID)
        myMotor[X_AXIS][0] = new Motors::Dynamixel2(X_AXIS, X_DYNAMIXEL_ID, DYNAMIXEL_TXD, DYNAMIXEL_RXD, DYNAMIXEL_RTS);
#else
        myMotor[X_AXIS][0] = new Motors::Nullmotor(X_AXIS);
#endif

#ifdef X2_TRINAMIC_DRIVER
#    if (X_TRINAMIC_DRIVER == 2130 || X_TRINAMIC_DRIVER == 5160)
        {
            myMotor[X_AXIS][1] =
                new Motors::TrinamicDriver(X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN, X2_DISABLE_PIN, X2_CS_PIN, X2_TRINAMIC_DRIVER, X2_RSENSE);
        }
#    elif (X2_TRINAMIC_DRIVER == 2208 || X2_TRINAMIC_DRIVER == 2209)
        {
            myMotor[X_AXIS][1] = new Motors::TrinamicUartDriver(
                X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN, X2_DISABLE_PIN, X2_TRINAMIC_DRIVER, X2_RSENSE, X2_DRIVER_ADDRESS);
        }
#    else
#        error X2 Axis undefined motor p/n
#    endif
#elif defined(X2_UNIPOLAR)
        myMotor[X_AXIS][1] = new Motors::UnipolarMotor(X2_AXIS, X2_PIN_PHASE_0, X2_PIN_PHASE_1, X2_PIN_PHASE_2, X2_PIN_PHASE_3);
#elif defined(X2_STEP_PIN)
        myMotor[X_AXIS][1] = new Motors::StandardStepper(X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN, X2_DISABLE_PIN);
#else
        myMotor[X_AXIS][1] = new Motors::Nullmotor(X2_AXIS);
#endif
    } else {
        myMotor[X_AXIS][0] = new Motors::Nullmotor(X_AXIS);
        myMotor[X_AXIS][1] = new Motors::Nullmotor(X2_AXIS);
    }

    if (n_axis >= 2) {
        // this WILL be done better with settings
#ifdef Y_TRINAMIC_DRIVER
#    if (Y_TRINAMIC_DRIVER == 2130 || Y_TRINAMIC_DRIVER == 5160)
        {
            myMotor[Y_AXIS][0] =
                new Motors::TrinamicDriver(Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN, Y_DISABLE_PIN, Y_CS_PIN, Y_TRINAMIC_DRIVER, Y_RSENSE);
        }
#    elif (Y_TRINAMIC_DRIVER == 2208 || Y_TRINAMIC_DRIVER == 2209)
        {
            myMotor[Y_AXIS][0] = new Motors::TrinamicUartDriver(
                Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN, Y_DISABLE_PIN, Y_TRINAMIC_DRIVER, Y_RSENSE, Y_DRIVER_ADDRESS);
        }
#    else
#        error Y Axis undefined motor p/n
#    endif
#elif defined(Y_SERVO_PIN)
        myMotor[Y_AXIS][0] = new Motors::RcServo(Y_AXIS, Y_SERVO_PIN);
#elif defined(Y_UNIPOLAR)
        myMotor[Y_AXIS][0] = new Motors::UnipolarMotor(Y_AXIS, Y_PIN_PHASE_0, Y_PIN_PHASE_1, Y_PIN_PHASE_2, Y_PIN_PHASE_3);
#elif defined(Y_STEP_PIN)
        myMotor[Y_AXIS][0] = new Motors::StandardStepper(Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN, Y_DISABLE_PIN);
#elif defined(Y_DYNAMIXEL_ID)
        myMotor[Y_AXIS][0] = new Motors::Dynamixel2(Y_AXIS, Y_DYNAMIXEL_ID, DYNAMIXEL_TXD, DYNAMIXEL_RXD, DYNAMIXEL_RTS);
#else
        myMotor[Y_AXIS][0] = new Motors::Nullmotor(Y_AXIS);
#endif

#ifdef Y2_TRINAMIC_DRIVER
#    if (Y2_TRINAMIC_DRIVER == 2130 || Y2_TRINAMIC_DRIVER == 5160)
        {
            myMotor[Y_AXIS][1] =
                new Motors::TrinamicDriver(Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN, Y2_DISABLE_PIN, Y2_CS_PIN, Y2_TRINAMIC_DRIVER, Y2_RSENSE);
        }
#    elif (Y2_TRINAMIC_DRIVER == 2208 || Y2_TRINAMIC_DRIVER == 2209)
        {
            myMotor[Y_AXIS][1] = new Motors::TrinamicUartDriver(
                Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN, Y2_DISABLE_PIN, Y2_TRINAMIC_DRIVER, Y2_RSENSE, Y2_DRIVER_ADDRESS);
        }
#    else
#        error Y2 Axis undefined motor p/n
#    endif
#elif defined(Y2_UNIPOLAR)
        myMotor[Y_AXIS][1] = new Motors::UnipolarMotor(Y2_AXIS, Y2_PIN_PHASE_0, Y2_PIN_PHASE_1, Y2_PIN_PHASE_2, Y2_PIN_PHASE_3);
#elif defined(Y2_STEP_PIN)
        myMotor[Y_AXIS][1] = new Motors::StandardStepper(Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN, Y2_DISABLE_PIN);
#else
        myMotor[Y_AXIS][1] = new Motors::Nullmotor(Y2_AXIS);
#endif
    } else {
        myMotor[Y_AXIS][0] = new Motors::Nullmotor(Y_AXIS);
        myMotor[Y_AXIS][1] = new Motors::Nullmotor(Y2_AXIS);
    }

    if (n_axis >= 3) {
        // this WILL be done better with settings
#ifdef Z_TRINAMIC_DRIVER
#    if (Z_TRINAMIC_DRIVER == 2130 || Z_TRINAMIC_DRIVER == 5160)
        {
            myMotor[Z_AXIS][0] =
                new Motors::TrinamicDriver(Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN, Z_DISABLE_PIN, Z_CS_PIN, Z_TRINAMIC_DRIVER, Z_RSENSE);
        }
#    elif (Z_TRINAMIC_DRIVER == 2208 || Z_TRINAMIC_DRIVER == 2209)
        {
            myMotor[Z_AXIS][0] = new Motors::TrinamicUartDriver(
                Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN, Z_DISABLE_PIN, Z_TRINAMIC_DRIVER, Z_RSENSE, Z_DRIVER_ADDRESS);
        }
#    else
#        error Z Axis undefined motor p/n
#    endif
#elif defined(Z_SERVO_PIN)
        myMotor[Z_AXIS][0] = new Motors::RcServo(Z_AXIS, Z_SERVO_PIN);
#elif defined(Z_UNIPOLAR)
        myMotor[Z_AXIS][0] = new Motors::UnipolarMotor(Z_AXIS, Z_PIN_PHASE_0, Z_PIN_PHASE_1, Z_PIN_PHASE_2, Z_PIN_PHASE_3);
#elif defined(Z_STEP_PIN)
        myMotor[Z_AXIS][0] = new Motors::StandardStepper(Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN, Z_DISABLE_PIN);
#elif defined(Z_DYNAMIXEL_ID)
        myMotor[Z_AXIS][0] = new Motors::Dynamixel2(Z_AXIS, Z_DYNAMIXEL_ID, DYNAMIXEL_TXD, DYNAMIXEL_RXD, DYNAMIXEL_RTS);
#else
        myMotor[Z_AXIS][0] = new Motors::Nullmotor(Z_AXIS);
#endif

#ifdef Z2_TRINAMIC_DRIVER
#    if (Z2_TRINAMIC_DRIVER == 2130 || Z2_TRINAMIC_DRIVER == 5160)
        {
            myMotor[Z_AXIS][1] =
                new Motors::TrinamicDriver(Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN, Z2_DISABLE_PIN, Z2_CS_PIN, Z2_TRINAMIC_DRIVER, Z2_RSENSE);
        }
#    elif (Z2_TRINAMIC_DRIVER == 2208 || Z2_TRINAMIC_DRIVER == 2209)
        {
            myMotor[Z_AXIS][1] = new Motors::TrinamicUartDriver(
                Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN, Z2_DISABLE_PIN, Z2_TRINAMIC_DRIVER, Z2_RSENSE, Z2_DRIVER_ADDRESS);
        }
#    else
#        error Z2 Axis undefined motor p/n
#    endif
#elif defined(Z2_UNIPOLAR)
        myMotor[Z_AXIS][1] = new Motors::UnipolarMotor(Z2_AXIS, Z2_PIN_PHASE_0, Z2_PIN_PHASE_1, Z2_PIN_PHASE_2, Z2_PIN_PHASE_3);
#elif defined(Z2_STEP_PIN)
        myMotor[Z_AXIS][1] = new Motors::StandardStepper(Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN, Z2_DISABLE_PIN);
#else
        myMotor[Z_AXIS][1] = new Motors::Nullmotor(Z2_AXIS);
#endif
    } else {
        myMotor[Z_AXIS][0] = new Motors::Nullmotor(Z_AXIS);
        myMotor[Z_AXIS][1] = new Motors::Nullmotor(Z2_AXIS);
    }

    if (n_axis >= 4) {
        // this WILL be done better with settings
#ifdef A_TRINAMIC_DRIVER
#    if (A_TRINAMIC_DRIVER == 2130 || A_TRINAMIC_DRIVER == 5160)
        {
            myMotor[A_AXIS][0] =
                new Motors::TrinamicDriver(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN, A_DISABLE_PIN, A_CS_PIN, A_TRINAMIC_DRIVER, A_RSENSE);
        }
#    elif (A_TRINAMIC_DRIVER == 2208 || A_TRINAMIC_DRIVER == 2209)
        {
            myMotor[A_AXIS][0] = new Motors::TrinamicUartDriver(
                A_AXIS, A_STEP_PIN, A_DIRECTION_PIN, A_DISABLE_PIN, A_TRINAMIC_DRIVER, A_RSENSE, A_DRIVER_ADDRESS);
        }
#    else
#        error A Axis undefined motor p/n
#    endif
#elif defined(A_SERVO_PIN)
        myMotor[A_AXIS][0] = new Motors::RcServo(A_AXIS, A_SERVO_PIN);
#elif defined(A_UNIPOLAR)
        myMotor[A_AXIS][0] = new Motors::UnipolarMotor(A_AXIS, A_PIN_PHASE_0, A_PIN_PHASE_1, A_PIN_PHASE_2, A_PIN_PHASE_3);
#elif defined(A_STEP_PIN)
        myMotor[A_AXIS][0] = new Motors::StandardStepper(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN, A_DISABLE_PIN);
#elif defined(A_DYNAMIXEL_ID)
        myMotor[A_AXIS][0] = new Motors::Dynamixel2(A_AXIS, A_DYNAMIXEL_ID, DYNAMIXEL_TXD, DYNAMIXEL_RXD, DYNAMIXEL_RTS);
#else
        myMotor[A_AXIS][0] = new Motors::Nullmotor(A_AXIS);
#endif

#ifdef A2_TRINAMIC_DRIVER
#    if (A2_TRINAMIC_DRIVER == 2130 || A2_TRINAMIC_DRIVER == 5160)
        {
            myMotor[A_AXIS][1] =
                new Motors::TrinamicDriver(A2_AXIS, A2_STEP_PIN, A2_DIRECTION_PIN, A2_DISABLE_PIN, A2_CS_PIN, A2_TRINAMIC_DRIVER, A2_RSENSE);
        }
#    elif (A2_TRINAMIC_DRIVER == 2208 || A2_TRINAMIC_DRIVER == 2209)
        {
            myMotor[A_AXIS][1] = new Motors::TrinamicUartDriver(
                A2_AXIS, A2_STEP_PIN, A2_DIRECTION_PIN, A2_DISABLE_PIN, A2_TRINAMIC_DRIVER, A2_RSENSE, A2_DRIVER_ADDRESS);
        }
#    else
#        error A2 Axis undefined motor p/n
#    endif
#elif defined(A2_UNIPOLAR)
        myMotor[A_AXIS][1] = new Motors::UnipolarMotor(A2_AXIS, A2_PIN_PHASE_0, A2_PIN_PHASE_1, A2_PIN_PHASE_2, A2_PIN_PHASE_3);
#elif defined(A2_STEP_PIN)
        myMotor[A_AXIS][1] = new Motors::StandardStepper(A2_AXIS, A2_STEP_PIN, A2_DIRECTION_PIN, A2_DISABLE_PIN);
#else
        myMotor[A_AXIS][1] = new Motors::Nullmotor(A2_AXIS);
#endif
    } else {
        myMotor[A_AXIS][0] = new Motors::Nullmotor(A_AXIS);
        myMotor[A_AXIS][1] = new Motors::Nullmotor(A2_AXIS);
    }

    if (n_axis >= 5) {
        // this WILL be done better with settings
#ifdef B_TRINAMIC_DRIVER
#    if (B_TRINAMIC_DRIVER == 2130 || B_TRINAMIC_DRIVER == 5160)
        {
            myMotor[B_AXIS][0] =
                new Motors::TrinamicDriver(B_AXIS, B_STEP_PIN, B_DIRECTION_PIN, B_DISABLE_PIN, B_CS_PIN, B_TRINAMIC_DRIVER, B_RSENSE);
        }
#    elif (B_TRINAMIC_DRIVER == 2208 || B_TRINAMIC_DRIVER == 2209)
        {
            myMotor[B_AXIS][0] = new Motors::TrinamicUartDriver(
                B_AXIS, B_STEP_PIN, B_DIRECTION_PIN, B_DISABLE_PIN, B_TRINAMIC_DRIVER, B_RSENSE, B_DRIVER_ADDRESS);
        }
#    else
#        error B Axis undefined motor p/n
#    endif
#elif defined(B_SERVO_PIN)
        myMotor[B_AXIS][0] = new Motors::RcServo(B_AXIS, B_SERVO_PIN);
#elif defined(B_UNIPOLAR)
        myMotor[B_AXIS][0] = new Motors::UnipolarMotor(B_AXIS, B_PIN_PHASE_0, B_PIN_PHASE_1, B_PIN_PHASE_2, B_PIN_PHASE_3);
#elif defined(B_STEP_PIN)
        myMotor[B_AXIS][0] = new Motors::StandardStepper(B_AXIS, B_STEP_PIN, B_DIRECTION_PIN, B_DISABLE_PIN);
#elif defined(B_DYNAMIXEL_ID)
        myMotor[B_AXIS][0] = new Motors::Dynamixel2(B_AXIS, B_DYNAMIXEL_ID, DYNAMIXEL_TXD, DYNAMIXEL_RXD, DYNAMIXEL_RTS);
#else
        myMotor[B_AXIS][0] = new Motors::Nullmotor(B_AXIS);
#endif

#ifdef B2_TRINAMIC_DRIVER
#    if (B2_TRINAMIC_DRIVER == 2130 || B2_TRINAMIC_DRIVER == 5160)
        {
            myMotor[B_AXIS][1] =
                new Motors::TrinamicDriver(B2_AXIS, B2_STEP_PIN, B2_DIRECTION_PIN, B2_DISABLE_PIN, B2_CS_PIN, B2_TRINAMIC_DRIVER, B2_RSENSE);
        }
#    elif (B2_TRINAMIC_DRIVER == 2208 || B2_TRINAMIC_DRIVER == 2209)
        {
            myMotor[B_AXIS][1] = new Motors::TrinamicUartDriver(
                B2_AXIS, B2_STEP_PIN, B2_DIRECTION_PIN, B2_DISABLE_PIN, B2_TRINAMIC_DRIVER, B2_RSENSE, B2_DRIVER_ADDRESS);
        }
#    else
#        error B2 Axis undefined motor p/n
#    endif
#elif defined(B2_UNIPOLAR)
        myMotor[B_AXIS][1] = new Motors::UnipolarMotor(B2_AXIS, B2_PIN_PHASE_0, B2_PIN_PHASE_1, B2_PIN_PHASE_2, B2_PIN_PHASE_3);
#elif defined(B2_STEP_PIN)
        myMotor[B_AXIS][1] = new Motors::StandardStepper(B2_AXIS, B2_STEP_PIN, B2_DIRECTION_PIN, B2_DISABLE_PIN);
#else
        myMotor[B_AXIS][1] = new Motors::Nullmotor(B2_AXIS);
#endif
    } else {
        myMotor[B_AXIS][0] = new Motors::Nullmotor(B_AXIS);
        myMotor[B_AXIS][1] = new Motors::Nullmotor(B2_AXIS);
    }

    if (n_axis >= 6) {
        // this WILL be done better with settings
#ifdef C_TRINAMIC_DRIVER
#    if (C_TRINAMIC_DRIVER == 2130 || C_TRINAMIC_DRIVER == 5160)
        {
            myMotor[C_AXIS][0] =
                new Motors::TrinamicDriver(C_AXIS, C_STEP_PIN, C_DIRECTION_PIN, C_DISABLE_PIN, C_CS_PIN, C_TRINAMIC_DRIVER, C_RSENSE);
        }
#    elif (C_TRINAMIC_DRIVER == 2208 || C_TRINAMIC_DRIVER == 2209)
        {
            myMotor[C_AXIS][0] = new Motors::TrinamicUartDriver(
                C_AXIS, C_STEP_PIN, C_DIRECTION_PIN, C_DISABLE_PIN, C_TRINAMIC_DRIVER, C_RSENSE, C_DRIVER_ADDRESS);
        }
#    else
#        error C Axis undefined motor p/n
#    endif
#elif defined(C_SERVO_PIN)
        myMotor[C_AXIS][0] = new Motors::RcServo(C_AXIS, C_SERVO_PIN);
#elif defined(C_UNIPOLAR)
        myMotor[C_AXIS][0] = new Motors::UnipolarMotor(C_AXIS, C_PIN_PHASE_0, C_PIN_PHASE_1, C_PIN_PHASE_2, C_PIN_PHASE_3);
#elif defined(C_STEP_PIN)
        myMotor[C_AXIS][0] = new Motors::StandardStepper(C_AXIS, C_STEP_PIN, C_DIRECTION_PIN, C_DISABLE_PIN);
#elif defined(C_DYNAMIXEL_ID)
        myMotor[C_AXIS][0] = new Motors::Dynamixel2(C_AXIS, C_DYNAMIXEL_ID, DYNAMIXEL_TXD, DYNAMIXEL_RXD, DYNAMIXEL_RTS);
#else
        myMotor[C_AXIS][0] = new Motors::Nullmotor(C_AXIS);
#endif

#ifdef C2_TRINAMIC_DRIVER
#    if (C2_TRINAMIC_DRIVER == 2130 || C2_TRINAMIC_DRIVER == 5160)
        {
            myMotor[C_AXIS][1] =
                new Motors::TrinamicDriver(C2_AXIS, C2_STEP_PIN, C2_DIRECTION_PIN, C2_DISABLE_PIN, C2_CS_PIN, C2_TRINAMIC_DRIVER, C2_RSENSE);
        }
#    elif (C2_TRINAMIC_DRIVER == 2208 || C2_TRINAMIC_DRIVER == 2209)
        {
            myMotor[C_AXIS][1] = new Motors::TrinamicUartDriver(
                C2_AXIS, C2_STEP_PIN, C2_DIRECTION_PIN, C2_DISABLE_PIN, C2_TRINAMIC_DRIVER, C2_RSENSE, C2_DRIVER_ADDRESS);
        }
#    else
#        error C2 Axis undefined motor p/n
#    endif
#elif defined(C2_UNIPOLAR)
        myMotor[C_AXIS][1] = new Motors::UnipolarMotor(C2_AXIS, C2_PIN_PHASE_0, C2_PIN_PHASE_1, C2_PIN_PHASE_2, C2_PIN_PHASE_3);
#elif defined(C2_STEP_PIN)
        myMotor[C_AXIS][1] = new Motors::StandardStepper(C2_AXIS, C2_STEP_PIN, C2_DIRECTION_PIN, C2_DISABLE_PIN);
#else
        myMotor[C_AXIS][1] = new Motors::Nullmotor(C2_AXIS);
#endif
    } else {
        myMotor[C_AXIS][0] = new Motors::Nullmotor(C_AXIS);
        myMotor[C_AXIS][1] = new Motors::Nullmotor(C2_AXIS);
    }

#ifdef USE_STEPSTICK

    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Using StepStick Mode");

    uint8_t ms3_pins[MAX_N_AXIS][2] = { { X_STEPPER_MS3, X2_STEPPER_MS3 }, { Y_STEPPER_MS3, Y2_STEPPER_MS3 },
                                        { Z_STEPPER_MS3, Z2_STEPPER_MS3 }, { A_STEPPER_MS3, A2_STEPPER_MS3 },
                                        { B_STEPPER_MS3, B2_STEPPER_MS3 }, { C_STEPPER_MS3, C2_STEPPER_MS3 } };

    for (int axis = 0; axis < n_axis; axis++) {
        for (int gang_index = 0; gang_index < 2; gang_index++) {
            uint8_t pin = ms3_pins[axis][gang_index];
            if (pin != UNDEFINED_PIN) {
                digitalWrite(pin, HIGH);
                pinMode(pin, OUTPUT);
            }
        }
    }

#    ifdef STEPPER_RESET
    // !RESET pin on steppers  (MISO On Schematic)
    digitalWrite(STEPPER_RESET, HIGH);
    pinMode(STEPPER_RESET, OUTPUT);
#    endif

#endif

    if (STEPPERS_DISABLE_PIN != UNDEFINED_PIN) {
        pinMode(STEPPERS_DISABLE_PIN, OUTPUT);  // global motor enable pin
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Global stepper disable pin:%s", pinName(STEPPERS_DISABLE_PIN));
    }

    // certain motors need features to be turned on. Check them here
    for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
        for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
            myMotor[axis][gang_index]->init();
        }
    }
}

void motors_set_disable(bool disable, uint8_t mask) {
    static bool    prev_disable = true;
    static uint8_t prev_mask    = 0;

    if ((disable == prev_disable) && (mask == prev_mask)) {
        return;
    }

    prev_disable = disable;
    prev_mask    = mask;

    if (step_enable_invert->get()) {
        disable = !disable;  // Apply pin invert.
    }

    // now loop through all the motors to see if they can individually disable
    auto n_axis = number_axis->get();
    for (uint8_t gang_index = 0; gang_index < MAX_GANGED; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
            if (bitnum_istrue(mask, axis)) {
                myMotor[axis][gang_index]->set_disable(disable);
            }
        }
    }

    // global disable.
    digitalWrite(STEPPERS_DISABLE_PIN, disable);

    // Add an optional delay for stepper drivers. that need time
    // Some need time after the enable before they can step.
    auto wait_disable_change = enable_delay_microseconds->get();
    if (wait_disable_change != 0) {
        auto disable_start_time = esp_timer_get_time() + wait_disable_change;

        while ((esp_timer_get_time() - disable_start_time) < 0) {
            NOP();
        }
    }
}

void motors_read_settings() {
    //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Read Settings");
    auto n_axis = number_axis->get();
    for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
            myMotor[axis][gang_index]->read_settings();
        }
    }
}

// use this to tell all the motors what the current homing mode is
// They can use this to setup things like Stall
uint8_t motors_set_homing_mode(uint8_t homing_mask, bool isHoming) {
    uint8_t can_home = 0;
    auto    n_axis   = number_axis->get();
    for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
        if (bitnum_istrue(homing_mask, axis)) {
            if (myMotor[axis][0]->set_homing_mode(isHoming)) {
                bitnum_true(can_home, axis);
            }
            myMotor[axis][1]->set_homing_mode(isHoming);
        }
    }
    return can_home;
}

bool motors_direction(uint8_t dir_mask) {
    auto n_axis = number_axis->get();
    //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "motors_set_direction_pins:0x%02X", onMask);

    // Set the direction pins, but optimize for the common
    // situation where the direction bits haven't changed.
    static uint8_t previous_dir = 255;  // should never be this value
    if (dir_mask != previous_dir) {
        previous_dir = dir_mask;

        for (int axis = X_AXIS; axis < n_axis; axis++) {
            bool thisDir = bitnum_istrue(dir_mask, axis);
            myMotor[axis][0]->set_direction(thisDir);
            myMotor[axis][1]->set_direction(thisDir);
        }

        return true;
    } else {
        return false;
    }
}

void motors_step(uint8_t step_mask) {
    auto n_axis = number_axis->get();
    //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "motors_set_direction_pins:0x%02X", onMask);

    // Turn on step pulses for motors that are supposed to step now
    for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
        if (bitnum_istrue(step_mask, axis)) {
            if ((ganged_mode == SquaringMode::Dual) || (ganged_mode == SquaringMode::A)) {
                myMotor[axis][0]->step();
            }
            if ((ganged_mode == SquaringMode::Dual) || (ganged_mode == SquaringMode::B)) {
                myMotor[axis][1]->step();
            }
        }
    }
}
// Turn all stepper pins off
void motors_unstep() {
    auto n_axis = number_axis->get();
    for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
        myMotor[axis][0]->unstep();
        myMotor[axis][1]->unstep();
    }
}
