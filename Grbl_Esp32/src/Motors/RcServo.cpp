/*
    RcServo.cpp

    This allows an RcServo to be used like any other motor. Serrvos
    do have limitation in travel and speed, so you do need to respect that.

    Part of Grbl_ESP32

    2020 -	Bart Dring

    The servo's travel will be mapped against the axis with $X/MaxTravel

    The rotation can be inverted with by $Stepper/DirInvert    

    Homing simply sets the axis Mpos to the endpoint as determined by $Homing/DirInver    

    Calibration is part of the setting (TBD) fixed at 1.00 now

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

#include "RcServo.h"

namespace Motors {
    RcServo::RcServo() {}

    RcServo::RcServo(uint8_t axis_index, uint8_t pwm_pin, float cal_min, float cal_max) {
        type_id                = RC_SERVO_MOTOR;
        this->_axis_index      = axis_index % MAX_AXES;
        this->_dual_axis_index = axis_index < MAX_AXES ? 0 : 1;  // 0 = primary 1 = ganged
        this->_pwm_pin         = pwm_pin;
        _cal_min               = cal_min;
        _cal_max               = cal_max;
        init();
    }

    void RcServo::init() {
        read_settings();
        _channel_num = sys_get_next_PWM_chan_num();
        ledcSetup(_channel_num, SERVO_PULSE_FREQ, SERVO_PULSE_RES_BITS);
        ledcAttachPin(_pwm_pin, _channel_num);
        _current_pwm_duty = 0;
        is_active         = true;   // as opposed to NullMotors, this is a real motor
        _can_home         = false;  // this axis cannot be confensionally homed

        set_axis_name();
        config_message();
    }

    void RcServo::config_message() {
        grbl_msg_sendf(CLIENT_SERIAL,
                       MsgLevel::Info,
                       "%s Axis RC Servo Pin:%d Pulse Len(%.0f,%.0f) Limits(%.3f,%.3f)",
                       _axis_name,
                       _pwm_pin,
                       _pwm_pulse_min,
                       _pwm_pulse_max,
                       _position_min,
                       _position_max);
    }

    void RcServo::_write_pwm(uint32_t duty) {
        // to prevent excessive calls to ledcWrite, make sure duty hass changed
        if (duty == _current_pwm_duty) {
            return;
        }

        _current_pwm_duty = duty;
        ledcWrite(_channel_num, duty);
    }

    // sets the PWM to zero. This allows most servos to be manually moved
    void RcServo::set_disable(bool disable) {
        return;
        _disabled = disable;
        if (_disabled) {
            _write_pwm(0);
        }
    }

    // Homing justs sets the new system position and the servo will move there
    void RcServo::set_homing_mode(uint8_t homing_mask, bool isHoming) {
        float home_pos = 0.0;

        sys_position[_axis_index] =
            axis_settings[_axis_index]->home_mpos->get() * axis_settings[_axis_index]->steps_per_mm->get();  // convert to steps

        set_location();  // force the PWM to update now

        vTaskDelay(750);  // give time to move
    }

    void RcServo::update() { set_location(); }

    void RcServo::set_location() {
        uint32_t servo_pulse_len;
        float    servo_pos, mpos, offset;

        read_settings();

        if (sys.state == State::Alarm) {
            set_disable(true);
            return;
        }

        mpos = system_convert_axis_steps_to_mpos(sys_position, _axis_index);  // get the axis machine position in mm
        // TBD working in MPos
        offset    = 0;  // gc_state.coord_system[axis_index] + gc_state.coord_offset[axis_index];  // get the current axis work offset
        servo_pos = mpos - offset;  // determine the current work position

        // determine the pulse length
        servo_pulse_len = (uint32_t)mapConstrain(servo_pos, _position_min, _position_max, _pwm_pulse_min, _pwm_pulse_max);

        _write_pwm(servo_pulse_len);
    }

    void RcServo::read_settings() {
        float travel = axis_settings[_axis_index]->max_travel->get();
        float mpos   = axis_settings[_axis_index]->home_mpos->get();
        //float max_mpos, min_mpos;

        if (bit_istrue(homing_dir_mask->get(), bit(_axis_index))) {
            _position_min = mpos;
            _position_max = mpos + travel;
        } else {
            _position_min = mpos - travel;
            _position_max = mpos;
        }

        _pwm_pulse_min = SERVO_MIN_PULSE * _cal_min;
        _pwm_pulse_max = SERVO_MAX_PULSE * _cal_max;

        if (bit_istrue(dir_invert_mask->get(), bit(_axis_index)))  // normal direction
            swap(_pwm_pulse_min, _pwm_pulse_max);
    }

}
