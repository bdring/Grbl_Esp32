/*
  CoreXY.cpp - 
  
  Copyright (c) 2020    Barton Dring @buildlog

  https://corexy.com/theory.html

  Limitations 
  - Must home via $H. $HX type homes not allowed
  - Must home one axis per cycle
  - limited to 3 axis systems...easy fix in increase (just donate)

  ============================================================================

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
    FYI: http://forums.trossenrobotics.com/tutorials/introduction-129/delta-robot-kinematics-3276/
    Better: http://hypertriangle.com/~alex/delta-robot-tutorial/
*/
#include "../src/Settings.h"

// Homing axis search distance multiplier. Computed by this value times the cycle travel.
#ifndef HOMING_AXIS_SEARCH_SCALAR
#    define HOMING_AXIS_SEARCH_SCALAR 1.1  // Must be > 1 to ensure limit switch will be engaged.
#endif
#ifndef HOMING_AXIS_LOCATE_SCALAR
#    define HOMING_AXIS_LOCATE_SCALAR 2.0  // Must be > 1 to ensure limit switch is cleared.
#endif

// The midTbot has a quirk where the x motor has to move twice as far as it would
// on a normal T-Bot or CoreXY
#ifndef MIDTBOT
const float geometry_factor = 1.0;
#else
const float geometry_factor = 2.0;
#endif

static float last_motors[MAX_N_AXIS]    = { 0.0 };  // A place to save the previous motor angles for distance/feed rate calcs
static float last_cartesian[MAX_N_AXIS] = {};

// prototypes for helper functions
float three_axis_dist(float* point1, float* point2);

void machine_init() {
    // print a startup message to show the kinematics are enable

#ifdef MIDTBOT
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "CoreXY (midTbot) Kinematics Init");
#else
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "CoreXY Kinematics Init");
#endif
}

// Cycle mask is 0 unless the user sends a single axis command like $HZ
// This will always return true to prevent the normal Grbl homing cycle
bool user_defined_homing(uint8_t cycle_mask) {
    uint8_t n_cycle;                       // each home is a multi cycle operation approach, pulloff, approach.....
    float   target[MAX_N_AXIS] = { 0.0 };  // The target for each move in the cycle
    float   max_travel;
    uint8_t axis;

    // check for multi axis homing per cycle ($Homing/Cycle0=XY type)...not allowed in CoreXY
    bool setting_error = false;
    auto n_axis        = number_axis->get();
    for (int cycle = 0; cycle < n_axis; cycle++) {
        if (numberOfSetBits(homing_cycle[cycle]->get()) > 1) {
            grbl_msg_sendf(CLIENT_SERIAL,
                           MsgLevel::Info,
                           "CoreXY Multi axis homing cycles not allowed. $Homing/Cycle%d=%s",
                           cycle,
                           homing_cycle[cycle]->getStringValue());
            setting_error = true;
        }
    }
    if (setting_error)
        return true;

    // setup the motion parameters
    plan_line_data_t  plan_data;
    plan_line_data_t* pl_data = &plan_data;
    memset(pl_data, 0, sizeof(plan_line_data_t));
    pl_data->motion                = {};
    pl_data->motion.systemMotion   = 1;
    pl_data->motion.noFeedOverride = 1;

    uint8_t cycle_count = (cycle_mask == 0) ? n_axis : 1;  // if we have a cycle_mask, we are only going to do one axis

    AxisMask mask = 0;
    for (int cycle = 0; cycle < cycle_count; cycle++) {
        // if we have a cycle_mask, do that. Otherwise get the cycle from the settings
        mask = cycle_mask ? cycle_mask : homing_cycle[cycle]->get();

        // If not X or Y do a normal home
        if (!(bitnum_istrue(mask, X_AXIS) || bitnum_istrue(mask, Y_AXIS))) {
            limits_go_home(mask);  // Homing cycle 0
            continue;              // continue to next item in for loop
        }

        mask = motors_set_homing_mode(mask, true);  // non standard homing motors will do their own thing and get removed from the mask

        for (uint8_t axis = X_AXIS; axis <= n_axis; axis++) {
            if (bit(axis) == mask) {
                // setup for the homing of this axis
                bool  approach       = true;
                float homing_rate    = homing_seek_rate->get();
                max_travel           = HOMING_AXIS_SEARCH_SCALAR * axis_settings[axis]->max_travel->get();
                sys.homing_axis_lock = 0xFF;                          // we don't need to lock any motors in CoreXY
                n_cycle              = (2 * NHomingLocateCycle + 1);  // approach + ((pulloff + approach) * Cycles)

                do {
                    bool switch_touched = false;

                    // zero all X&Y posiitons before each cycle
                    for (int idx = X_AXIS; idx <= Y_AXIS; idx++) {
                        sys_position[idx] = 0.0;
                        target[idx]       = 0.0;
                    }

                    if (bit_istrue(homing_dir_mask->get(), bit(axis))) {
                        approach ? target[axis] = -max_travel : target[axis] = max_travel;
                    } else {
                        approach ? target[axis] = max_travel : target[axis] = -max_travel;
                    }

                    for (int axis = Z_AXIS; axis < n_axis; axis++) {
                        target[axis] = system_convert_axis_steps_to_mpos(sys_position, axis);
                    }

                    // convert back to motor steps
                    inverse_kinematics(target);

                    pl_data->feed_rate = homing_rate;   // feed or seek rates
                    plan_buffer_line(target, pl_data);  // Bypass mc_line(). Directly plan homing motion.
                    sys.step_control                  = {};
                    sys.step_control.executeSysMotion = true;  // Set to execute homing motion and clear existing flags.
                    st_prep_buffer();                          // Prep and fill segment buffer from newly planned block.
                    st_wake_up();                              // Initiate motion

                    do {
                        if (approach) {
                            switch_touched = bitnum_istrue(limits_get_state(), axis);
                        }
                        st_prep_buffer();  // Check and prep segment buffer. NOTE: Should take no longer than 200us.
                        // Exit routines: No time to run protocol_execute_realtime() in this loop.
                        if (sys_rt_exec_state.bit.safetyDoor || sys_rt_exec_state.bit.reset || cycle_stop) {
                            ExecState rt_exec_state;
                            rt_exec_state.value = sys_rt_exec_state.value;
                            // Homing failure condition: Reset issued during cycle.
                            if (rt_exec_state.bit.reset) {
                                sys_rt_exec_alarm = ExecAlarm::HomingFailReset;
                            }
                            // Homing failure condition: Safety door was opened.
                            if (rt_exec_state.bit.safetyDoor) {
                                sys_rt_exec_alarm = ExecAlarm::HomingFailDoor;
                            }
                            // Homing failure condition: Limit switch still engaged after pull-off motion
                            if (!approach && (limits_get_state() & cycle_mask)) {
                                sys_rt_exec_alarm = ExecAlarm::HomingFailPulloff;
                            }
                            // Homing failure condition: Limit switch not found during approach.
                            if (approach && cycle_stop) {
                                sys_rt_exec_alarm = ExecAlarm::HomingFailApproach;
                            }

                            if (sys_rt_exec_alarm != ExecAlarm::None) {
                                motors_set_homing_mode(cycle_mask, false);  // tell motors homing is done...failed
                                mc_reset();                                 // Stop motors, if they are running.
                                protocol_execute_realtime();
                                return true;
                            } else {
                                // Pull-off motion complete. Disable CYCLE_STOP from executing.
                                cycle_stop = false;
                                break;
                            }
                        }
                    } while (!switch_touched);

#ifdef USE_I2S_STEPS
                    if (current_stepper == ST_I2S_STREAM) {
                        if (!approach) {
                            delay_ms(I2S_OUT_DELAY_MS);
                        }
                    }
#endif
                    st_reset();                        // Immediately force kill steppers and reset step segment buffer.
                    delay_ms(homing_debounce->get());  // Delay to allow transient dynamics to dissipate.

                    approach = !approach;
                    // After first cycle, homing enters locating phase. Shorten search to pull-off distance.
                    if (approach) {
                        max_travel  = homing_pulloff->get() * HOMING_AXIS_LOCATE_SCALAR;
                        homing_rate = homing_feed_rate->get();
                    } else {
                        max_travel  = homing_pulloff->get();
                        homing_rate = homing_seek_rate->get();
                    }
                } while (n_cycle-- > 0);
            }
        }
    }  // for

    // after sussefully setting X & Y axes, we set the current positions

    // set the cartesian axis position
    for (axis = X_AXIS; axis <= Y_AXIS; axis++) {
        if (bitnum_istrue(homing_dir_mask->get(), axis)) {
            target[axis] = limitsMinPosition(axis) + homing_pulloff->get();
        } else {
            target[axis] = limitsMaxPosition(axis) - homing_pulloff->get();
        }
    }

    last_cartesian[X_AXIS] = target[X_AXIS];
    last_cartesian[Y_AXIS] = target[Y_AXIS];

    for (int axis = Z_AXIS; axis < n_axis; axis++) {
        last_cartesian[axis] = system_convert_axis_steps_to_mpos(sys_position, axis);
    }

    // convert to motors
    inverse_kinematics(target);
    // convert to steps
    for (axis = X_AXIS; axis <= Y_AXIS; axis++) {
        sys_position[axis] = target[axis] * axis_settings[axis]->steps_per_mm->get();
    }

    sys.step_control = {};  // Return step control to normal operation.

    gc_sync_position();
    plan_sync_position();
    kinematics_post_homing();
    limits_init();

    return true;
}

// This function is used by Grbl convert Cartesian to motors
// this does not do any motion control
void inverse_kinematics(float* position) {
    float motors[MAX_N_AXIS];

    motors[X_AXIS] = geometry_factor * position[X_AXIS] + position[Y_AXIS];
    motors[Y_AXIS] = geometry_factor * position[X_AXIS] - position[Y_AXIS];

    position[X_AXIS] = motors[X_AXIS];
    position[Y_AXIS] = motors[Y_AXIS];

    // Z and higher just pass through unchanged
}

// Inverse Kinematics calculates motor positions from real world cartesian positions
// position is the current position
// Breaking into segments is not needed with CoreXY, because it is a linear system.
void inverse_kinematics(float* target, plan_line_data_t* pl_data, float* position)  //The target and position are provided in MPos
{
    float dx, dy, dz;  // distances in each cartesian axis
    float motors[MAX_N_AXIS];

    float feed_rate = pl_data->feed_rate;  // save original feed rate

    // calculate cartesian move distance for each axis
    dx         = target[X_AXIS] - position[X_AXIS];
    dy         = target[Y_AXIS] - position[Y_AXIS];
    dz         = target[Z_AXIS] - position[Z_AXIS];
    float dist = sqrt((dx * dx) + (dy * dy) + (dz * dz));

    motors[X_AXIS] = geometry_factor * target[X_AXIS] + target[Y_AXIS];
    motors[Y_AXIS] = geometry_factor * target[X_AXIS] - target[Y_AXIS];

    auto n_axis = number_axis->get();
    for (uint8_t axis = Z_AXIS; axis <= n_axis; axis++) {
        motors[axis] = target[axis];
    }

    float motor_distance = three_axis_dist(motors, last_motors);

    if (!pl_data->motion.rapidMotion) {
        pl_data->feed_rate *= (motor_distance / dist);
    }

    memcpy(last_motors, motors, sizeof(motors));

    mc_line(motors, pl_data);
}

// motors -> cartesian
void forward_kinematics(float* position) {
    float   calc_fwd[MAX_N_AXIS];
    float   wco[MAX_N_AXIS];
    float   print_position[N_AXIS];
    int32_t current_position[N_AXIS];  // Copy current state of the system position variable

    memcpy(current_position, sys_position, sizeof(sys_position));
    system_convert_array_steps_to_mpos(print_position, current_position);

    // determine the Work Coordinate Offsets for each axis
    auto n_axis = number_axis->get();
    for (int axis = 0; axis < n_axis; axis++) {
        // Apply work coordinate offsets and tool length offset to current position.
        wco[axis] = gc_state.coord_system[axis] + gc_state.coord_offset[axis];
        if (axis == TOOL_LENGTH_OFFSET_AXIS) {
            wco[axis] += gc_state.tool_length_offset;
        }
    }

    // apply the forward kinemetics to the machine coordinates
    // https://corexy.com/theory.html
    //calc_fwd[X_AXIS] = 0.5 / geometry_factor * (position[X_AXIS] + position[Y_AXIS]);
    calc_fwd[X_AXIS] = ((0.5 * (print_position[X_AXIS] + print_position[Y_AXIS]) / geometry_factor) - wco[X_AXIS]);
    calc_fwd[Y_AXIS] = ((0.5 * (print_position[X_AXIS] - print_position[Y_AXIS])) - wco[Y_AXIS]);

    for (int axis = 0; axis < n_axis; axis++) {
        if (axis > Y_AXIS) {  // for axes above Y there is no kinematics
            calc_fwd[axis] = print_position[axis] - wco[axis];
        }
        position[axis] = calc_fwd[axis];  // this is the value returned to reporting
    }
}

bool kinematics_pre_homing(uint8_t cycle_mask) {
    return false;
}

void kinematics_post_homing() {
    auto n_axis = number_axis->get();
    for (uint8_t axis = X_AXIS; axis <= n_axis; axis++) {
        gc_state.position[axis] = last_cartesian[axis];
    }
}

// this is used used by Grbl soft limits to see if the range of the machine is exceeded.
uint8_t kinematic_limits_check(float* target) {
    return true;
}

void user_m30() {}

// ================ Local Helper functions =================

// Determine the unit distance between (2) 3D points
float three_axis_dist(float* point1, float* point2) {
    return sqrt(((point1[0] - point2[0]) * (point1[0] - point2[0])) + ((point1[1] - point2[1]) * (point1[1] - point2[1])) +
                ((point1[2] - point2[2]) * (point1[2] - point2[2])));
}
