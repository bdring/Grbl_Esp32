#include "LimitPin.h"
#include "Axes.h"
#include "MachineConfig.h"  // config

#include "../NutsBolts.h"      // bitnum_true etc
#include "../MotionControl.h"  // mc_reset
#include "../Limits.h"
#include "../System.h"  // sys_rt_exec_alarm

namespace Machine {
    LimitPin::LimitPin(Pin& pin, int axis, int gang, int direction, bool& pHardLimits) :
        _axis(axis), _gang(gang), _value(false), _pHardLimits(pHardLimits), _pin(pin) {
        String sDir;
        // Select one or two bitmask variables to receive the switch data
        switch (direction) {
            case 1:
                _posLimits = &Axes::posLimitMask;
                _negLimits = nullptr;
                sDir       = "Pos";
                break;
            case -1:
                _posLimits = nullptr;
                _negLimits = &Axes::negLimitMask;
                sDir       = "Neg";
                break;
            case 0:
                _posLimits = &Axes::posLimitMask;
                _negLimits = &Axes::negLimitMask;
                sDir       = "All";
                break;
            default:  // invalid
                _negLimits   = nullptr;
                _posLimits   = nullptr;
                _pHardLimits = false;
                break;
        }
        // Set a bitmap with bits to represent the axis and which motors are affected,
        // and construct a legend string like "Y Axis Limit" or "Y Axis Gang0 Limit".
        // The bitmap looks like CBAZYXcbazyx where gang0 motors are in the lower
        // bits and gang1 in the upper bits.  If both gangs are affected, there is
        // a bit set in both the upper and lower groups.
        switch (gang) {
            case 0:
                _bitmask = 1;  // Set bit as for x axis gang 0
                _legend  = " Gang0";
                break;
            case 1:
                _bitmask = 1 << MAX_N_AXIS;  // Set bit as for X axis gang 1
                _legend  = " Gang1";
                break;
            case -1:  // Axis level switch - set both bits
                _bitmask = (1 << MAX_N_AXIS) | 1;
                _legend  = "";
                break;
            default:
                break;
        }
        _bitmask <<= axis;  // Shift the bits into position
        _legend = String(config->_axes->axisName(axis)) + " Axis" + _legend + " " + sDir + " Limit";
    }

    void IRAM_ATTR LimitPin::handleISR() {
        bool pinState = _pin.read();
        _value        = _pin.read();
        if (_value) {
            if (_posLimits != nullptr) {
                bit_true(*_posLimits, _bitmask);
            }
            if (_negLimits != nullptr) {
                bit_true(*_negLimits, _bitmask);
            }
        } else {
            if (_posLimits != nullptr) {
                bit_false(*_posLimits, _bitmask);
            }
            if (_negLimits != nullptr) {
                bit_false(*_negLimits, _bitmask);
            }
        }
        if (sys.state != State::Alarm && sys.state != State::ConfigAlarm && sys.state != State::Homing) {
            if (_pHardLimits && sys_rt_exec_alarm == ExecAlarm::None) {
#if 0

                if (config->_softwareDebounceMs) {
                    // send a message to wakeup the task that rechecks the switches after a small delay
                    int evt;
                    xQueueSendFromISR(limit_sw_queue, &evt, NULL);
                    return;
                }
#endif

                // log_debug("Hard limits");  // This might not work from ISR context
                mc_reset();                                // Initiate system kill.
                sys_rt_exec_alarm = ExecAlarm::HardLimit;  // Indicate hard limit critical event
            }
        }
    }

    void LimitPin::init() {
        if (_pin.undefined()) {
            return;
        }
        bitnum_true(Axes::limitMask, _axis);
        _pin.report(_legend.c_str());
        auto attr = Pin::Attr::Input | Pin::Attr::ISR;
        if (_pin.capabilities().has(Pins::PinCapabilities::PullUp)) {
            attr = attr | Pin::Attr::PullUp;
        }
        _pin.setAttr(attr);
        _pin.attachInterrupt<LimitPin, &LimitPin::handleISR>(this, CHANGE);
    }

    LimitPin::~LimitPin() { _pin.detachInterrupt(); }
}
