/*
	TickleMeLaser.h

	This is similar to the Laser except that it allows for a tickle pulse 
  to meet the requirements to keep RF CO2 lasers (like Synrad tubes) 
  primed for operation.

	Add on Spindle Code for Grbl_ESP32
	2020 -	Taylor Fry

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
#include "TickleMeLaser.h"

// ===================================== Laser ==============================================

namespace Spindles {

    uint32_t TickleMeLaser::set_rpm(uint32_t rpm) {
        uint32_t pwm_value;

        if (_output_pin == UNDEFINED_PIN) {
            return rpm;
        }

        //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "set_rpm(%d)", rpm);

        // apply override
        rpm = rpm * sys.spindle_speed_ovr / 100;  // Scale by spindle speed override value (uint8_t percent)

        // apply limits
        if ((_min_rpm >= _max_rpm) || (rpm >= _max_rpm)) {
            rpm = _max_rpm;
        } else if (rpm != 0 && rpm <= _min_rpm) {
            rpm = _min_rpm;
        }

        sys.spindle_speed = rpm;

        if (_piecewide_linear) {
            //pwm_value = piecewise_linear_fit(rpm); TODO
            pwm_value = 0;
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Warning: Linear fit not implemented yet.");

        } else {
            if (rpm == 0) {
                //TICKLE BUSINESS HERE, I just set it to 5, as at 5khz 5/1000 is a duty cycle of .5% which
                //is 1us of a 200us 5khz pulse.  The machine still expects this 5khz pulse even if a different
                //frequency is used to fire the laser.  This would involve changing the pwm frequency on the fly
                
                //for the branch that allows on the fly frequency changes, set it to 5k here.
                pwm_value = 5;
            } else {
                //for the branch that allows on the fly frequency changes, set it to desired frequency here.
                //the biggest gain for rf lasers on changing frequency is getting rid of lines on edges during fast cuts
                //ultimatley the frequency value should be driven by the velocity of the move
                pwm_value = map_uint32_t(rpm, _min_rpm, _max_rpm, _pwm_min_value, _pwm_max_value);
            }
        }

        set_enable_pin(_current_state != SpindleState::Disable);
        set_output(pwm_value);

        return 0;
    }

}
