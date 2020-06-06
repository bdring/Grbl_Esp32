/*
    StandardStepperClass.cpp

    This is used for a stepper motor that just requires step and direction
    pins.
    TODO: Add an enable pin

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

StandardStepper :: StandardStepper() {

}

StandardStepper :: StandardStepper(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint8_t disable_pin) {
    type_id = STANDARD_MOTOR;
    this->axis_index = axis_index % MAX_AXES;
    this->dual_axis_index = axis_index < MAX_AXES ? 0 : 1; // 0 = primary 1 = ganged
    this->step_pin = step_pin;
    this->dir_pin = dir_pin;
    this->disable_pin = disable_pin;
    init();
}

void StandardStepper :: init() {
    _is_homing = false;
    is_active = true;  // as opposed to NullMotors, this is a real motor
    set_axis_name();
    init_step_dir_pins();
    config_message();
}

void StandardStepper :: init_step_dir_pins() {
    // TODO Step pin, but RMT complicates things
    _invert_step_pin = bit_istrue(step_invert_mask->get(), bit(axis_index));
    pinMode(dir_pin, OUTPUT);

#ifdef USE_RMT_STEPS
    rmtConfig.rmt_mode = RMT_MODE_TX;
    rmtConfig.clk_div = 20;
    rmtConfig.mem_block_num = 2;
    rmtConfig.tx_config.loop_en = false;
    rmtConfig.tx_config.carrier_en = false;
    rmtConfig.tx_config.carrier_freq_hz = 0;
    rmtConfig.tx_config.carrier_duty_percent = 50;
    rmtConfig.tx_config.carrier_level = RMT_CARRIER_LEVEL_LOW;
    rmtConfig.tx_config.idle_output_en = true;


#ifdef STEP_PULSE_DELAY
    rmtItem[0].duration0 = STEP_PULSE_DELAY * 4;
#else
    rmtItem[0].duration0 = 1;
#endif

    rmtItem[0].duration1 = 4 * pulse_microseconds->get();
    rmtItem[1].duration0 = 0;
    rmtItem[1].duration1 = 0;

    rmt_chan_num[axis_index][dual_axis_index] = sys_get_next_RMT_chan_num();
    rmt_set_source_clk((rmt_channel_t)rmt_chan_num[axis_index][dual_axis_index], RMT_BASECLK_APB);
    rmtConfig.channel = (rmt_channel_t)rmt_chan_num[axis_index][dual_axis_index];
    rmtConfig.tx_config.idle_level = _invert_step_pin ? RMT_IDLE_LEVEL_HIGH : RMT_IDLE_LEVEL_LOW;
    rmtConfig.gpio_num = gpio_num_t(step_pin); // c is a wacky lang
    rmtItem[0].level0 = rmtConfig.tx_config.idle_level;
    rmtItem[0].level1 = !rmtConfig.tx_config.idle_level;
    rmt_config(&rmtConfig);
    rmt_fill_tx_items(rmtConfig.channel, &rmtItem[0], rmtConfig.mem_block_num, 0);

#else
    pinMode(step_pin, OUTPUT);

#endif // USE_RMT_STEPS
    if (disable_pin != UNDEFINED_PIN)
        pinMode(disable_pin, OUTPUT);

}


void StandardStepper :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "%s Axis standard stepper motor Step:%s Dir:%s Disable:%s",
                   _axis_name,
                   pinName(step_pin),
                   pinName(dir_pin),
                   pinName(disable_pin));
}

void StandardStepper :: set_direction_pins(uint8_t onMask) {
    digitalWrite(dir_pin, (onMask & (1 << axis_index)));
}

void StandardStepper :: set_disable(bool disable) {

    if (disable_pin != UNDEFINED_PIN)
        digitalWrite(disable_pin, disable);
}
