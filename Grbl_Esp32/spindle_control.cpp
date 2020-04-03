/*
  spindle_control.cpp - Header for system level commands and real-time processes
  Part of Grbl
  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC

	2018 -	Bart Dring This file was modified for use on the ESP32
					CPU. Do not use this with Grbl for atMega328P

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

#include "grbl.h"
#include "tools/SpindleClass.h"
#include "tools/SpindleClass.cpp"

int8_t spindle_pwm_chan_num;

// define a spindle type
//RelaySpindle my_spindle;
PWMSpindle my_spindle;

void spindle_init() {
    my_spindle.init();
    my_spindle.config_message();
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Spindle is rate adjustable %d", my_spindle.isRateAdjusted());
}

void spindle_stop() {
    my_spindle.stop();
}

uint8_t spindle_get_state() {
    return my_spindle.get_state();
}



// Called by spindle_set_state() and step segment generator. Keep routine small and efficient.
void spindle_set_state(uint8_t state, float rpm) {
    my_spindle.set_state(state, rpm);
}


void spindle_sync(uint8_t state, float rpm) {
    if (sys.state == STATE_CHECK_MODE)
        return;
    protocol_buffer_synchronize(); // Empty planner buffer to ensure spindle is set when programmed.
    spindle_set_state(state, rpm);
}


