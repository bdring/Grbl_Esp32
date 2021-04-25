#pragma once

/*
  MotionControl.h - high level interface for issuing motion commands
  Part of Grbl

  Copyright (c) 2011-2015 Sungeun K. Jeon
  Copyright (c) 2009-2011 Simen Svale Skogsrud

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

// System motion commands must have a line number of zero.
const int HOMING_CYCLE_LINE_NUMBER   = 0;
const int PARKING_MOTION_LINE_NUMBER = 0;

#define HOMING_CYCLE_ALL 0  // Must be zero.

// Execute linear motion in absolute millimeter coordinates. Feed rate given in millimeters/second
// unless invert_feed_rate is true. Then the feed_rate means that the motion should be completed in
// (1 minute)/feed_rate time.
bool cartesian_to_motors(float* target, plan_line_data_t* pl_data, float* position);
bool mc_line(float* target, plan_line_data_t* pl_data);  // returns true if line was submitted to planner

// Execute an arc in offset mode format. position == current xyz, target == target xyz,
// offset == offset from current xyz, axis_XXX defines circle plane in tool space, axis_linear is
// the direction of helical travel, radius == circle radius, is_clockwise_arc boolean. Used
// for vector transformation direction.
void mc_arc(float*            target,
            plan_line_data_t* pl_data,
            float*            position,
            float*            offset,
            float             radius,
            uint8_t           axis_0,
            uint8_t           axis_1,
            uint8_t           axis_linear,
            uint8_t           is_clockwise_arc);

// Dwell for a specific number of seconds
bool mc_dwell(int32_t milliseconds);

// Perform homing cycle to locate machine zero. Requires limit switches.
void mc_homing_cycle(uint8_t cycle_mask);

// Perform tool length probe cycle. Requires probe switch.
GCUpdatePos mc_probe_cycle(float* target, plan_line_data_t* pl_data, uint8_t parser_flags);

// Handles updating the override control state.
void mc_override_ctrl_update(uint8_t override_state);

// Plans and executes the single special motion case for parking. Independent of main planner buffer.
void mc_parking_motion(float* parking_target, plan_line_data_t* pl_data);

// Performs system reset. If in motion state, kills all motion and sets system alarm.
void mc_reset();

enum class SquaringMode : uint8_t {
    Dual,  // both motors run
    A,     // A motor runs
    B,     // B motor runs
};

extern SquaringMode ganged_mode;
