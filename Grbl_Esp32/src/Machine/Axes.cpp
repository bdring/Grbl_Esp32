#include "Axes.h"

#include "../Motors/Motor.h"
#include "../Motors/NullMotor.h"
#include "../NutsBolts.h"
#include "../MotionControl.h"
#include "../Stepper.h"     // stepper_id_t
#include "MachineConfig.h"  // config->

namespace Machine {
    uint32_t Axes::posLimitMask = 0;
    uint32_t Axes::negLimitMask = 0;
    uint32_t Axes::homingMask   = 0;
    uint32_t Axes::limitMask    = 0;

    Axes::Axes() : _axis() {
        for (int i = 0; i < MAX_N_AXIS; ++i) {
            _axis[i] = nullptr;
        }
    }

    void Axes::init() {
        log_info("Axis count " << config->_axes->_numberAxis);

        if (_sharedStepperDisable.defined()) {
            _sharedStepperDisable.setAttr(Pin::Attr::Output);
            _sharedStepperDisable.report("Shared stepper disable");
        }

        // certain motors need features to be turned on. Check them here
        for (uint8_t axis = X_AXIS; axis < _numberAxis; axis++) {
            auto a = _axis[axis];
            if (a) {
                a->init();
            }
        }
    }

    void IRAM_ATTR Axes::set_disable(int axis, bool disable) {
        for (int gang_index = 0; gang_index < Axis::MAX_NUMBER_GANGED; gang_index++) {
            auto a = _axis[axis]->_gangs[gang_index]->_motor;
            a->set_disable(disable);
        }

        // invert only inverts the global stepper disable pin.
        _sharedStepperDisable.write(disable);
    }

    void IRAM_ATTR Axes::set_disable(bool disable) {
        for (int axis = 0; axis < _numberAxis; axis++) {
            set_disable(axis, disable);
        }

        _sharedStepperDisable.write(disable);
    }

    void Axes::read_settings() {
        //log_info("Read Settings");
        for (uint8_t axis = X_AXIS; axis < _numberAxis; axis++) {
            for (uint8_t gang_index = 0; gang_index < Axis::MAX_NUMBER_GANGED; gang_index++) {
                auto a = _axis[axis];
                if (!a) {
                    log_info("No specification for axis " << axis);
                    break;
                }
                auto g = a->_gangs[gang_index];
                if (!g) {
                    log_info("No specification for axis " << axis << " gang " << gang_index);
                    break;
                }
                auto m = g->_motor;
                if (!m) {
                    log_info("No motor for axis " << axis << " gang " << gang_index);
                }
                m->read_settings();
            }
        }
    }

    // use this to tell all the motors what the current homing mode is
    // They can use this to setup things like Stall
    uint32_t Axes::set_homing_mode(uint8_t homing_mask, bool isHoming) {
        release_all_motors();  // On homing transitions, cancel all motor lockouts
        uint8_t can_home = 0;

        for (uint8_t axis = X_AXIS; axis < _numberAxis; axis++) {
            if (bitnum_istrue(homing_mask, axis)) {
                auto a = _axis[axis];
                if (a != nullptr) {
                    auto motor = a->_gangs[0]->_motor;

                    if (motor->set_homing_mode(isHoming)) {
                        bitnum_true(can_home, axis);
                    }

                    for (uint8_t gang_index = 1; gang_index < Axis::MAX_NUMBER_GANGED; gang_index++) {
                        auto a2 = _axis[axis]->_gangs[gang_index]->_motor;
                        a2->set_homing_mode(isHoming);
                    }
                }
            }
        }

        return can_home;
    }

    void Axes::release_all_motors() { _motorLockoutMask = 0xffffffff; }
    void Axes::stop_motors(uint32_t mask) { bit_false(_motorLockoutMask, mask); }

    void IRAM_ATTR Axes::step(uint8_t step_mask, uint8_t dir_mask) {
        auto n_axis = _numberAxis;
        //log_info("motors_set_direction_pins:0x%02X", onMask);

        // Set the direction pins, but optimize for the common
        // situation where the direction bits haven't changed.
        static uint8_t previous_dir = 255;  // should never be this value
        if (dir_mask != previous_dir) {
            previous_dir = dir_mask;

            for (int axis = X_AXIS; axis < n_axis; axis++) {
                bool thisDir = bitnum_istrue(dir_mask, axis);

                for (uint8_t gang_index = 0; gang_index < Axis::MAX_NUMBER_GANGED; gang_index++) {
                    auto a = _axis[axis]->_gangs[gang_index]->_motor;

                    if (a != nullptr) {
                        a->set_direction(thisDir);
                    }
                }
            }
            config->_stepping->waitDirection();
        }

        config->_stepping->startPulseTimer();

        // Turn on step pulses for motors that are supposed to step now
        for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
            if (bitnum_istrue(step_mask, axis)) {
                auto a = _axis[axis];

                if (bitnum_istrue(_motorLockoutMask, axis)) {
                    a->_gangs[0]->_motor->step();
                }
                if (bitnum_istrue(_motorLockoutMask, axis + 16)) {
                    a->_gangs[1]->_motor->step();
                }
            }
        }
    }

    // Turn all stepper pins off
    void IRAM_ATTR Axes::unstep() {
        config->_stepping->waitPulse();
        auto n_axis = _numberAxis;
        for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
            for (uint8_t gang_index = 0; gang_index < Axis::MAX_NUMBER_GANGED; gang_index++) {
                auto a = _axis[axis]->_gangs[gang_index]->_motor;
                a->unstep();
                a->unstep();
            }
        }

        config->_stepping->finishPulse();
    }

    // Some small helpers to find the axis index and axis ganged index for a given motor. This
    // is helpful for some motors that need this info, as well as debug information.
    size_t Axes::findAxisIndex(const Motors::Motor* const motor) const {
        for (int i = 0; i < _numberAxis; ++i) {
            for (int j = 0; j < Axis::MAX_NUMBER_GANGED; ++j) {
                if (_axis[i] != nullptr && _axis[i]->hasMotor(motor)) {
                    return i;
                }
            }
        }

        Assert(false, "Cannot find axis for motor. Something wonky is going on here...");
        return SIZE_MAX;
    }

    size_t Axes::findAxisGanged(const Motors::Motor* const motor) const {
        for (int i = 0; i < _numberAxis; ++i) {
            if (_axis[i] != nullptr && _axis[i]->hasMotor(motor)) {
                for (int j = 0; j < Axis::MAX_NUMBER_GANGED; ++j) {
                    if (_axis[i]->_gangs[j]->_motor == motor) {
                        return j;
                    }
                }
            }
        }

        Assert(false, "Cannot find axis for motor. Something wonky is going on here...");
        return SIZE_MAX;
    }

    // Configuration helpers:

    void Axes::group(Configuration::HandlerBase& handler) {
        handler.item("number_axis", _numberAxis);
        handler.item("shared_stepper_disable", _sharedStepperDisable);

        // Handle axis names xyzabc.  handler.section is inferred
        // from a template.
        char tmp[3];
        tmp[2] = '\0';

        for (size_t i = 0; i < MAX_N_AXIS; ++i) {
            tmp[0] = tolower(_names[i]);
            tmp[1] = '\0';

            handler.section(tmp, _axis[i], i);
        }
    }

    void Axes::afterParse() {
        for (size_t i = 0; i < MAX_N_AXIS; ++i) {
            if (_axis[i] == nullptr) {
                _axis[i] = new Axis(i);
            }
        }
    }

    Axes::~Axes() {
        for (int i = 0; i < MAX_N_AXIS; ++i) {
            delete _axis[i];
        }
    }
}
