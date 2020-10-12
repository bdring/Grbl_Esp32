#pragma once

/*
    Motor.h
    Header file for Motor Classes
    Here is the hierarchy
        Motor
            Nullmotor
            StandardStepper
                TrinamicDriver
            Unipolar
            RC Servo

    These are for motors coordinated by Grbl_ESP32
    See motorClass.cpp for more details

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

#include "Motors.h"

#include <cstdint>

namespace Motors {
    class Motor {
    public:
        Motor(motor_class_id_t type, uint8_t axis_index);

        // init() establishes configured motor parameters.  It is called after
        // all motor objects have been constructed.
        virtual void init();

        // config_message(), called from init(), displays a message describing
        // the motor configuration - pins and other motor-specific items
        // TODO Architecture: Should this be private or not in the base Motor
        // class?  There is no external use of this method.
        virtual void config_message();

        // debug_message() displays motor-specific information that can be
        // used to assist with motor configuration.  For many motor types,
        // it is a no-op.
        // TODO Architecture: Should this be private?  It only applies to
        // Trinamic drivers so maybe there is a cleaner approach to solving
        // the stallguard debugging problem.
        virtual void debug_message();

        // read_settings(), called from init() and motors_read_settings(),
        // re-establishes the motor configuration parameters that come
        // from $ settings.
        virtual void read_settings();

        // set_homing_mode() is called from motors_set_homing_mode(),
        // which in turn is called at the beginning of a homing cycle
        // with isHoming true, and at the end with isHoming false.
        // Some motor types require differ setups for homing and
        // normal operation.
        virtual void set_homing_mode(bool isHoming);

        // set_disable() disables or enables a motor.  It is used to
        // make a motor transition between idle and non-idle states.
        virtual void set_disable(bool disable);

        // set_direction() sets the motor movement direction.  It is
        // invoked for every motion segment.
        virtual void set_direction(bool);

        // step() initiates a step operation on a motor.  It is called
        // from motors_step() for ever motor than needs to step now.
        // For ordinary step/direction motors, it sets the step pin
        // to the active state.
        virtual void step();

        // unstep() turns off the step pin, if applicable, for a motor.
        // It is called from motors_unstep() for all motors, since
        // motors_unstep() is used in many contexts where the previous
        // states of the step pins are unknown.
        virtual void unstep();

        // test(), called from init(), checks to see if a motor is
        // responsive, returning true on failure.  Typical
        // implementations also display messages to show the result.
        // TODO Architecture: Should this be private?
        virtual bool test();

        // axis_name() returns the string name of the axis associated
        // with the motor, for example "X2".  It is used by many
        // of the methods that display messages, to indicate the
        // motor to which the message applies.
        virtual char* axis_name();

        // update() is used for some types of "smart" motors that
        // can be told to move to a specific position.  It is
        // called from a periodic task.
        virtual void update();

        // can_home() returns false if the motor is incapable of
        // homing.  Homing cycles use it to exclude such motors
        // from the homing process.  Most motors can be homed.
        virtual bool can_home();

        // type_id is an enumerated value that designates
        // what kind of motor it is.  It is used to enable
        // or disable various external features, for example
        // the periodic task that is used for motors that
        // implement the "update()" method.
        // TODO Architecture: expose fine-grained capabilities
        // so that the external code does not have to
        // know which types implement which features.
        motor_class_id_t type_id;

    protected:
        // _axis_index is the axis from XYZABC, while
        // _dual_axis_index is 0 for the primary motor on that
        // axis and 1 for the ganged motor.
        // These variables are used for several purposes:
        // * Displaying the axis name in messages
        // * When reading settings, determining which setting
        //   applies to this motor
        // * For some motor types, it is necessary to maintain
        //   tables of all the motors of that type; those
        //   tables can be indexed by these variables.
        // TODO Architecture: It might be useful to cache a
        // reference to the axis settings entry.
        uint8_t _axis_index;       // X_AXIS, etc
        uint8_t _dual_axis_index;  // 0 = primary 1=ganged

        // Storage for the can_home() method
        bool _can_home = true;
    };
}
