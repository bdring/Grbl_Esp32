/*
  Jog.cpp - Jogging methods
  Part of Grbl

  Copyright (c) 2016 Sungeun K. Jeon for Gnea Research LLC

	2018 -	Bart Dring This file was modifed for use on the ESP32
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

#include "Grbl.h"

// Sets up valid jog motion received from g-code parser, checks for soft-limits, and executes the jog.
Error jog_execute(plan_line_data_t* pl_data, parser_block_t* gc_block) {
    // Initialize planner data struct for jogging motions.
    // NOTE: Spindle and coolant are allowed to fully function with overrides during a jog.
    pl_data->feed_rate             = gc_block->values.f;
    pl_data->motion.noFeedOverride = 1;
#ifdef USE_LINE_NUMBERS
    pl_data->line_number = gc_block->values.n;
#endif
    if (soft_limits->get()) {
        if (limitsCheckTravel(gc_block->values.xyz)) {
            return Error::TravelExceeded;
        }
    }
// Valid jog command. Plan, set state, and execute.
#ifndef USE_KINEMATICS
    mc_line(gc_block->values.xyz, pl_data);
#else  // else use kinematics
    inverse_kinematics(gc_block->values.xyz, pl_data, gc_state.position);
#endif

    if (sys.state == State::Idle) {
        if (plan_get_current_block() != NULL) {  // Check if there is a block to execute.
            sys.state = State::Jog;
            st_prep_buffer();
            st_wake_up();  // NOTE: Manual start. No state machine required.
        }
    }
    return Error::Ok;
}
