/*
    RcServo.cpp

    This allows an RcServo to be used like any other motor. Servos
    do have limitation in travel and speed, so you do need to respect that.

    Part of Grbl_ESP32

    2020 -	Bart Dring

    The servo's travel will be mapped against the axis with $X/MaxTravel

    The rotation can be inverted with by $Stepper/DirInvert

    Homing simply sets the axis Mpos to the endpoint as determined by $Homing/DirInvert

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
    RcServo::RcServo(uint8_t axis_index, uint8_t pwm_pin) : Servo(axis_index), _pwm_pin(pwm_pin) {}

    void RcServo::init() {
        char* setting_cal_min = (char*)malloc(20);
        sprintf(setting_cal_min, "%c/RcServo/Cal/Min", report_get_axis_letter(_axis_index));  //
        rc_servo_cal_min = new FloatSetting(EXTENDED, WG, NULL, setting_cal_min, 1.0, 0.5, 2.0);

        char* setting_cal_max = (char*)malloc(20);
        sprintf(setting_cal_max, "%c/RcServo/Cal/Max", report_get_axis_letter(_axis_index));  //
        rc_servo_cal_max = new FloatSetting(EXTENDED, WG, NULL, setting_cal_max, 1.0, 0.5, 2.0);

        rc_servo_cal_min->load();
        rc_servo_cal_max->load();

        read_settings();
        _channel_num = sys_get_next_PWM_chan_num();
        ledcSetup(_channel_num, SERVO_PULSE_FREQ, SERVO_PULSE_RES_BITS);
        ledcAttachPin(_pwm_pin, _channel_num);
        _current_pwm_duty = 0;
        _disabled         = true;
        config_message();
        startUpdateTask();
    }

    void RcServo::config_message() {
        grbl_msg_sendf(CLIENT_SERIAL,
                       MsgLevel::Info,
                       "%s RC Servo Pin:%d Pulse Len(%.0f,%.0f) %s",
                       reportAxisNameMsg(_axis_index, _dual_axis_index),
                       _pwm_pin,
                       _pwm_pulse_min,
                       _pwm_pulse_max,
                       reportAxisLimitsMsg(_axis_index));
    }

    void RcServo::_write_pwm(uint32_t duty) {
        // to prevent excessive calls to ledcWrite, make sure duty has changed
        if (duty == _current_pwm_duty) {
            return;
        }

        _current_pwm_duty = duty;
        ledcWrite(_channel_num, duty);
    }

    // sets the PWM to zero. This allows most servos to be manually moved
    void RcServo::set_disable(bool disable) {
        _disabled = disable;
        if (_disabled) {
            _write_pwm(0);
        }
    }

    // Homing justs sets the new system position and the servo will move there
    bool RcServo::set_homing_mode(bool isHoming) {
        sys_position[_axis_index] =
            axis_settings[_axis_index]->home_mpos->get() * axis_settings[_axis_index]->steps_per_mm->get();  // convert to steps

        set_location();   // force the PWM to update now
        vTaskDelay(750);  // give time to move
        return false;     // Cannot be homed in the conventional way
    }

    void RcServo::update() { set_location(); }

    void RcServo::set_location() {
        uint32_t servo_pulse_len;
        float    servo_pos, mpos, offset;

        if (_disabled)
            return;

        read_settings();

        mpos = system_convert_axis_steps_to_mpos(sys_position, _axis_index);  // get the axis machine position in mm
        // TBD working in MPos
        offset    = 0;  // gc_state.coord_system[axis_index] + gc_state.coord_offset[axis_index];  // get the current axis work offset
        servo_pos = mpos - offset;  // determine the current work position

        // determine the pulse length
        servo_pulse_len = (uint32_t)mapConstrain(
            servo_pos, limitsMinPosition(_axis_index), limitsMaxPosition(_axis_index), _pwm_pulse_min, _pwm_pulse_max);

        _write_pwm(servo_pulse_len);
    }

    void RcServo::read_settings() {
        if (bitnum_istrue(dir_invert_mask->get(), _axis_index)) {
            // swap the pwm values
            _pwm_pulse_min = SERVO_MAX_PULSE * (1.0 + (1.0 - rc_servo_cal_min->get()));
            _pwm_pulse_max = SERVO_MIN_PULSE * (1.0 + (1.0 - rc_servo_cal_max->get()));

        } else {
            _pwm_pulse_min = SERVO_MIN_PULSE * rc_servo_cal_min->get();
            _pwm_pulse_max = SERVO_MAX_PULSE * rc_servo_cal_max->get();
        }
    }
}
