/*
    The solenoid monitors the current position of the Axis
    it is assigned to. It uses a max and max position value and a max and min pulse
    length.

    The solenoid turns on when it is equal to or greater than _position_max

    Tests
    - Does it turn on at the transition point?
    - Doess hold duration an reduction work?
    - Does $3 reverse the direction?
    - Do homing work to move axis and set pulse currectly 
    - Does diable work?


*/

Solenoid :: Solenoid(uint8_t axis_index, gpio_num_t pwm_pin, float transition_poiont) {
    type_id = SOLENOID;
    this->axis_index = axis_index % MAX_AXES;
    this->dual_axis_index = axis_index < MAX_AXES ? 0 : 1; // 0 = primary 1 = ganged
    this->_pwm_pin = pwm_pin;
    _transition_poiont = transition_poiont;
    init();
}

void Solenoid :: init() {
    //read_settings();
    _channel_num = sys_get_next_PWM_chan_num();
    ledcSetup(_channel_num, SOLENOID_PULSE_FREQ, SOLENOID_PULSE_RES_BITS);
    ledcAttachPin(_pwm_pin, _channel_num);
    _current_pwm_duty = 0;
    is_active = true;  // as opposed to NullMotors, this is a real motor
    set_axis_name();
    config_message();
}

void Solenoid :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "%s Axis Solenoid Output:%d Max:%5.3fmm",
                   _axis_name,
                   _pwm_pin,
                   _transition_poiont);
}

void Solenoid::set_disable(bool disable) {
    if (disable  && SOLENOID_ALLOW_DISABLE)
        _write_pwm(0);

    _disabled = disable;
}

void Solenoid::update() {
    set_location();
}

void Solenoid::set_location() {
    float solenoid_pos, mpos, offset;
    uint32_t duty;
    static uint8_t hold_counter;
    static uint32_t last_duty = 0;

    if (_disabled  && SOLENOID_ALLOW_DISABLE)
        return;

    _pwm_pulse_min = SOLENOID_MIN_PULSE;
    _pwm_pulse_max = SOLENOID_MAX_PULSE;

    // Is direction reversed via the $3 command?
    if (bit_istrue(settings.dir_invert_mask, bit(axis_index))) 	// normal direction
        swap(_pwm_pulse_min, _pwm_pulse_max);

    // $23 sets homing direction bit_true homes in positive direction
    if (sys.state == STATE_HOMING) {
        bool home_positive;
        home_positive = bit_isfalse(settings.homing_dir_mask, bit(axis_index)); // no bit  = home positive        

        if (bit_istrue(settings.dir_invert_mask, bit(axis_index)))
            home_positive = ! home_positive;

        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%s Solenoid Home Pos2:%d", _axis_name, home_positive);
            
        if (home_positive)
            duty = _pwm_pulse_max;
        else
            duty = _pwm_pulse_min;

    } else {
        mpos = system_convert_axis_steps_to_mpos(sys_position, axis_index);  // get the axis machine position in mm
        offset = gc_state.coord_system[axis_index] + gc_state.coord_offset[axis_index]; // get the current axis work offset
        solenoid_pos = mpos - offset; // determine the current work position

        if (solenoid_pos >= _position_max)
            duty = _pwm_pulse_max;
        else
            duty = _pwm_pulse_min;
    }


    if (duty != last_duty) {
        hold_counter = 0; // reset the hold counter
        last_duty = duty; // remember the duty
    } else {
        if (duty != SOLENOID_MIN_PULSE) {
            if (hold_counter >= SOLENOID_HOLD_DELAY) {
                duty *= (SOLENOID_HOLD_PERCENTAGE / 100.0); // apply the hold current
            } else
                hold_counter++;
        }
    }

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%s Solenoid Pwm %d", _axis_name, duty);

    _write_pwm(duty);


}

