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
		Get rid of Z_SERVO, just reply on ServoPins[Z_AXIS][0]
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
        myMotor[X_AXIS][0] = new Motors::TrinamicDriver(X_AXIS,
                                                        StepPins[X_AXIS][0]->get(),
                                                        DirectionPins[X_AXIS][0]->get(),
                                                        DisablePins[X_AXIS][0]->get(),
                                                        ClearToSendPins[X_AXIS][0]->get(),
                                                        X_TRINAMIC_DRIVER,
                                                        X_RSENSE);
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
        myMotor[X_AXIS][0] = new Motors::UnipolarMotor(X_AXIS,
                                                       PhasePins[0][X_AXIS][0]->get(),
                                                       PhasePins[1][X_AXIS][0]->get(),
                                                       PhasePins[2][X_AXIS][0]->get(),
                                                       PhasePins[3][X_AXIS][0]->get());
#elif defined(X_DYNAMIXEL_ID)
        myMotor[X_AXIS][0] =
            new Motors::Dynamixel2(X_AXIS, X_DYNAMIXEL_ID, DynamixelTXDPin->get(), DynamixelRXDPin->get(), DynamixelRTSPin->get());
#else
        if (ServoPins[X_AXIS][0]->get() != Pin::UNDEFINED)
            myMotor[X_AXIS][0] = new Motors::RcServo(X_AXIS, ServoPins[X_AXIS][0]->get());
        else if (StepPins[X_AXIS][0]->get() != Pin::UNDEFINED)
            myMotor[X_AXIS][0] = new Motors::StandardStepper(
                X_AXIS, StepPins[X_AXIS][0]->get(), DirectionPins[X_AXIS][0]->get(), DisablePins[X_AXIS][0]->get());
        else
            myMotor[X_AXIS][0] = new Motors::Nullmotor(X_AXIS);
#endif

#ifdef X2_TRINAMIC_DRIVER
        myMotor[X_AXIS][1] = new Motors::TrinamicDriver(X2_AXIS,
                                                        StepPins[X_AXIS][1]->get(),
                                                        DirectionPins[X_AXIS][1]->get(),
                                                        DisablePins[X_AXIS][1]->get(),
                                                        ClearToSendPins[X_AXIS][1]->get(),
                                                        X2_TRINAMIC_DRIVER,
                                                        X2_RSENSE);
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
        myMotor[X_AXIS][1] = new Motors::UnipolarMotor(X2_AXIS,
                                                       PhasePins[1][X_AXIS][1]->get(),
                                                       PhasePins[1][X_AXIS][1]->get(),
                                                       PhasePins[2][X_AXIS][1]->get(),
                                                       PhasePins[3][X_AXIS][1]->get());
#else
        if (StepPins[X_AXIS][1]->get() != Pin::UNDEFINED)
            myMotor[X_AXIS][1] = new Motors::StandardStepper(
                X2_AXIS, StepPins[X_AXIS][1]->get(), DirectionPins[X_AXIS][1]->get(), DisablePins[X_AXIS][1]->get());
        else
            myMotor[X_AXIS][1] = new Motors::Nullmotor(X2_AXIS);
#endif
    } else {
        myMotor[X_AXIS][0] = new Motors::Nullmotor(X_AXIS);
        myMotor[X_AXIS][1] = new Motors::Nullmotor(X2_AXIS);
    }

    if (n_axis >= 2) {
        // this WILL be done better with settings
#ifdef Y_TRINAMIC_DRIVER
        myMotor[Y_AXIS][0] = new Motors::TrinamicDriver(Y_AXIS,
                                                        StepPins[Y_AXIS][0]->get(),
                                                        DirectionPins[Y_AXIS][0]->get(),
                                                        DisablePins[Y_AXIS][0]->get(),
                                                        ClearToSendPins[Y_AXIS][0]->get(),
                                                        Y_TRINAMIC_DRIVER,
                                                        Y_RSENSE);
#    if (X_TRINAMIC_DRIVER == 2130 || X_TRINAMIC_DRIVER == 5160)
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
        myMotor[Y_AXIS][0] = new Motors::UnipolarMotor(Y_AXIS,
                                                       PhasePins[0][Y_AXIS][0]->get(),
                                                       PhasePins[1][Y_AXIS][0]->get(),
                                                       PhasePins[2][Y_AXIS][0]->get(),
                                                       PhasePins[3][Y_AXIS][0]->get());
#elif defined(Y_DYNAMIXEL_ID)
        myMotor[Y_AXIS][0] =
            new Motors::Dynamixel2(Y_AXIS, Y_DYNAMIXEL_ID, DynamixelTXDPin->get(), DynamixelRXDPin->get(), DynamixelRTSPin->get());
#else
        if (ServoPins[Y_AXIS][0]->get() != Pin::UNDEFINED)
            myMotor[Y_AXIS][0] = new Motors::RcServo(Y_AXIS, ServoPins[Y_AXIS][0]->get());
        else if (StepPins[Y_AXIS][0]->get() != Pin::UNDEFINED)
            myMotor[Y_AXIS][0] = new Motors::StandardStepper(
                Y_AXIS, StepPins[Y_AXIS][0]->get(), DirectionPins[Y_AXIS][0]->get(), DisablePins[Y_AXIS][0]->get());
        else
            myMotor[Y_AXIS][0] = new Motors::Nullmotor(Y_AXIS);
#endif

#ifdef Y2_TRINAMIC_DRIVER
        myMotor[Y_AXIS][1] = new Motors::TrinamicDriver(Y2_AXIS,
                                                        StepPins[Y_AXIS][1]->get(),
                                                        DirectionPins[Y_AXIS][1]->get(),
                                                        DisablePins[Y_AXIS][1]->get(),
                                                        ClearToSendPins[Y_AXIS][1]->get(),
                                                        Y2_TRINAMIC_DRIVER,
                                                        Y2_RSENSE);
#    if (X_TRINAMIC_DRIVER == 2130 || X_TRINAMIC_DRIVER == 5160)
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
        myMotor[Y_AXIS][1] = new Motors::UnipolarMotor(Y2_AXIS,
                                                       PhasePins[1][Y_AXIS][1]->get(),
                                                       PhasePins[1][Y_AXIS][1]->get(),
                                                       PhasePins[2][Y_AXIS][1]->get(),
                                                       PhasePins[3][Y_AXIS][1]->get());
#else
        if (StepPins[Y_AXIS][1]->get() != Pin::UNDEFINED)
            myMotor[Y_AXIS][1] = new Motors::StandardStepper(
                Y2_AXIS, StepPins[Y_AXIS][1]->get(), DirectionPins[Y_AXIS][1]->get(), DisablePins[Y_AXIS][1]->get());
        else
            myMotor[Y_AXIS][1] = new Motors::Nullmotor(Y2_AXIS);
#endif
    } else {
        myMotor[Y_AXIS][0] = new Motors::Nullmotor(Y_AXIS);
        myMotor[Y_AXIS][1] = new Motors::Nullmotor(Y2_AXIS);
    }

    if (n_axis >= 3) {
        // this WILL be done better with settings
#ifdef Z_TRINAMIC_DRIVER
        myMotor[Z_AXIS][0] = new Motors::TrinamicDriver(Z_AXIS,
                                                        StepPins[Z_AXIS][0]->get(),
                                                        DirectionPins[Z_AXIS][0]->get(),
                                                        DisablePins[Z_AXIS][0]->get(),
                                                        ClearToSendPins[Z_AXIS][0]->get(),
                                                        Z_TRINAMIC_DRIVER,
                                                        Z_RSENSE);
#    if (X_TRINAMIC_DRIVER == 2130 || X_TRINAMIC_DRIVER == 5160)
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
        myMotor[Z_AXIS][0] = new Motors::UnipolarMotor(Z_AXIS,
                                                       PhasePins[0][Z_AXIS][0]->get(),
                                                       PhasePins[1][Z_AXIS][0]->get(),
                                                       PhasePins[2][Z_AXIS][0]->get(),
                                                       PhasePins[3][Z_AXIS][0]->get());
#elif defined(Z_DYNAMIXEL_ID)
        myMotor[Z_AXIS][0] =
            new Motors::Dynamixel2(Z_AXIS, Z_DYNAMIXEL_ID, DynamixelTXDPin->get(), DynamixelRXDPin->get(), DynamixelRTSPin->get());
#else
        if (ServoPins[Z_AXIS][0]->get() != Pin::UNDEFINED)
            myMotor[Z_AXIS][0] = new Motors::RcServo(Z_AXIS, ServoPins[Z_AXIS][0]->get());
        else if (StepPins[Z_AXIS][0]->get() != Pin::UNDEFINED)
            myMotor[Z_AXIS][0] = new Motors::StandardStepper(
                Z_AXIS, StepPins[Z_AXIS][0]->get(), DirectionPins[Z_AXIS][0]->get(), DisablePins[Z_AXIS][0]->get());
        else
            myMotor[Z_AXIS][0] = new Motors::Nullmotor(Z_AXIS);
#endif

#ifdef Z2_TRINAMIC_DRIVER
        myMotor[Z_AXIS][1] = new Motors::TrinamicDriver(Z2_AXIS,
                                                        StepPins[Z_AXIS][1]->get(),
                                                        DirectionPins[Z_AXIS][1]->get(),
                                                        DisablePins[Z_AXIS][1]->get(),
                                                        ClearToSendPins[Z_AXIS][1]->get(),
                                                        Z2_TRINAMIC_DRIVER,
                                                        Z2_RSENSE);
#    if (X_TRINAMIC_DRIVER == 2130 || X_TRINAMIC_DRIVER == 5160)
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
        myMotor[Z_AXIS][1] = new Motors::UnipolarMotor(Z2_AXIS,
                                                       PhasePins[1][Z_AXIS][1]->get(),
                                                       PhasePins[1][Z_AXIS][1]->get(),
                                                       PhasePins[2][Z_AXIS][1]->get(),
                                                       PhasePins[3][Z_AXIS][1]->get());
#else
        if (StepPins[Z_AXIS][1]->get() != Pin::UNDEFINED)
            myMotor[Z_AXIS][1] = new Motors::StandardStepper(
                Z2_AXIS, StepPins[Z_AXIS][1]->get(), DirectionPins[Z_AXIS][1]->get(), DisablePins[Z_AXIS][1]->get());
        else
            myMotor[Z_AXIS][1] = new Motors::Nullmotor(Z2_AXIS);
#endif
    } else {
        myMotor[Z_AXIS][0] = new Motors::Nullmotor(Z_AXIS);
        myMotor[Z_AXIS][1] = new Motors::Nullmotor(Z2_AXIS);
    }

    if (n_axis >= 4) {
        // this WILL be done better with settings
#ifdef A_TRINAMIC_DRIVER
        myMotor[A_AXIS][0] = new Motors::TrinamicDriver(A_AXIS,
                                                        StepPins[A_AXIS][0]->get(),
                                                        DirectionPins[A_AXIS][0]->get(),
                                                        DisablePins[A_AXIS][0]->get(),
                                                        ClearToSendPins[A_AXIS][0]->get(),
                                                        A_TRINAMIC_DRIVER,
                                                        A_RSENSE);
#    if (X_TRINAMIC_DRIVER == 2130 || X_TRINAMIC_DRIVER == 5160)
        {
            myMotor[A_AXIS][1] =
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
        myMotor[A_AXIS][0] = new Motors::UnipolarMotor(A_AXIS,
                                                       PhasePins[0][A_AXIS][0]->get(),
                                                       PhasePins[1][A_AXIS][0]->get(),
                                                       PhasePins[2][A_AXIS][0]->get(),
                                                       PhasePins[3][A_AXIS][0]->get());
#elif defined(A_DYNAMIXEL_ID)
        myMotor[A_AXIS][0] =
            new Motors::Dynamixel2(A_AXIS, A_DYNAMIXEL_ID, DynamixelTXDPin->get(), DynamixelRXDPin->get(), DynamixelRTSPin->get());
#else
        if (ServoPins[A_AXIS][0]->get() != Pin::UNDEFINED)
            myMotor[A_AXIS][0] = new Motors::RcServo(A_AXIS, ServoPins[A_AXIS][0]->get());
        else if (StepPins[A_AXIS][0]->get() != Pin::UNDEFINED)
            myMotor[A_AXIS][0] = new Motors::StandardStepper(
                A_AXIS, StepPins[A_AXIS][0]->get(), DirectionPins[A_AXIS][0]->get(), DisablePins[A_AXIS][0]->get());
        else
            myMotor[A_AXIS][0] = new Motors::Nullmotor(A_AXIS);
#endif

#ifdef A2_TRINAMIC_DRIVER
        myMotor[A_AXIS][1] = new Motors::TrinamicDriver(A2_AXIS,
                                                        StepPins[A_AXIS][1]->get(),
                                                        DirectionPins[A_AXIS][1]->get(),
                                                        DisablePins[A_AXIS][1]->get(),
                                                        ClearToSendPins[A_AXIS][1]->get(),
                                                        A2_TRINAMIC_DRIVER,
                                                        A2_RSENSE);
#    if (X_TRINAMIC_DRIVER == 2130 || X_TRINAMIC_DRIVER == 5160)
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
        myMotor[A_AXIS][1] = new Motors::UnipolarMotor(A2_AXIS,
                                                       PhasePins[1][A_AXIS][1]->get(),
                                                       PhasePins[1][A_AXIS][1]->get(),
                                                       PhasePins[2][A_AXIS][1]->get(),
                                                       PhasePins[3][A_AXIS][1]->get());
#else
        if (StepPins[A_AXIS][1]->get() != Pin::UNDEFINED)
            myMotor[A_AXIS][1] = new Motors::StandardStepper(
                A2_AXIS, StepPins[A_AXIS][1]->get(), DirectionPins[A_AXIS][1]->get(), DisablePins[A_AXIS][1]->get());
        else
            myMotor[A_AXIS][1] = new Motors::Nullmotor(A2_AXIS);
#endif
    } else {
        myMotor[A_AXIS][0] = new Motors::Nullmotor(A_AXIS);
        myMotor[A_AXIS][1] = new Motors::Nullmotor(A2_AXIS);
    }

    if (n_axis >= 5) {
        // this WILL be done better with settings
#ifdef B_TRINAMIC_DRIVER
        myMotor[B_AXIS][0] = new Motors::TrinamicDriver(B_AXIS,
                                                        StepPins[B_AXIS][0]->get(),
                                                        DirectionPins[B_AXIS][0]->get(),
                                                        DisablePins[B_AXIS][0]->get(),
                                                        ClearToSendPins[B_AXIS][0]->get(),
                                                        B_TRINAMIC_DRIVER,
                                                        B_RSENSE);
#    if (X_TRINAMIC_DRIVER == 2130 || X_TRINAMIC_DRIVER == 5160)
        {
            myMotor[B_AXIS][1] =
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
        myMotor[B_AXIS][0] = new Motors::UnipolarMotor(B_AXIS,
                                                       PhasePins[0][B_AXIS][0]->get(),
                                                       PhasePins[1][B_AXIS][0]->get(),
                                                       PhasePins[2][B_AXIS][0]->get(),
                                                       PhasePins[3][B_AXIS][0]->get());
#elif defined(B_DYNAMIXEL_ID)
        myMotor[B_AXIS][0] =
            new Motors::Dynamixel2(B_AXIS, B_DYNAMIXEL_ID, DynamixelTXDPin->get(), DynamixelRXDPin->get(), DynamixelRTSPin->get());
#else
        if (ServoPins[B_AXIS][0]->get() != Pin::UNDEFINED)
            myMotor[B_AXIS][0] = new Motors::RcServo(B_AXIS, ServoPins[B_AXIS][0]->get());
        else if (StepPins[B_AXIS][0]->get() != Pin::UNDEFINED)
            myMotor[B_AXIS][0] = new Motors::StandardStepper(
                B_AXIS, StepPins[B_AXIS][0]->get(), DirectionPins[B_AXIS][0]->get(), DisablePins[B_AXIS][0]->get());
        else
            myMotor[B_AXIS][0] = new Motors::Nullmotor(B_AXIS);
#endif

#ifdef B2_TRINAMIC_DRIVER
        myMotor[B_AXIS][1] = new Motors::TrinamicDriver(B2_AXIS,
                                                        StepPins[B_AXIS][1]->get(),
                                                        DirectionPins[B_AXIS][1]->get(),
                                                        DisablePins[B_AXIS][1]->get(),
                                                        ClearToSendPins[B_AXIS][1]->get(),
                                                        B2_TRINAMIC_DRIVER,
                                                        B2_RSENSE);
#    if (X_TRINAMIC_DRIVER == 2130 || X_TRINAMIC_DRIVER == 5160)
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
        myMotor[B_AXIS][1] = new Motors::UnipolarMotor(B2_AXIS,
                                                       PhasePins[1][B_AXIS][1]->get(),
                                                       PhasePins[1][B_AXIS][1]->get(),
                                                       PhasePins[2][B_AXIS][1]->get(),
                                                       PhasePins[3][B_AXIS][1]->get());
#else
        if (StepPins[B_AXIS][1]->get() != Pin::UNDEFINED)
            myMotor[B_AXIS][1] = new Motors::StandardStepper(
                B2_AXIS, StepPins[B_AXIS][1]->get(), DirectionPins[B_AXIS][1]->get(), DisablePins[B_AXIS][1]->get());
        else
            myMotor[B_AXIS][1] = new Motors::Nullmotor(B2_AXIS);
#endif
    } else {
        myMotor[B_AXIS][0] = new Motors::Nullmotor(B_AXIS);
        myMotor[B_AXIS][1] = new Motors::Nullmotor(B2_AXIS);
    }

    if (n_axis >= 6) {
        // this WILL be done better with settings
#ifdef C_TRINAMIC_DRIVER
        myMotor[C_AXIS][0] = new Motors::TrinamicDriver(C_AXIS,
                                                        StepPins[C_AXIS][0]->get(),
                                                        DirectionPins[C_AXIS][0]->get(),
                                                        DisablePins[C_AXIS][0]->get(),
                                                        ClearToSendPins[C_AXIS][0]->get(),
                                                        C_TRINAMIC_DRIVER,
                                                        C_RSENSE);
#    if (X_TRINAMIC_DRIVER == 2130 || X_TRINAMIC_DRIVER == 5160)
        {
            myMotor[C_AXIS][1] =
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
        myMotor[C_AXIS][0] = new Motors::UnipolarMotor(C_AXIS,
                                                       PhasePins[0][C_AXIS][0]->get(),
                                                       PhasePins[1][C_AXIS][0]->get(),
                                                       PhasePins[2][C_AXIS][0]->get(),
                                                       PhasePins[3][C_AXIS][0]->get());
#elif defined(C_DYNAMIXEL_ID)
        myMotor[C_AXIS][0] =
            new Motors::Dynamixel2(C_AXIS, C_DYNAMIXEL_ID, DynamixelTXDPin->get(), DynamixelRXDPin->get(), DynamixelRTSPin->get());
#else
        if (ServoPins[C_AXIS][0]->get() != Pin::UNDEFINED)
            myMotor[C_AXIS][0] = new Motors::RcServo(C_AXIS, ServoPins[C_AXIS][0]->get());
        else if (StepPins[C_AXIS][0]->get() != Pin::UNDEFINED)
            myMotor[C_AXIS][0] = new Motors::StandardStepper(
                C_AXIS, StepPins[C_AXIS][0]->get(), DirectionPins[C_AXIS][0]->get(), DisablePins[C_AXIS][0]->get());
        else
            myMotor[C_AXIS][0] = new Motors::Nullmotor(C_AXIS);
#endif

#ifdef C2_TRINAMIC_DRIVER
        myMotor[C_AXIS][1] = new Motors::TrinamicDriver(C2_AXIS,
                                                        StepPins[C_AXIS][1]->get(),
                                                        DirectionPins[C_AXIS][1]->get(),
                                                        DisablePins[C_AXIS][1]->get(),
                                                        ClearToSendPins[C_AXIS][1]->get(),
                                                        C2_TRINAMIC_DRIVER,
                                                        C2_RSENSE);
#    if (X_TRINAMIC_DRIVER == 2130 || X_TRINAMIC_DRIVER == 5160)
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
        myMotor[C_AXIS][1] = new Motors::UnipolarMotor(C2_AXIS,
                                                       PhasePins[1][C_AXIS][1]->get(),
                                                       PhasePins[1][C_AXIS][1]->get(),
                                                       PhasePins[2][C_AXIS][1]->get(),
                                                       PhasePins[3][C_AXIS][1]->get());
#else
        if (StepPins[C_AXIS][1]->get() != Pin::UNDEFINED)
            myMotor[C_AXIS][1] = new Motors::StandardStepper(
                C2_AXIS, StepPins[C_AXIS][1]->get(), DirectionPins[C_AXIS][1]->get(), DisablePins[C_AXIS][1]->get());
        else
            myMotor[C_AXIS][1] = new Motors::Nullmotor(C2_AXIS);
#endif
    } else {
        myMotor[C_AXIS][0] = new Motors::Nullmotor(C_AXIS);
        myMotor[C_AXIS][1] = new Motors::Nullmotor(C2_AXIS);
    }

#ifdef USE_STEPSTICK

    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Using StepStick Mode");

    Pin ms3_pins[MAX_N_AXIS][2] = { { StepStickMS3[X_AXIS][0]->get(), StepStickMS3[X_AXIS][1]->get() },
                                    { StepStickMS3[Y_AXIS][0]->get(), StepStickMS3[Y_AXIS][1]->get() },
                                    { StepStickMS3[Z_AXIS][0]->get(), StepStickMS3[Z_AXIS][1]->get() },
                                    { StepStickMS3[A_AXIS][0]->get(), StepStickMS3[A_AXIS][1]->get() },
                                    { StepStickMS3[B_AXIS][0]->get(), StepStickMS3[B_AXIS][1]->get() },
                                    { StepStickMS3[C_AXIS][0]->get(), StepStickMS3[C_AXIS][1]->get() } };

    for (int axis = 0; axis < n_axis; axis++) {
        for (int gang_index = 0; gang_index < 2; gang_index++) {
            Pin pin = ms3_pins[axis][gang_index];
            if (pin != Pin::UNDEFINED) {
                pin.setAttr(Pin::Attr::Output | Pin::Attr::InitialOn);
            }
        }
    }

    if (StepperResetPin->get() != Pin::UNDEFINED) {
        // !RESET pin on steppers  (MISO On Schematic)
        StepperResetPin->get().setAttr(Pin::Attr::Output | Pin::Attr::InitialOn);
    }

#endif

    if (SteppersDisablePin->get() != Pin::UNDEFINED) {
        SteppersDisablePin->get().setAttr(Pin::Attr::Output);  // global motor enable pin
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Global stepper disable pin:%s", SteppersDisablePin->get().name());
    }

    // certain motors need features to be turned on. Check them here
    for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
        for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
            myMotor[axis][gang_index]->init();
        }
    }
}

void motors_set_disable(bool disable, uint8_t mask) {
    static bool previous_state = true;

    // now loop through all the motors to see if they can individually disable
    auto n_axis = number_axis->get();
    for (uint8_t gang_index = 0; gang_index < MAX_GANGED; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
            if (bitnum_istrue(mask, axis)) {
                myMotor[axis][gang_index]->set_disable(disable);
            }
        }
    }

    // invert only inverts the global stepper disable pin.
    SteppersDisablePin->get().write(disable);
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

void motors_step(uint8_t step_mask, uint8_t dir_mask) {
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
    }
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