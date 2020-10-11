/*
    StandardStepper.cpp

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

#include "StandardStepper.h"

namespace Motors {
    StandardStepper::StandardStepper(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint8_t disable_pin) :
        Motor(STANDARD_MOTOR, axis_index), _step_pin(step_pin), _dir_pin(dir_pin), _disable_pin(disable_pin) {
        init();
    }

    void StandardStepper::init() {
        is_active    = true;  // as opposed to NullMotors, this is a real motor
        init_step_dir_pins();
        read_settings();
        config_message();
    }

    void StandardStepper::init_step_dir_pins() {
        _invert_step_pin = bitnum_istrue(step_invert_mask->get(), _axis_index);
        _invert_dir_pin  = bitnum_istrue(dir_invert_mask->get(), _axis_index);
        pinMode(_dir_pin, OUTPUT);

#ifdef USE_RMT_STEPS
        rmtConfig.rmt_mode                       = RMT_MODE_TX;
        rmtConfig.clk_div                        = 20;
        rmtConfig.mem_block_num                  = 2;
        rmtConfig.tx_config.loop_en              = false;
        rmtConfig.tx_config.carrier_en           = false;
        rmtConfig.tx_config.carrier_freq_hz      = 0;
        rmtConfig.tx_config.carrier_duty_percent = 50;
        rmtConfig.tx_config.carrier_level        = RMT_CARRIER_LEVEL_LOW;
        rmtConfig.tx_config.idle_output_en       = true;

#    ifdef STEP_PULSE_DELAY
        rmtItem[0].duration0 = STEP_PULSE_DELAY * 4;
#    else
        rmtItem[0].duration0 = 1;
#    endif

        rmtItem[0].duration1 = 4 * pulse_microseconds->get();
        rmtItem[1].duration0 = 0;
        rmtItem[1].duration1 = 0;

        _rmt_chan_num = sys_get_next_RMT_chan_num();
        if (_rmt_chan_num == RMT_CHANNEL_MAX) {
            return;
        }
        rmt_set_source_clk(_rmt_chan_num, RMT_BASECLK_APB);
        rmtConfig.channel              = _rmt_chan_num;
        rmtConfig.tx_config.idle_level = _invert_step_pin ? RMT_IDLE_LEVEL_HIGH : RMT_IDLE_LEVEL_LOW;
        rmtConfig.gpio_num             = gpio_num_t(step_pin);  // c is a wacky lang
        rmtItem[0].level0              = rmtConfig.tx_config.idle_level;
        rmtItem[0].level1              = !rmtConfig.tx_config.idle_level;
        rmt_config(&rmtConfig);
        rmt_fill_tx_items(rmtConfig.channel, &rmtItem[0], rmtConfig.mem_block_num, 0);

#else
        pinMode(_step_pin, OUTPUT);

#endif  // USE_RMT_STEPS
        pinMode(_disable_pin, OUTPUT);
    }

    void StandardStepper::config_message() {
        grbl_msg_sendf(CLIENT_SERIAL,
                       MsgLevel::Info,
                       "%s Axis Standard Stepper Step:%s Dir:%s Disable:%s Limits(%0.3f,%0.3f)",
                       axis_name(),
                       pinName(_step_pin).c_str(),
                       pinName(_dir_pin).c_str(),
                       pinName(_disable_pin).c_str(),
                       _position_min,
                       _position_max);
    }

    void StandardStepper::step() {
#ifdef USE_RMT_STEPS
        RMT.conf_ch[rmt_chan_num].conf1.mem_rd_rst = 1;
        RMT.conf_ch[rmt_chan_num].conf1.tx_start   = 1;
#else
        digitalWrite(_step_pin, !_invert_step_pin);
#endif  // USE_RMT_STEPS
    }

    void StandardStepper::unstep() {
#ifndef USE_RMT_STEPS
        digitalWrite(_step_pin, _invert_step_pin);
#endif  // USE_RMT_STEPS
    }

    void StandardStepper::set_direction(bool dir) { digitalWrite(_dir_pin, dir ^ _invert_dir_pin); }

    void StandardStepper::set_disable(bool disable) { digitalWrite(_disable_pin, disable); }
}
