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

// TODO:
//  I don't like the single value motor_index. Should it be axis, gang_index, like the rest of Grbl?
//  We might want to change stepstick to, or add, specific drive names DRV8825, A4988. This may prove useful one day.
void init_motors() {
    auto n_axis = number_axis->get();
    for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
        for (uint8_t gang_index = 0; gang_index < MAX_GANGED; gang_index++) {
            uint8_t motor_index = axis + (6 * gang_index);  // X X2 thing
            switch (motor_types[axis][gang_index]->get()) {
                case static_cast<int8_t>(MotorType::None):
                    myMotor[axis][gang_index] = new Motors::Nullmotor(motor_index);
                    break;
                case static_cast<int8_t>(MotorType::StepStick):
                case static_cast<int8_t>(MotorType::External):
                    myMotor[axis][0] = new Motors::StandardStepper(motor_index,
                                                                   StepPins[axis][gang_index]->get(),
                                                                   DirectionPins[axis][gang_index]->get(),
                                                                   DisablePins[axis][gang_index]->get());
                    break;
                case static_cast<int8_t>(MotorType::TMC2130):
                case static_cast<int8_t>(MotorType::TMC5160):
                    myMotor[axis][gang_index] = new Motors::TrinamicDriver(motor_index,
                                                                           StepPins[axis][gang_index]->get(),
                                                                           DirectionPins[axis][gang_index]->get(),
                                                                           DisablePins[axis][gang_index]->get(),
                                                                           ChipSelectPins[axis][gang_index]->get(),
                                                                           static_cast<MotorType>(motor_types[axis][gang_index]->get()),
                                                                           motor_rsense[axis][gang_index]->get());

                    break;
                case static_cast<int8_t>(MotorType::TMC2208):
                case static_cast<int8_t>(MotorType::TMC2209):
                    myMotor[axis][gang_index] = new Motors::TrinamicUartDriver(motor_index,
                                                                               StepPins[axis][gang_index]->get(),
                                                                               DirectionPins[axis][gang_index]->get(),
                                                                               DisablePins[axis][gang_index]->get(),
                                                                               static_cast<MotorType>(motor_types[axis][gang_index]->get()),
                                                                               motor_rsense[axis][gang_index]->get(),
                                                                               motor_address[axis][gang_index]->get());
                    break;
                case static_cast<int8_t>(MotorType::Unipolar):
                    myMotor[axis][gang_index] = new Motors::UnipolarMotor(motor_index,
                                                                          PhasePins[1][axis][gang_index]->get(),
                                                                          PhasePins[1][axis][gang_index]->get(),
                                                                          PhasePins[2][axis][gang_index]->get(),
                                                                          PhasePins[3][axis][gang_index]->get());
                    break;
                case static_cast<int8_t>(MotorType::Dynamixel):
                    myMotor[axis][gang_index] = new Motors::Dynamixel2(motor_index,
                                                                       motor_address[axis][gang_index]->get(),
                                                                       DynamixelTXDPin->get(),
                                                                       DynamixelRXDPin->get(),
                                                                       DynamixelRTSPin->get());
                    break;
                case static_cast<int8_t>(MotorType::RCServo):
                    myMotor[axis][gang_index] = new Motors::RcServo(motor_index, ServoPins[axis][gang_index]->get());
                    break;
                case static_cast<int8_t>(MotorType::Solenoid):
                    // not yet
                    break;
                default:
                    myMotor[axis][gang_index] = new Motors::Nullmotor(motor_index);
                    break;
            }
        }
    }

    for (int axis = 0; axis < n_axis; axis++) {
        for (int gang_index = 0; gang_index < 2; gang_index++) {
            if (StepStickMS3[axis][gang_index]->get() != Pin::UNDEFINED) {
                StepStickMS3[axis][gang_index]->get().setAttr(Pin::Attr::Output | Pin::Attr::InitialOn);
            }
        }
    }

    if (StepperResetPin->get() != Pin::UNDEFINED) {
        // !RESET pin on steppers  (MISO On Schematic)
        StepperResetPin->get().setAttr(Pin::Attr::Output | Pin::Attr::InitialOn);
    }

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
