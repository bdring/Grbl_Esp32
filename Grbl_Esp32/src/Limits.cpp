/*
  Limits.cpp - code pertaining to limit-switches and performing the homing cycle
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

	2018 -	Bart Dring This file was modifed for use on the ESP32
					CPU. Do not use this with Grbl for atMega328P
  2018-12-29 - Wolfgang Lienbacher renamed file from limits.h to grbl_limits.h
          fixing ambiguation issues with limit.h in the esp32 Arduino Framework
          when compiling with VS-Code/PlatformIO.

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

#include "Limits.h"

#include "Machine/MachineConfig.h"
#include "Planner.h"
#include "MotionControl.h"  // HOMING_CYCLE_LINE_NUMBER
#include "NutsBolts.h"      // bitnum_true, etc
#include "System.h"         // sys.*
#include "Stepper.h"        // st_wake
#include "Report.h"         // CLIENT_
#include "Protocol.h"       // protocol_execute_realtime
#include "I2SOut.h"         // I2S_OUT_DELAY_MS
#include "Platform.h"
#include "Machine/Axes.h"

#include <freertos/task.h>
#include <freertos/queue.h>
#include <Arduino.h>  // IRAM_ATTR
#include <string.h>   // memset, memcpy
#include <algorithm>  // min, max
#include <atomic>     // fence

xQueueHandle limit_sw_queue;  // used by limit switch debouncing

// Calculate the motion for the next homing move.
//  Input: axesMask - the axes that should participate in this homing cycle
//  Input: approach - the direction of motion - true for approach, false for pulloff
//  Input: seek - the phase - true for the initial high-speed seek, false for the slow second phase
//  Output: axislock - the axes that actually participate, accounting
//  Output: target - the endpoint vector of the motion
//  Output: rate    - the feed rate
//  Return: debounce - the maximum delay time of all the axes

// For multi-axis homing, we use the per-axis rates and travel limits to compute
// a target vector and a feedrate as follows:
// The goal is for each axis to travel at its specified rate, and for the
// maximum travel to be enough for each participating axis to reach its limit.
// For the rate goal, the axis components of the target vector must be proportional
// to the per-axis rates, and the overall feed rate must be the magnitude of the
// vector of per-axis rates.
// For the travel goal, the axis components of the target vector must be scaled
// according to the one that would take the longest.
// The time to complete a maxTravel move for a given feedRate is maxTravel/feedRate.
// We compute that time for all axes in the homing cycle, then find the longest one.
// Then we scale the travel distances for the other axes so they would complete
// at the same time.

static uint32_t limits_plan_move(AxisMask axesMask, bool approach, bool seek) {
    float    maxSeekTime  = 0.0;
    float    limitingRate = 0.0;
    uint32_t debounce     = 0;
    float    rate         = 0.0;

    auto   axes   = config->_axes;
    auto   n_axis = axes->_numberAxis;
    float* target = system_get_mpos();

    // Find the axis that will take the longest
    for (int axis = 0; axis < n_axis; axis++) {
        if (bitnum_isfalse(axesMask, axis)) {
            continue;
        }
        // Set target location for active axes and setup computation for homing rate.
        sys_position[axis] = 0;

        auto axisConfig = axes->_axis[axis];
        auto homing     = axisConfig->_homing;

        debounce = std::max(debounce, homing->_debounce_ms);

        float axis_rate = seek ? homing->_seekRate : homing->_feedRate;

        // Accumulate the squares of the homing rates for later use
        // in computing the aggregate feed rate.
        rate += (axis_rate * axis_rate);

        auto travel = seek ? axisConfig->_maxTravel : homing->_pulloff;

        // First we compute the maximum-time-to-completion vector; later we will
        // convert it back to positions after we determine the limiting axis.
        // Set target direction based on cycle mask and homing cycle approach state.
        auto seekTime = travel / axis_rate;

        target[axis] = (homing->_positiveDirection ^ approach) ? -seekTime : seekTime;
        if (seekTime > maxSeekTime) {
            maxSeekTime  = seekTime;
            limitingRate = axis_rate;
        }
    }
    // Scale the target array, currently in units of time, back to positions
    // When approaching a small fudge factor to ensure that the limit is reached -
    // but no fudge factor when pulling off.
    for (int axis = 0; axis < n_axis; axis++) {
        if (bitnum_istrue(axesMask, axis)) {
            auto homing = config->_axes->_axis[axis]->_homing;
            auto scaler = approach ? (seek ? homing->_seek_scaler : homing->_feed_scaler) : 1.0;
            target[axis] *= limitingRate * scaler;
        }
    }

    plan_line_data_t plan_data;
    plan_data.spindle_speed         = 0;
    plan_data.motion                = {};
    plan_data.motion.systemMotion   = 1;
    plan_data.motion.noFeedOverride = 1;
    plan_data.spindle               = SpindleState::Disable;
    plan_data.coolant.Mist          = 0;
    plan_data.coolant.Flood         = 0;
    plan_data.line_number           = HOMING_CYCLE_LINE_NUMBER;
    plan_data.is_jog                = false;

    plan_data.feed_rate = float(sqrt(rate));  // Magnitude of homing rate vector
    plan_buffer_line(target, &plan_data);     // Bypass mc_line(). Directly plan homing motion.

    sys.step_control                  = {};
    sys.step_control.executeSysMotion = true;  // Set to execute homing motion and clear existing flags.
    Stepper::prep_buffer();                    // Prep and fill segment buffer from newly planned block.
    Stepper::wake_up();                        // Initiate motion

    return debounce;
}

// Returns true if an error occurred
static ExecAlarm limits_handle_errors(bool approach, uint8_t cycle_mask) {
    // This checks some of the events that would normally be handled
    // by protocol_execute_realtime().  The homing loop is time-critical
    // so we handle those events directly here, calling protocol_execute_realtime()
    // only if one of those events is active.
    if (rtReset) {
        // Homing failure: Reset issued during cycle.
        return ExecAlarm::HomingFailReset;
    }
    if (rtSafetyDoor) {
        // Homing failure: Safety door was opened.
        return ExecAlarm::HomingFailDoor;
    }
    if (rtCycleStop) {
        if (approach) {
            // Homing failure: Limit switch not found during approach.
            return ExecAlarm::HomingFailApproach;
        }
        // Pulloff
        if (limits_check(cycle_mask)) {
            // Homing failure: Limit switch still engaged after pull-off motion
            return ExecAlarm::HomingFailPulloff;
        }
    }
    // If we get here, either none of the rtX events were triggered, or
    // it was rtCycleStop during a pulloff with the limit switches
    // disengaged, i.e. a normal pulloff completion.
    return ExecAlarm::None;
}

// Homes the specified cycle axes, sets the machine position, and performs a pull-off motion after
// completing. Homing is a special motion case, which involves rapid uncontrolled stops to locate
// the trigger point of the limit switches. The rapid stops are handled by a system level axis lock
// mask, which prevents the stepper algorithm from executing step pulses. Homing motions typically
// circumvent the processes for executing motions in normal operation.
// NOTE: Only the abort realtime command can interrupt this process.
static void limits_go_home(uint8_t cycle_mask, uint32_t n_locate_cycles) {
    if (sys.abort) {
        return;  // Block if system reset has been issued.
    }

    auto axes   = config->_axes;
    auto n_axis = axes->_numberAxis;

    cycle_mask &= Machine::Axes::homingMask;

    // Initialize plan data struct for homing motion. Spindle and coolant are disabled.

    // Put motors on axes listed in cycle_mask in homing mode and
    // replace cycle_mask with the list of motors that are ready for homing.
    // Motors with non standard homing can home during motors_set_homing_mode(...)
    cycle_mask = config->_axes->set_homing_mode(cycle_mask, true);  // tell motors homing is about to start

    // See if any motors are left.  This could be 0 if none of the motors specified
    // by the original value of cycle_mask is capable of standard homing.
    if (cycle_mask == 0) {
        return;
    }

    // Initialize variables used for homing computations.
    uint8_t n_cycle = (2 * n_locate_cycles + 1);

    // approach is the direction of motion; it cycles between true and false
    bool approach = true;

    // seek starts out true for initial rapid motion toward the homing switches,
    // then becomes false after the first approach cycle, for slower motion on
    // subsequent fine-positioning steps
    bool seek = true;

    do {
        uint32_t debounce_ms = limits_plan_move(cycle_mask, approach, seek);

        // Perform homing cycle. Planner buffer should be empty, as required to initiate the homing cycle.

        // Start with all motors allowed to move
        config->_axes->release_all_motors();

        // For approach cycles:
        // remainingMotors starts out with the bits set for all the motors in this homing cycle.
        // As limit switches are hit, their bits are cleared and the associated motor is stopped,
        // continuing until no bits are set (normal exit)

        // For pulloff cycles:
        // Motion continues until rtCycleStop is set, indicating that the target was reached,
        // without looking at the limit switches (which are initially active)

        // There are also some error conditions that can abort the operation:
        // rtReset or rtSafetyDoor - the user hits either of those buttons
        // rtCycleStop in approach
        //   - the max travel distance was reached without hitting all the limit switches
        // rtCycleStop in pulloff but a switch is still active
        //   - pulloff failed to clear all the switches

        // XXX we need to include gang1 in the remaining mask
        // The following might fail if only one gang has limit switches. Anaylze me.
        uint32_t remainingMotors = (cycle_mask | (cycle_mask << 16)) & Machine::Axes::motorMask;

        uint32_t old = 0;

        do {
            if (approach) {
                // Check limit state. Lock out cycle axes when they change.
                // XXX do we check only the switch in the direction of motion?
                uint32_t limitedAxes = Machine::Axes::posLimitMask | Machine::Axes::negLimitMask;

                if (old != remainingMotors) {
                    //log_info("remainingMotors: " << remainingMotors);
                    log_info("remainingMotors: 0x" << String(remainingMotors, 16));
                    log_info("limitedAxes: 0x" << String(limitedAxes, 16));
                    old = remainingMotors;
                }

                config->_axes->stop_motors(limitedAxes);
                bit_false(remainingMotors, limitedAxes);
            }
            Stepper::prep_buffer();  // Check and prep segment buffer.

            ExecAlarm alarm = limits_handle_errors(approach, cycle_mask);
            if (alarm != ExecAlarm::None) {
                // Homing failure
                sys_rt_exec_alarm = alarm;
                config->_axes->set_homing_mode(cycle_mask, false);  // tell motors homing is done...failed
                log_debug("Homing fail");
                mc_reset();  // Stop motors, if they are running.
                // protocol_execute_realtime() will handle any pending rtXXX conditions
                protocol_execute_realtime();
                return;
            }

            if (rtCycleStop) {
                // Normal pulloff completion with limit switches disengaged
                rtCycleStop = false;
                break;
            }
            // Keep trying until all axes have finished
        } while (remainingMotors);

        if (!approach) {
            config->_stepping->synchronize();
        }

        Stepper::reset();       // Immediately force kill steppers and reset step segment buffer.
        delay_ms(debounce_ms);  // Delay to allow transient dynamics to dissipate.

        // After the initial approach, we switch to the slow rate for subsequent steps
        // The pattern is  fast approach, slow pulloff, slow approach, slow pulloff, ...
        seek = false;

        // Reverse direction.
        approach = !approach;

    } while (n_cycle-- > 0);

    // The active cycle axes should now be homed and machine limits have been located. By
    // default, Grbl defines machine space as all negative, as do most CNCs. Since limit switches
    // can be on either side of an axes, check and set axes machine zero appropriately. Also,
    // set up pull-off maneuver from axes limit switches that have been homed. This provides
    // some initial clearance off the switches and should also help prevent them from falsely
    // triggering when hard limits are enabled or when more than one axes shares a limit pin.

    // Set machine positions for homed limit switches. Don't update non-homed axes.
    for (int axis = 0; axis < n_axis; axis++) {
        Machine::Axis* axisConf = config->_axes->_axis[axis];
        auto           homing   = axisConf->_homing;
        if (bitnum_istrue(cycle_mask, axis)) {
            auto mpos    = homing->_mpos;
            auto pulloff = homing->_pulloff;
            auto steps   = axisConf->_stepsPerMm;
            if (homing->_positiveDirection) {
                sys_position[axis] = int32_t((mpos + pulloff) * steps);
            } else {
                sys_position[axis] = int32_t((mpos - pulloff) * steps);
            }
        }
    }
    sys.step_control = {};                              // Return step control to normal operation.
    config->_axes->set_homing_mode(cycle_mask, false);  // tell motors homing is done
}

// return true if the mask has exactly one bit set,
// so it refers to exactly one axis
static bool mask_is_single_axis(AxisMask axis_mask) {
    // This code depends on the fact that, for binary numberes
    // with only one bit set - and only for such numbers -
    // the bits in one less than the number are disjoint
    // with that bit.  For a number like B100, if you
    // subtract one, the low order 00 bits will have to
    // borrow from the high 1 bit and thus clear it.
    // If any lower bits are 1, then there will be no
    // borrow to clear the highest 1 bit.
    return axis_mask && ((axis_mask & (axis_mask - 1)) == 0);
}

static AxisMask squaredAxes() {
    AxisMask mask   = 0;
    auto     axes   = config->_axes;
    auto     n_axis = axes->_numberAxis;
    for (int axis = 0; axis < n_axis; axis++) {
        auto homing = axes->_axis[axis]->_homing;
        if (homing && homing->_square) {
            bitnum_true(mask, axis);
        }
    }
    return mask;
}

static bool axis_is_squared(AxisMask axis_mask) {
    // Squaring can only be done if it is the only axis in the mask

    // cases:
    // axis_mask has one bit:
    //   axis is squared: return true
    //   else: return false
    // else:
    //   one of the axes is squared: message and return false
    //   else return false

    if (axis_mask & squaredAxes()) {
        if (mask_is_single_axis(axis_mask)) {
            return true;
        }
        log_info("Cannot multi-axis home with squared axes. Homing normally");
    }

    return false;
}

// For this routine, homing_mask cannot be 0.  The 0 case,
// meaning run all cycles, is handled by the caller mc_homing_cycle()
static void limits_run_one_homing_cycle(AxisMask homing_mask) {
    if (axis_is_squared(homing_mask)) {
        // For squaring, we first do the fast seek using both motors,
        // skipping the second slow moving phase.
        ganged_mode = gangDual;
        limits_go_home(homing_mask, 0);  // Do not do a second touch cycle

        // Then we do the slow motion on the individual motors
        ganged_mode = gangA;
        limits_go_home(homing_mask, NHomingLocateCycle);

        ganged_mode = gangB;
        limits_go_home(homing_mask, NHomingLocateCycle);

        ganged_mode = gangDual;  // always return to dual
    } else {
        limits_go_home(homing_mask, NHomingLocateCycle);
    }
}

void limits_run_homing_cycles(AxisMask axis_mask) {
    // -------------------------------------------------------------------------------------
    // Perform homing routine. NOTE: Special motion case. Only system reset works.
    if (axis_mask != HOMING_CYCLE_ALL) {
        limits_run_one_homing_cycle(axis_mask);
    } else {
        // Run all homing cycles
        bool someAxisHomed = false;

        for (int cycle = 1; cycle <= MAX_N_AXIS; cycle++) {
            // Set axis_mask to the axes that home on this cycle
            axis_mask   = 0;
            auto n_axis = config->_axes->_numberAxis;
            for (int axis = 0; axis < n_axis; axis++) {
                auto axisConfig = config->_axes->_axis[axis];
                auto homing     = axisConfig->_homing;
                if (homing && homing->_cycle == cycle) {
                    bitnum_true(axis_mask, axis);
                }
            }

            if (axis_mask) {  // if there are some axes in this cycle
                someAxisHomed = true;
                limits_run_one_homing_cycle(axis_mask);
            }
        }
        if (!someAxisHomed) {
            report_status_message(Error::HomingNoCycles, CLIENT_ALL);
            sys.state = State::Alarm;
        }
    }
}

void limits_init() {
    if (Machine::Axes::limitMask) {
        if (limit_sw_queue == NULL && config->_softwareDebounceMs != 0) {
            // setup task used for debouncing
            if (limit_sw_queue == NULL) {
                limit_sw_queue = xQueueCreate(10, sizeof(int));
                xTaskCreate(limitCheckTask,
                            "limitCheckTask",
                            2048,
                            NULL,
                            5,  // priority
                            NULL);
            }
        }
    }
}

// Check the limit switches for the axes listed in check_mask.
// Return a mask of the switches that are engaged.
AxisMask limits_check(AxisMask check_mask) {
    // Expand the bitmask to include both gangs
    bit_true(check_mask, check_mask << 16);
    return (Machine::Axes::posLimitMask | Machine::Axes::negLimitMask) & check_mask;
}

// Returns limit state as a bit-wise uint8 variable. Each bit indicates an axis limit, where
// triggered is 1 and not triggered is 0. Invert mask is applied. Axes are defined by their
// number in bit position, i.e. Z_AXIS is bit(2), and Y_AXIS is bit(1).
AxisMask limits_get_state() {
    return limits_check(Machine::Axes::limitMask);
}

// Performs a soft limit check. Called from mcline() only. Assumes the machine has been homed,
// the workspace volume is in all negative space, and the system is in normal operation.
// NOTE: Used by jogging to limit travel within soft-limit volume.
void limits_soft_check(float* target) {
    if (limitsCheckTravel(target)) {
        sys.soft_limit = true;
        // Force feed hold if cycle is active. All buffered blocks are guaranteed to be within
        // workspace volume so just come to a controlled stop so position is not lost. When complete
        // enter alarm mode.
        if (sys.state == State::Cycle) {
            rtFeedHold = true;
            do {
                protocol_execute_realtime();
                if (sys.abort) {
                    return;
                }
            } while (sys.state != State::Idle);
        }
        log_debug("Soft limits");
        mc_reset();                                // Issue system reset and ensure spindle and coolant are shutdown.
        sys_rt_exec_alarm = ExecAlarm::SoftLimit;  // Indicate soft limit critical event
        protocol_execute_realtime();               // Execute to enter critical event loop and system abort
        return;
    }
}

void limitCheckTask(void* pvParameters) {
    while (true) {
        std::atomic_thread_fence(std::memory_order::memory_order_seq_cst);  // read fence for settings

        int evt;
        xQueueReceive(limit_sw_queue, &evt, portMAX_DELAY);            // block until receive queue
        vTaskDelay(config->_softwareDebounceMs / portTICK_PERIOD_MS);  // delay a while
        AxisMask switch_state;
        switch_state = limits_get_state();
        if (switch_state) {
            log_debug("Limit Switch State " << String(switch_state, HEX));
            mc_reset();                                // Initiate system kill.
            sys_rt_exec_alarm = ExecAlarm::HardLimit;  // Indicate hard limit critical event
        }
        static UBaseType_t uxHighWaterMark = 0;
#ifdef DEBUG_TASK_STACK
        reportTaskStackSize(uxHighWaterMark);
#endif
    }
}

float limitsMaxPosition(uint8_t axis) {
    auto  axisConfig = config->_axes->_axis[axis];
    auto  homing     = axisConfig->_homing;
    float mpos       = (homing != nullptr) ? homing->_mpos : 0;
    auto  maxtravel  = axisConfig->_maxTravel;

    return (homing == nullptr || homing->_positiveDirection) ? mpos + maxtravel : mpos;
}

float limitsMinPosition(uint8_t axis) {
    auto  axisConfig = config->_axes->_axis[axis];
    auto  homing     = axisConfig->_homing;
    float mpos       = (homing != nullptr) ? homing->_mpos : 0;
    auto  maxtravel  = axisConfig->_maxTravel;

    return (homing == nullptr || homing->_positiveDirection) ? mpos : mpos - maxtravel;
}

// Checks and reports if target array exceeds machine travel limits.
// Return true if exceeding limits
// Set $<axis>/MaxTravel=0 to selectively remove an axis from soft limit checks
bool WEAK_LINK limitsCheckTravel(float* target) {
    auto axes   = config->_axes;
    auto n_axis = axes->_numberAxis;
    for (int axis = 0; axis < n_axis; axis++) {
        auto axisSetting = axes->_axis[axis];
        if ((target[axis] < limitsMinPosition(axis) || target[axis] > limitsMaxPosition(axis)) && axisSetting->_maxTravel > 0) {
            return true;
        }
    }
    return false;
}

bool WEAK_LINK user_defined_homing(AxisMask cycle_mask) {
    return false;
}
