/*
    RcServoServoClass.cpp

    This allows an RcServo to be used like any other motor. Serrvos
    do have limitation in travel and speed, so you do need to respect that.

    Part of Grbl_ESP32

    2020 -	Bart Dring

    Servos have a limited travel, so they map the travel across a range in
    the current work coordinatee system. The servo can only travel as far
    as the range, but the internal axis value can keep going.

    Range: The range is specified in the machine definition file with...
    #define X_SERVO_RANGE_MIN       0.0
    #define X_SERVO_RANGE_MAX       5.0

    Direction: The direction can be changed using the $3 setting for the axis

    Homing: During homing, the servo will move to one of the endpoints. The
    endpoint is determined by the $23 or $HomingDirInvertMask setting for the axis.
    Do not define a homing cycle for the axis with the servo.
    You do need at least 1 homing cycle.  TODO: Fix this

    Calibration. You can tweak the endpoints using the $10n or nStepsPerMm and
    $13n or $xMaxTravel setting, where n is the axis.
    The value is a percent. If you secify a percent outside the
    the range specified by the values below, it will be reset to 100.0 (100% ... no change)
    The calibration adjusts in direction of positive momement, so a value above 100% moves
    towards the higher axis value.

    #define SERVO_CAL_MIN
    #define SERVO_CAL_MAX

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

RcServo :: RcServo() {

}

RcServo :: RcServo(uint8_t axis_index, uint8_t pwm_pin, float min, float max) {
    type_id = RC_SERVO_MOTOR;
    this->axis_index = axis_index % MAX_AXES;
    this->dual_axis_index = axis_index < MAX_AXES ? 0 : 1; // 0 = primary 1 = ganged
    this->_pwm_pin = pwm_pin;
    _position_min = min;
    _position_max = max;
    init();
}

void RcServo :: init() {
    read_settings();
    _channel_num = sys_get_next_PWM_chan_num();
    ledcSetup(_channel_num, SERVO_PULSE_FREQ, SERVO_PULSE_RES_BITS);
    ledcAttachPin(_pwm_pin, _channel_num);
    _current_pwm_duty = 0;
    is_active = true;  // as opposed to NullMotors, this is a real motor
    set_axis_name();
    config_message();
}

void RcServo :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "%s Axis RC Servo motor Output:%d Min:%5.3fmm Max:%5.3fmm",
                   _axis_name,
                   _pwm_pin,
                   _position_min,
                   _position_max);
}

void RcServo::_write_pwm(uint32_t duty) {
    // to prevent excessive calls to ledcWrite, make sure duty hass changed
    if (duty == _current_pwm_duty)
        return;

    _current_pwm_duty = duty;

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%s Servo Pwm %d", _axis_name, duty);
    ledcWrite(_channel_num, duty);
}

// sets the PWM to zero. This allows most servos to be manually moved
void RcServo::set_disable(bool disable) {
    return;
    _disabled = disable;
    if (_disabled)
        _write_pwm(0);
}

void RcServo::set_homing_mode(bool is_homing, bool isHoming) {
    float home_pos = 0.0;

    if (!is_homing)
        return;

    if (bit_istrue(homing_dir_mask->get(), bit(axis_index)))
        home_pos =  _position_min;
    else
        home_pos = _position_max;

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Servo set home %d %3.2f", is_homing, home_pos);
    sys_position[axis_index] = home_pos * axis_settings[axis_index]->steps_per_mm->get(); // convert to steps

}

void RcServo::update() {
    set_location();
}

void RcServo::set_location() {
    uint32_t servo_pulse_len;
    float servo_pos, mpos, offset;
    // skip location if we are in alarm mode

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "locate");
    _get_calibration();

    if (sys.state == STATE_ALARM) {
        set_disable(true);
        return;
    }

    mpos = system_convert_axis_steps_to_mpos(sys_position, axis_index);  // get the axis machine position in mm
    offset = gc_state.coord_system[axis_index] + gc_state.coord_offset[axis_index]; // get the current axis work offset
    servo_pos = mpos - offset; // determine the current work position

    // determine the pulse length
    servo_pulse_len = (uint32_t)mapConstrain(servo_pos, _position_min, _position_max, _pwm_pulse_min, _pwm_pulse_max);

    _write_pwm(servo_pulse_len);

}

void RcServo::read_settings() {
    _get_calibration();
}

// this should change to use its own settings.
void RcServo::_get_calibration() {
    float _cal_min = 1.0;
    float _cal_max = 1.0;

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Read settings");

    // make sure the min is in range
    if ((axis_settings[axis_index]->steps_per_mm->get() < SERVO_CAL_MIN) || (axis_settings[axis_index]->steps_per_mm->get() > SERVO_CAL_MAX)) {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Servo calibration ($10%d) value error. Reset to 100", axis_index);
        char reset_val[] = "100";
        axis_settings[axis_index]->steps_per_mm->setStringValue(reset_val);
    }

    // make sure the max is in range
    // Note: Max travel is set positive via $$, but stored as a negative number
    if ((axis_settings[axis_index]->max_travel->get() < SERVO_CAL_MIN) || (axis_settings[axis_index]->max_travel->get() > SERVO_CAL_MAX)) {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Servo calibration ($13%d) value error. %3.2f Reset to 100", axis_index, axis_settings[axis_index]->max_travel->get());
        char reset_val[] = "100";
        axis_settings[axis_index]->max_travel->setStringValue(reset_val);
    }

    _pwm_pulse_min = SERVO_MIN_PULSE;
    _pwm_pulse_max = SERVO_MAX_PULSE;


    if (bit_istrue(dir_invert_mask->get(), bit(axis_index))) {	// normal direction
        _cal_min = 2.0 - (axis_settings[axis_index]->steps_per_mm->get() / 100.0);
        _cal_max = 2.0 - (axis_settings[axis_index]->max_travel->get() / 100.0);
        swap(_pwm_pulse_min, _pwm_pulse_max);
    } else { // inverted direction
        _cal_min = (axis_settings[axis_index]->steps_per_mm->get() / 100.0);
        _cal_max = (axis_settings[axis_index]->max_travel->get() / 100.0);
    }

    _pwm_pulse_min *= _cal_min;
    _pwm_pulse_max *= _cal_max;

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Servo calibration min:%1.2f max %1.2f", _pwm_pulse_min, _pwm_pulse_max);

}