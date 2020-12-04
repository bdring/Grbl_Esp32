#include "UnipolarMotor.h"

namespace Motors {
    UnipolarMotor::UnipolarMotor(uint8_t axis_index, uint8_t pin_phase0, uint8_t pin_phase1, uint8_t pin_phase2, uint8_t pin_phase3) :
        Motor(axis_index), _pin_phase0(pin_phase0), _pin_phase1(pin_phase1), _pin_phase2(pin_phase2),
        _pin_phase3(pin_phase3),

        _half_step(true)  // TODO read from settings ... microstep > 1 = half step
    {}

    void UnipolarMotor::init() {
        pinMode(_pin_phase0, OUTPUT);
        pinMode(_pin_phase1, OUTPUT);
        pinMode(_pin_phase2, OUTPUT);
        pinMode(_pin_phase3, OUTPUT);
        _current_phase = 0;
        config_message();
    }

    void UnipolarMotor::config_message() {
        grbl_msg_sendf(CLIENT_SERIAL,
                       MsgLevel::Info,
                       "%s Unipolar Stepper Ph0:%s Ph1:%s Ph2:%s Ph3:%s %s",
                       reportAxisNameMsg(_axis_index, _dual_axis_index),
                       pinName(_pin_phase0).c_str(),
                       pinName(_pin_phase1).c_str(),
                       pinName(_pin_phase2).c_str(),
                       pinName(_pin_phase3).c_str(),
                       reportAxisLimitsMsg(_axis_index));
    }

    void UnipolarMotor::set_disable(bool disable) {
        if (disable) {
            digitalWrite(_pin_phase0, 0);
            digitalWrite(_pin_phase1, 0);
            digitalWrite(_pin_phase2, 0);
            digitalWrite(_pin_phase3, 0);
        }
        _enabled = !disable;
    }

    void UnipolarMotor::set_direction(bool dir) { _dir = dir; }

    void UnipolarMotor::step() {
        uint8_t _phase[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  // temporary phase values...all start as off
        uint8_t phase_max;

        if (!_enabled)
            return;  // don't do anything, phase is not changed or lost

        phase_max = _half_step ? 7 : 3;

        if (_dir) {  // count up
            _current_phase = _current_phase == phase_max ? 0 : _current_phase + 1;
        } else {  // count down
            _current_phase = _current_phase == 0 ? phase_max : _current_phase - 1;
        }
        /*
			8 Step : A – AB – B – BC – C – CD – D – DA
			4 Step : AB – BC – CD – DA

			Step		IN4	IN3	IN2	IN1
			A 		0 	0 	0 	1
			AB		0	0	1	1
			B		0	0	1	0
			BC		0	1	1	0
			C		0	1	0	0
			CD		1	1	0	0
			D		1	0	0	0
			DA		1	0	0	1
		*/
        if (_half_step) {
            switch (_current_phase) {
                case 0:
                    _phase[0] = 1;
                    break;
                case 1:
                    _phase[0] = 1;
                    _phase[1] = 1;
                    break;
                case 2:
                    _phase[1] = 1;
                    break;
                case 3:
                    _phase[1] = 1;
                    _phase[2] = 1;
                    break;
                case 4:
                    _phase[2] = 1;
                    break;
                case 5:
                    _phase[2] = 1;
                    _phase[3] = 1;
                    break;
                case 6:
                    _phase[3] = 1;
                    break;
                case 7:
                    _phase[3] = 1;
                    _phase[0] = 1;
                    break;
            }
        } else {
            switch (_current_phase) {
                case 0:
                    _phase[0] = 1;
                    _phase[1] = 1;
                    break;
                case 1:
                    _phase[1] = 1;
                    _phase[2] = 1;
                    break;
                case 2:
                    _phase[2] = 1;
                    _phase[3] = 1;
                    break;
                case 3:
                    _phase[3] = 1;
                    _phase[0] = 1;
                    break;
            }
        }
        digitalWrite(_pin_phase0, _phase[0]);
        digitalWrite(_pin_phase1, _phase[1]);
        digitalWrite(_pin_phase2, _phase[2]);
        digitalWrite(_pin_phase3, _phase[3]);
    }
}
