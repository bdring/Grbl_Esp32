/*
  Stepper.cpp - stepper motor driver: executes motion plans using stepper motors
  Part of Grbl

  Copyright (c) 2011-2016 Sungeun K. Jeon for Gnea Research LLC
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

#include <atomic>

// Stores the planner block Bresenham algorithm execution data for the segments in the segment
// buffer. Normally, this buffer is partially in-use, but, for the worst case scenario, it will
// never exceed the number of accessible stepper buffer segments (SEGMENT_BUFFER_SIZE-1).
// NOTE: This data is copied from the prepped planner blocks so that the planner blocks may be
// discarded when entirely consumed and completed by the segment buffer. Also, AMASS alters this
// data for its own use.
typedef struct {
    uint32_t steps[MAX_N_AXIS];
    uint32_t step_event_count;
    uint8_t  direction_bits;
    uint8_t  is_pwm_rate_adjusted;  // Tracks motions that require constant laser power/rate
} st_block_t;
static st_block_t st_block_buffer[SEGMENT_BUFFER_SIZE - 1];

// Primary stepper segment ring buffer. Contains small, short line segments for the stepper
// algorithm to execute, which are "checked-out" incrementally from the first block in the
// planner buffer. Once "checked-out", the steps in the segments buffer cannot be modified by
// the planner, where the remaining planner block steps still can.
typedef struct {
    uint16_t n_step;          // Number of step events to be executed for this segment
    uint16_t isrPeriod;       // Time to next ISR tick, in units of timer ticks
    uint8_t  st_block_index;  // Stepper block data index. Uses this information to execute this segment.
    uint8_t  amass_level;     // AMASS level for the ISR to execute this segment
    uint16_t spindle_rpm;     // TODO get rid of this.
} segment_t;
static segment_t segment_buffer[SEGMENT_BUFFER_SIZE];

// Stepper ISR data struct. Contains the running data for the main stepper ISR.
typedef struct {
    // Used by the bresenham line algorithm

    uint32_t counter[MAX_N_AXIS];  // Counter variables for the bresenham line tracer

    uint8_t  step_bits;        // Stores out_bits output to complete the step pulse delay
    uint8_t  execute_step;     // Flags step execution for each interrupt.
    uint8_t  step_pulse_time;  // Step pulse reset time after step rise
    uint8_t  step_outbits;     // The next stepping-bits to be output
    uint8_t  dir_outbits;
    uint32_t steps[MAX_N_AXIS];

    uint16_t    step_count;        // Steps remaining in line segment motion
    uint8_t     exec_block_index;  // Tracks the current st_block index. Change indicates new block.
    st_block_t* exec_block;        // Pointer to the block data for the segment being executed
    segment_t*  exec_segment;      // Pointer to the segment being executed
} stepper_t;
static stepper_t st;

// Step segment ring buffer indices
static volatile uint8_t segment_buffer_tail;
static uint8_t          segment_buffer_head;
static uint8_t          segment_next_head;

// Used to avoid ISR nesting of the "Stepper Driver Interrupt". Should never occur though.
static std::atomic<bool> busy;

// Pointers for the step segment being prepped from the planner buffer. Accessed only by the
// main program. Pointers may be planning segments or planner blocks ahead of what being executed.
static plan_block_t* pl_block;       // Pointer to the planner block being prepped
static st_block_t*   st_prep_block;  // Pointer to the stepper block data being prepped

// esp32 work around for disable in main loop
uint64_t stepper_idle_counter;  // used to count down until time to disable stepper drivers
bool     stepper_idle;

// Segment preparation data struct. Contains all the necessary information to compute new segments
// based on the current executing planner block.
typedef struct {
    uint8_t  st_block_index;  // Index of stepper common data block being prepped
    PrepFlag recalculate_flag;

    float dt_remainder;
    float steps_remaining;
    float step_per_mm;
    float req_mm_increment;

#ifdef PARKING_ENABLE
    uint8_t last_st_block_index;
    float   last_steps_remaining;
    float   last_step_per_mm;
    float   last_dt_remainder;
#endif

    uint8_t ramp_type;    // Current segment ramp state
    float   mm_complete;  // End of velocity profile from end of current planner block in (mm).
    // NOTE: This value must coincide with a step(no mantissa) when converted.
    float current_speed;     // Current speed at the end of the segment buffer (mm/min)
    float maximum_speed;     // Maximum speed of executing block. Not always nominal speed. (mm/min)
    float exit_speed;        // Exit speed of executing block (mm/min)
    float accelerate_until;  // Acceleration ramp end measured from end of block (mm)
    float decelerate_after;  // Deceleration ramp start measured from end of block (mm)

    float inv_rate;  // Used by PWM laser mode to speed up segment calculations.
    //uint16_t current_spindle_pwm;  // todo remove
    float current_spindle_rpm;

} st_prep_t;
static st_prep_t prep;

const char* stepper_names[] = {
    "Timed Steps",
    "RMT Steps",
    "I2S Steps, Stream",
    "I2S Steps, Static",
};

stepper_id_t current_stepper = DEFAULT_STEPPER;

/* "The Stepper Driver Interrupt" - This timer interrupt is the workhorse of Grbl. Grbl employs
   the venerable Bresenham line algorithm to manage and exactly synchronize multi-axis moves.
   Unlike the popular DDA algorithm, the Bresenham algorithm is not susceptible to numerical
   round-off errors and only requires fast integer counters, meaning low computational overhead
   and maximizing the Arduino's capabilities. However, the downside of the Bresenham algorithm
   is, for certain multi-axis motions, the non-dominant axes may suffer from un-smooth step
   pulse trains, or aliasing, which can lead to strange audible noises or shaking. This is
   particularly noticeable or may cause motion issues at low step frequencies (0-5kHz), but
   is usually not a physical problem at higher frequencies, although audible.
     To improve Bresenham multi-axis performance, Grbl uses what we call an Adaptive Multi-Axis
   Step Smoothing (AMASS) algorithm, which does what the name implies. At lower step frequencies,
   AMASS artificially increases the Bresenham resolution without effecting the algorithm's
   innate exactness. AMASS adapts its resolution levels automatically depending on the step
   frequency to be executed, meaning that for even lower step frequencies the step smoothing
   level increases. Algorithmically, AMASS is acheived by a simple bit-shifting of the Bresenham
   step count for each AMASS level. For example, for a Level 1 step smoothing, we bit shift
   the Bresenham step event count, effectively multiplying it by 2, while the axis step counts
   remain the same, and then double the stepper ISR frequency. In effect, we are allowing the
   non-dominant Bresenham axes step in the intermediate ISR tick, while the dominant axis is
   stepping every two ISR ticks, rather than every ISR tick in the traditional sense. At AMASS
   Level 2, we simply bit-shift again, so the non-dominant Bresenham axes can step within any
   of the four ISR ticks, the dominant axis steps every four ISR ticks, and quadruple the
   stepper ISR frequency. And so on. This, in effect, virtually eliminates multi-axis aliasing
   issues with the Bresenham algorithm and does not significantly alter Grbl's performance, but
   in fact, more efficiently utilizes unused CPU cycles overall throughout all configurations.
     AMASS retains the Bresenham algorithm exactness by requiring that it always executes a full
   Bresenham step, regardless of AMASS Level. Meaning that for an AMASS Level 2, all four
   intermediate steps must be completed such that baseline Bresenham (Level 0) count is always
   retained. Similarly, AMASS Level 3 means all eight intermediate steps must be executed.
   Although the AMASS Levels are in reality arbitrary, where the baseline Bresenham counts can
   be multiplied by any integer value, multiplication by powers of two are simply used to ease
   CPU overhead with bitshift integer operations.
     This interrupt is simple and dumb by design. All the computational heavy-lifting, as in
   determining accelerations, is performed elsewhere. This interrupt pops pre-computed segments,
   defined as constant velocity over n number of steps, from the step segment buffer and then
   executes them by pulsing the stepper pins appropriately via the Bresenham algorithm. This
   ISR is supported by The Stepper Port Reset Interrupt which it uses to reset the stepper port
   after each pulse. The bresenham line tracer algorithm controls all stepper outputs
   simultaneously with these two interrupts.

	 NOTE: This interrupt must be as efficient as possible and complete before the next ISR tick,
   which for ESP32 Grbl must be less than xx.xusec (TBD). Oscilloscope measured time in
   ISR is 5usec typical and 25usec maximum, well below requirement.
   NOTE: This ISR expects at least one step to be executed per segment.

	 The complete step timing should look this...
		Direction pin is set
		An optional delay (direction_delay_microseconds) is put after this
		The step pin is started
		A pulse length is determine (via option $0 ... pulse_microseconds)
		The pulse is ended
		Direction will remain the same until another step occurs with a change in direction.


*/

static void stepper_pulse_func();

// TODO: Replace direct updating of the int32 position counters in the ISR somehow. Perhaps use smaller
// int8 variables and update position counters only when a segment completes. This can get complicated
// with probing and homing cycles that require true real-time positions.
void IRAM_ATTR onStepperDriverTimer(void* para) {
    // Timer ISR, normally takes a step.
    //
    // When handling an interrupt within an interrupt serivce routine (ISR), the interrupt status bit
    // needs to be explicitly cleared.
    TIMERG0.int_clr_timers.t0 = 1;

    bool expected = false;
    if (busy.compare_exchange_strong(expected, true)) {
        stepper_pulse_func();

        TIMERG0.hw_timer[STEP_TIMER_INDEX].config.alarm_en = TIMER_ALARM_EN;

        busy.store(false);
    }
}

/**
 * This phase of the ISR should ONLY create the pulses for the steppers.
 * This prevents jitter caused by the interval between the start of the
 * interrupt and the start of the pulses. DON'T add any logic ahead of the
 * call to this method that might cause variation in the timing. The aim
 * is to keep pulse timing as regular as possible.
 */
static void stepper_pulse_func() {
    auto n_axis = number_axis->get();

    if (motors_direction(st.dir_outbits)) {
        auto wait_direction = direction_delay_microseconds->get();
        if (wait_direction > 0) {
            // Stepper drivers need some time between changing direction and doing a pulse.
            switch (current_stepper) {
                case ST_I2S_STREAM:
                    i2s_out_push_sample(wait_direction);
                    break;
                case ST_I2S_STATIC:
                case ST_TIMED: {
                    // wait for step pulse time to complete...some time expired during code above
                    //
                    // If we are using GPIO stepping as opposed to RMT, record the
                    // time that we turned on the direction pins so we can delay a bit.
                    // If we are using RMT, we can't delay here.
                    auto direction_pulse_start_time = esp_timer_get_time() + wait_direction;
                    while ((esp_timer_get_time() - direction_pulse_start_time) < 0) {
                        NOP();  // spin here until time to turn off step
                    }
                    break;
                }
                case ST_RMT:
                    break;
            }
        }
    }

    // If we are using GPIO stepping as opposed to RMT, record the
    // time that we turned on the step pins so we can turn them off
    // at the end of this routine without incurring another interrupt.
    // This is unnecessary with RMT and I2S stepping since both of
    // those methods time the turn off automatically.
    //
    // NOTE: We could use direction_pulse_start_time + wait_direction, but let's play it safe
    uint64_t step_pulse_start_time = esp_timer_get_time();
    motors_step(st.step_outbits);

    // If there is no step segment, attempt to pop one from the stepper buffer
    if (st.exec_segment == NULL) {
        // Anything in the buffer? If so, load and initialize next step segment.
        if (segment_buffer_head != segment_buffer_tail) {
            // Initialize new step segment and load number of steps to execute
            st.exec_segment = &segment_buffer[segment_buffer_tail];
            // Initialize step segment timing per step and load number of steps to execute.
            Stepper_Timer_WritePeriod(st.exec_segment->isrPeriod);
            st.step_count = st.exec_segment->n_step;  // NOTE: Can sometimes be zero when moving slow.
            // If the new segment starts a new planner block, initialize stepper variables and counters.
            // NOTE: When the segment data index changes, this indicates a new planner block.
            if (st.exec_block_index != st.exec_segment->st_block_index) {
                st.exec_block_index = st.exec_segment->st_block_index;
                st.exec_block       = &st_block_buffer[st.exec_block_index];
                // Initialize Bresenham line and distance counters
                for (int axis = 0; axis < n_axis; axis++) {
                    st.counter[axis] = (st.exec_block->step_event_count >> 1);
                }
            }
            st.dir_outbits = st.exec_block->direction_bits;
            // Adjust Bresenham axis increment counters according to AMASS level.
            for (int axis = 0; axis < n_axis; axis++) {
                st.steps[axis] = st.exec_block->steps[axis] >> st.exec_segment->amass_level;
            }
            // Set real-time spindle output as segment is loaded, just prior to the first step.
            spindle->set_rpm(st.exec_segment->spindle_rpm);
        } else {
            // Segment buffer empty. Shutdown.
            st_go_idle();
            if (sys.state != State::Jog) {  // added to prevent ... jog after probing crash
                // Ensure pwm is set properly upon completion of rate-controlled motion.
                if (st.exec_block != NULL && st.exec_block->is_pwm_rate_adjusted) {
                    spindle->set_rpm(0);
                }
            }
            cycle_stop = true;
            return;  // Nothing to do but exit.
        }
    }
    // Check probing state.
    if (sys_probe_state == Probe::Active) {
        probe_state_monitor();
    }
    // Reset step out bits.
    st.step_outbits = 0;

    for (int axis = 0; axis < n_axis; axis++) {
        // Execute step displacement profile by Bresenham line algorithm
        st.counter[axis] += st.steps[axis];
        if (st.counter[axis] > st.exec_block->step_event_count) {
            st.step_outbits |= bit(axis);
            st.counter[axis] -= st.exec_block->step_event_count;
            if (st.exec_block->direction_bits & bit(axis)) {
                sys_position[axis]--;
            } else {
                sys_position[axis]++;
            }
        }
    }

    // During a homing cycle, lock out and prevent desired axes from moving.
    if (sys.state == State::Homing) {
        st.step_outbits &= sys.homing_axis_lock;
    }
    st.step_count--;  // Decrement step events count
    if (st.step_count == 0) {
        // Segment is complete. Discard current segment and advance segment indexing.
        st.exec_segment = NULL;
        if (++segment_buffer_tail == SEGMENT_BUFFER_SIZE) {
            segment_buffer_tail = 0;
        }
    }

    switch (current_stepper) {
        case ST_I2S_STREAM:
            // Generate the number of pulses needed to span pulse_microseconds
            i2s_out_push_sample(pulse_microseconds->get());
            motors_unstep();
            break;
        case ST_I2S_STATIC:
        case ST_TIMED:
            // wait for step pulse time to complete...some time expired during code above
            while (esp_timer_get_time() - step_pulse_start_time < pulse_microseconds->get()) {
                NOP();  // spin here until time to turn off step
            }
            motors_unstep();
            break;
        case ST_RMT:
            break;
    }
}

void stepper_init() {
    busy.store(false); 
    
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Axis count %d", number_axis->get());
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "%s", stepper_names[current_stepper]);

#ifdef USE_I2S_STEPS
    // I2S stepper stream mode use callback but timer interrupt
    i2s_out_set_pulse_callback(stepper_pulse_func);
#endif
    // Other stepper use timer interrupt
    Stepper_Timer_Init();
}

void stepper_switch(stepper_id_t new_stepper) {
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Debug, "Switch stepper: %s -> %s", stepper_names[current_stepper], stepper_names[new_stepper]);
    if (current_stepper == new_stepper) {
        // do not need to change
        return;
    }
#ifdef USE_I2S_STEPS
    if (current_stepper == ST_I2S_STREAM) {
        if (i2s_out_get_pulser_status() != PASSTHROUGH) {
            // Called during streaming. Stop streaming.
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Debug, "Stop the I2S streaming and switch to the passthrough mode.");
            i2s_out_set_passthrough();
            i2s_out_delay();  // Wait for a change in mode.
        }
    }
#endif
    current_stepper = new_stepper;
}

// enabled. Startup init and limits call this function but shouldn't start the cycle.
void st_wake_up() {
    //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "st_wake_up");
    // Enable stepper drivers.
    motors_set_disable(false);
    stepper_idle = false;

    // Initialize step pulse timing from settings. Here to ensure updating after re-writing.
#ifdef USE_RMT_STEPS
    // Step pulse delay handling is not require with ESP32...the RMT function does it.
    if (direction_delay_microseconds->get() < 1)
    {
        // Set step pulse time. Ad hoc computation from oscilloscope. Uses two's complement.
        st.step_pulse_time = -(((pulse_microseconds->get() - 2) * ticksPerMicrosecond) >> 3);
    }
#else  // Normal operation
    // Set step pulse time. Ad hoc computation from oscilloscope. Uses two's complement.
    st.step_pulse_time = -(((pulse_microseconds->get() - 2) * ticksPerMicrosecond) >> 3);
#endif

    // Enable Stepper Driver Interrupt
    Stepper_Timer_Start();
}

// Reset and clear stepper subsystem variables
void st_reset() {
#ifdef ESP_DEBUG
    //Serial.println("st_reset()");
#endif
    // Initialize stepper driver idle state.
#ifdef USE_I2S_STEPS
    if (current_stepper == ST_I2S_STREAM) {
        i2s_out_reset();
    }
#endif
    st_go_idle();
    // Initialize stepper algorithm variables.
    memset(&prep, 0, sizeof(st_prep_t));
    memset(&st, 0, sizeof(stepper_t));
    st.exec_segment     = NULL;
    pl_block            = NULL;  // Planner block pointer used by segment buffer
    segment_buffer_tail = 0;
    segment_buffer_head = 0;  // empty = tail
    segment_next_head   = 1;
    st.step_outbits     = 0;
    st.dir_outbits      = 0;  // Initialize direction bits to default.
    // TODO do we need to turn step pins off?
}

// Stepper shutdown
void st_go_idle() {
    // Disable Stepper Driver Interrupt. Allow Stepper Port Reset Interrupt to finish, if active.
    Stepper_Timer_Stop();

    // Set stepper driver idle state, disabled or enabled, depending on settings and circumstances.
    if (((stepper_idle_lock_time->get() != 0xff) || sys_rt_exec_alarm != ExecAlarm::None || sys.state == State::Sleep) &&
        sys.state != State::Homing) {
        // Force stepper dwell to lock axes for a defined amount of time to ensure the axes come to a complete
        // stop and not drift from residual inertial forces at the end of the last movement.

        if (sys.state == State::Sleep || sys_rt_exec_alarm != ExecAlarm::None) {
            motors_set_disable(true);
        } else {
            stepper_idle         = true;  // esp32 work around for disable in main loop
            stepper_idle_counter = esp_timer_get_time() + (stepper_idle_lock_time->get() * 1000);  // * 1000 because the time is in uSecs
            // after idle countdown will be disabled in protocol loop
        }
    } else {
        motors_set_disable(false);
    }

    motors_unstep();
    st.step_outbits = 0;
}

// Called by planner_recalculate() when the executing block is updated by the new plan.
void st_update_plan_block_parameters() {
    if (pl_block != NULL) {  // Ignore if at start of a new block.
        prep.recalculate_flag.recalculate = 1;
        pl_block->entry_speed_sqr         = prep.current_speed * prep.current_speed;  // Update entry speed.
        pl_block                          = NULL;  // Flag st_prep_segment() to load and check active velocity profile.
    }
}

#ifdef PARKING_ENABLE
// Changes the run state of the step segment buffer to execute the special parking motion.
void st_parking_setup_buffer() {
    // Store step execution data of partially completed block, if necessary.
    if (prep.recalculate_flag.holdPartialBlock) {
        prep.last_st_block_index  = prep.st_block_index;
        prep.last_steps_remaining = prep.steps_remaining;
        prep.last_dt_remainder    = prep.dt_remainder;
        prep.last_step_per_mm     = prep.step_per_mm;
    }
    // Set flags to execute a parking motion
    prep.recalculate_flag.parking     = 1;
    prep.recalculate_flag.recalculate = 0;
    pl_block                          = NULL;  // Always reset parking motion to reload new block.
}

// Restores the step segment buffer to the normal run state after a parking motion.
void st_parking_restore_buffer() {
    // Restore step execution data and flags of partially completed block, if necessary.
    if (prep.recalculate_flag.holdPartialBlock) {
        st_prep_block                          = &st_block_buffer[prep.last_st_block_index];
        prep.st_block_index                    = prep.last_st_block_index;
        prep.steps_remaining                   = prep.last_steps_remaining;
        prep.dt_remainder                      = prep.last_dt_remainder;
        prep.step_per_mm                       = prep.last_step_per_mm;
        prep.recalculate_flag.holdPartialBlock = 1;
        prep.recalculate_flag.recalculate      = 1;
        prep.req_mm_increment                  = REQ_MM_INCREMENT_SCALAR / prep.step_per_mm;  // Recompute this value.
    } else {
        prep.recalculate_flag = {};
    }

    pl_block = NULL;  // Set to reload next block.
}
#endif

// Increments the step segment buffer block data ring buffer.
static uint8_t st_next_block_index(uint8_t block_index) {
    block_index++;
    return block_index == (SEGMENT_BUFFER_SIZE - 1) ? 0 : block_index;
}

/* Prepares step segment buffer. Continuously called from main program.

   The segment buffer is an intermediary buffer interface between the execution of steps
   by the stepper algorithm and the velocity profiles generated by the planner. The stepper
   algorithm only executes steps within the segment buffer and is filled by the main program
   when steps are "checked-out" from the first block in the planner buffer. This keeps the
   step execution and planning optimization processes atomic and protected from each other.
   The number of steps "checked-out" from the planner buffer and the number of segments in
   the segment buffer is sized and computed such that no operation in the main program takes
   longer than the time it takes the stepper algorithm to empty it before refilling it.
   Currently, the segment buffer conservatively holds roughly up to 40-50 msec of steps.
   NOTE: Computation units are in steps, millimeters, and minutes.
*/
void st_prep_buffer() {
    // Block step prep buffer, while in a suspend state and there is no suspend motion to execute.
    if (sys.step_control.endMotion) {
        return;
    }

    while (segment_buffer_tail != segment_next_head) {  // Check if we need to fill the buffer.
        // Determine if we need to load a new planner block or if the block needs to be recomputed.
        if (pl_block == NULL) {
            // Query planner for a queued block
            if (sys.step_control.executeSysMotion) {
                pl_block = plan_get_system_motion_block();
            } else {
                pl_block = plan_get_current_block();
            }

            if (pl_block == NULL) {
                return;  // No planner blocks. Exit.
            }

            // Check if we need to only recompute the velocity profile or load a new block.
            if (prep.recalculate_flag.recalculate) {
#ifdef PARKING_ENABLE
                if (prep.recalculate_flag.parking) {
                    prep.recalculate_flag.recalculate = 0;
                } else {
                    prep.recalculate_flag = {};
                }
#else
                prep.recalculate_flag = {};
#endif
            } else {
                // Load the Bresenham stepping data for the block.
                prep.st_block_index = st_next_block_index(prep.st_block_index);
                // Prepare and copy Bresenham algorithm segment data from the new planner block, so that
                // when the segment buffer completes the planner block, it may be discarded when the
                // segment buffer finishes the prepped block, but the stepper ISR is still executing it.
                st_prep_block                 = &st_block_buffer[prep.st_block_index];
                st_prep_block->direction_bits = pl_block->direction_bits;
                uint8_t idx;
                auto    n_axis = number_axis->get();

                // Bit-shift multiply all Bresenham data by the max AMASS level so that
                // we never divide beyond the original data anywhere in the algorithm.
                // If the original data is divided, we can lose a step from integer roundoff.
                for (idx = 0; idx < n_axis; idx++) {
                    st_prep_block->steps[idx] = pl_block->steps[idx] << maxAmassLevel;
                }
                st_prep_block->step_event_count = pl_block->step_event_count << maxAmassLevel;

                // Initialize segment buffer data for generating the segments.
                prep.steps_remaining  = (float)pl_block->step_event_count;
                prep.step_per_mm      = prep.steps_remaining / pl_block->millimeters;
                prep.req_mm_increment = REQ_MM_INCREMENT_SCALAR / prep.step_per_mm;
                prep.dt_remainder     = 0.0;  // Reset for new segment block
                if ((sys.step_control.executeHold) || prep.recalculate_flag.decelOverride) {
                    // New block loaded mid-hold. Override planner block entry speed to enforce deceleration.
                    prep.current_speed                  = prep.exit_speed;
                    pl_block->entry_speed_sqr           = prep.exit_speed * prep.exit_speed;
                    prep.recalculate_flag.decelOverride = 0;
                } else {
                    prep.current_speed = sqrt(pl_block->entry_speed_sqr);
                }

                st_prep_block->is_pwm_rate_adjusted = false;  // set default value
                // prep.inv_rate is only used if is_pwm_rate_adjusted is true
                if (spindle->inLaserMode()) {  //
                    if (pl_block->spindle == SpindleState::Ccw) {
                        // Pre-compute inverse programmed rate to speed up PWM updating per step segment.
                        prep.inv_rate                       = 1.0 / pl_block->programmed_rate;
                        st_prep_block->is_pwm_rate_adjusted = true;
                    }
                }
            }
            /* ---------------------------------------------------------------------------------
             Compute the velocity profile of a new planner block based on its entry and exit
             speeds, or recompute the profile of a partially-completed planner block if the
             planner has updated it. For a commanded forced-deceleration, such as from a feed
             hold, override the planner velocities and decelerate to the target exit speed.
            */
            prep.mm_complete  = 0.0;  // Default velocity profile complete at 0.0mm from end of block.
            float inv_2_accel = 0.5 / pl_block->acceleration;
            if (sys.step_control.executeHold) {  // [Forced Deceleration to Zero Velocity]
                // Compute velocity profile parameters for a feed hold in-progress. This profile overrides
                // the planner block profile, enforcing a deceleration to zero speed.
                prep.ramp_type = RAMP_DECEL;
                // Compute decelerate distance relative to end of block.
                float decel_dist = pl_block->millimeters - inv_2_accel * pl_block->entry_speed_sqr;
                if (decel_dist < 0.0) {
                    // Deceleration through entire planner block. End of feed hold is not in this block.
                    prep.exit_speed = sqrt(pl_block->entry_speed_sqr - 2 * pl_block->acceleration * pl_block->millimeters);
                } else {
                    prep.mm_complete = decel_dist;  // End of feed hold.
                    prep.exit_speed  = 0.0;
                }
            } else {  // [Normal Operation]
                // Compute or recompute velocity profile parameters of the prepped planner block.
                prep.ramp_type        = RAMP_ACCEL;  // Initialize as acceleration ramp.
                prep.accelerate_until = pl_block->millimeters;
                float exit_speed_sqr;
                float nominal_speed;
                if (sys.step_control.executeSysMotion) {
                    prep.exit_speed = exit_speed_sqr = 0.0;  // Enforce stop at end of system motion.
                } else {
                    exit_speed_sqr  = plan_get_exec_block_exit_speed_sqr();
                    prep.exit_speed = sqrt(exit_speed_sqr);
                }

                nominal_speed            = plan_compute_profile_nominal_speed(pl_block);
                float nominal_speed_sqr  = nominal_speed * nominal_speed;
                float intersect_distance = 0.5 * (pl_block->millimeters + inv_2_accel * (pl_block->entry_speed_sqr - exit_speed_sqr));
                if (pl_block->entry_speed_sqr > nominal_speed_sqr) {  // Only occurs during override reductions.
                    prep.accelerate_until = pl_block->millimeters - inv_2_accel * (pl_block->entry_speed_sqr - nominal_speed_sqr);
                    if (prep.accelerate_until <= 0.0) {  // Deceleration-only.
                        prep.ramp_type = RAMP_DECEL;
                        // prep.decelerate_after = pl_block->millimeters;
                        // prep.maximum_speed = prep.current_speed;
                        // Compute override block exit speed since it doesn't match the planner exit speed.
                        prep.exit_speed = sqrt(pl_block->entry_speed_sqr - 2 * pl_block->acceleration * pl_block->millimeters);
                        prep.recalculate_flag.decelOverride = 1;  // Flag to load next block as deceleration override.
                        // TODO: Determine correct handling of parameters in deceleration-only.
                        // Can be tricky since entry speed will be current speed, as in feed holds.
                        // Also, look into near-zero speed handling issues with this.
                    } else {
                        // Decelerate to cruise or cruise-decelerate types. Guaranteed to intersect updated plan.
                        prep.decelerate_after = inv_2_accel * (nominal_speed_sqr - exit_speed_sqr);
                        prep.maximum_speed    = nominal_speed;
                        prep.ramp_type        = RAMP_DECEL_OVERRIDE;
                    }
                } else if (intersect_distance > 0.0) {
                    if (intersect_distance < pl_block->millimeters) {  // Either trapezoid or triangle types
                        // NOTE: For acceleration-cruise and cruise-only types, following calculation will be 0.0.
                        prep.decelerate_after = inv_2_accel * (nominal_speed_sqr - exit_speed_sqr);
                        if (prep.decelerate_after < intersect_distance) {  // Trapezoid type
                            prep.maximum_speed = nominal_speed;
                            if (pl_block->entry_speed_sqr == nominal_speed_sqr) {
                                // Cruise-deceleration or cruise-only type.
                                prep.ramp_type = RAMP_CRUISE;
                            } else {
                                // Full-trapezoid or acceleration-cruise types
                                prep.accelerate_until -= inv_2_accel * (nominal_speed_sqr - pl_block->entry_speed_sqr);
                            }
                        } else {  // Triangle type
                            prep.accelerate_until = intersect_distance;
                            prep.decelerate_after = intersect_distance;
                            prep.maximum_speed    = sqrt(2.0 * pl_block->acceleration * intersect_distance + exit_speed_sqr);
                        }
                    } else {  // Deceleration-only type
                        prep.ramp_type = RAMP_DECEL;
                        // prep.decelerate_after = pl_block->millimeters;
                        // prep.maximum_speed = prep.current_speed;
                    }
                } else {  // Acceleration-only type
                    prep.accelerate_until = 0.0;
                    // prep.decelerate_after = 0.0;
                    prep.maximum_speed = prep.exit_speed;
                }
            }

            sys.step_control.updateSpindleRpm = true;  // Force update whenever updating block.
        }

        // Initialize new segment
        segment_t* prep_segment = &segment_buffer[segment_buffer_head];

        // Set new segment to point to the current segment data block.
        prep_segment->st_block_index = prep.st_block_index;

        /*------------------------------------------------------------------------------------
            Compute the average velocity of this new segment by determining the total distance
          traveled over the segment time DT_SEGMENT. The following code first attempts to create
          a full segment based on the current ramp conditions. If the segment time is incomplete
          when terminating at a ramp state change, the code will continue to loop through the
          progressing ramp states to fill the remaining segment execution time. However, if
          an incomplete segment terminates at the end of the velocity profile, the segment is
          considered completed despite having a truncated execution time less than DT_SEGMENT.
            The velocity profile is always assumed to progress through the ramp sequence:
          acceleration ramp, cruising state, and deceleration ramp. Each ramp's travel distance
          may range from zero to the length of the block. Velocity profiles can end either at
          the end of planner block (typical) or mid-block at the end of a forced deceleration,
          such as from a feed hold.
        */
        float dt_max   = DT_SEGMENT;                                // Maximum segment time
        float dt       = 0.0;                                       // Initialize segment time
        float time_var = dt_max;                                    // Time worker variable
        float mm_var;                                               // mm-Distance worker variable
        float speed_var;                                            // Speed worker variable
        float mm_remaining = pl_block->millimeters;                 // New segment distance from end of block.
        float minimum_mm   = mm_remaining - prep.req_mm_increment;  // Guarantee at least one step.

        if (minimum_mm < 0.0) {
            minimum_mm = 0.0;
        }

        do {
            switch (prep.ramp_type) {
                case RAMP_DECEL_OVERRIDE:
                    speed_var = pl_block->acceleration * time_var;
                    mm_var    = time_var * (prep.current_speed - 0.5 * speed_var);
                    mm_remaining -= mm_var;
                    if ((mm_remaining < prep.accelerate_until) || (mm_var <= 0)) {
                        // Cruise or cruise-deceleration types only for deceleration override.
                        mm_remaining       = prep.accelerate_until;  // NOTE: 0.0 at EOB
                        time_var           = 2.0 * (pl_block->millimeters - mm_remaining) / (prep.current_speed + prep.maximum_speed);
                        prep.ramp_type     = RAMP_CRUISE;
                        prep.current_speed = prep.maximum_speed;
                    } else {  // Mid-deceleration override ramp.
                        prep.current_speed -= speed_var;
                    }
                    break;
                case RAMP_ACCEL:
                    // NOTE: Acceleration ramp only computes during first do-while loop.
                    speed_var = pl_block->acceleration * time_var;
                    mm_remaining -= time_var * (prep.current_speed + 0.5 * speed_var);
                    if (mm_remaining < prep.accelerate_until) {  // End of acceleration ramp.
                        // Acceleration-cruise, acceleration-deceleration ramp junction, or end of block.
                        mm_remaining = prep.accelerate_until;  // NOTE: 0.0 at EOB
                        time_var     = 2.0 * (pl_block->millimeters - mm_remaining) / (prep.current_speed + prep.maximum_speed);
                        if (mm_remaining == prep.decelerate_after) {
                            prep.ramp_type = RAMP_DECEL;
                        } else {
                            prep.ramp_type = RAMP_CRUISE;
                        }
                        prep.current_speed = prep.maximum_speed;
                    } else {  // Acceleration only.
                        prep.current_speed += speed_var;
                    }
                    break;
                case RAMP_CRUISE:
                    // NOTE: mm_var used to retain the last mm_remaining for incomplete segment time_var calculations.
                    // NOTE: If maximum_speed*time_var value is too low, round-off can cause mm_var to not change. To
                    //   prevent this, simply enforce a minimum speed threshold in the planner.
                    mm_var = mm_remaining - prep.maximum_speed * time_var;
                    if (mm_var < prep.decelerate_after) {  // End of cruise.
                        // Cruise-deceleration junction or end of block.
                        time_var       = (mm_remaining - prep.decelerate_after) / prep.maximum_speed;
                        mm_remaining   = prep.decelerate_after;  // NOTE: 0.0 at EOB
                        prep.ramp_type = RAMP_DECEL;
                    } else {  // Cruising only.
                        mm_remaining = mm_var;
                    }
                    break;
                default:  // case RAMP_DECEL:
                    // NOTE: mm_var used as a misc worker variable to prevent errors when near zero speed.
                    speed_var = pl_block->acceleration * time_var;  // Used as delta speed (mm/min)
                    if (prep.current_speed > speed_var) {           // Check if at or below zero speed.
                        // Compute distance from end of segment to end of block.
                        mm_var = mm_remaining - time_var * (prep.current_speed - 0.5 * speed_var);  // (mm)
                        if (mm_var > prep.mm_complete) {                                            // Typical case. In deceleration ramp.
                            mm_remaining = mm_var;
                            prep.current_speed -= speed_var;
                            break;  // Segment complete. Exit switch-case statement. Continue do-while loop.
                        }
                    }
                    // Otherwise, at end of block or end of forced-deceleration.
                    time_var           = 2.0 * (mm_remaining - prep.mm_complete) / (prep.current_speed + prep.exit_speed);
                    mm_remaining       = prep.mm_complete;
                    prep.current_speed = prep.exit_speed;
            }

            dt += time_var;  // Add computed ramp time to total segment time.
            if (dt < dt_max) {
                time_var = dt_max - dt;  // **Incomplete** At ramp junction.
            } else {
                if (mm_remaining > minimum_mm) {  // Check for very slow segments with zero steps.
                    // Increase segment time to ensure at least one step in segment. Override and loop
                    // through distance calculations until minimum_mm or mm_complete.
                    dt_max += DT_SEGMENT;
                    time_var = dt_max - dt;
                } else {
                    break;  // **Complete** Exit loop. Segment execution time maxed.
                }
            }
        } while (mm_remaining > prep.mm_complete);  // **Complete** Exit loop. Profile complete.

        /* -----------------------------------------------------------------------------------
          Compute spindle speed PWM output for step segment
        */
        if (st_prep_block->is_pwm_rate_adjusted || sys.step_control.updateSpindleRpm) {
            if (pl_block->spindle != SpindleState::Disable) {
                float rpm = pl_block->spindle_speed;
                // NOTE: Feed and rapid overrides are independent of PWM value and do not alter laser power/rate.
                if (st_prep_block->is_pwm_rate_adjusted) {
                    rpm *= (prep.current_speed * prep.inv_rate);
                    //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "RPM %.2f", rpm);
                    //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Rates CV %.2f IV %.2f RPM %.2f", prep.current_speed, prep.inv_rate, rpm);
                }
                // If current_speed is zero, then may need to be rpm_min*(100/MAX_SPINDLE_SPEED_OVERRIDE)
                // but this would be instantaneous only and during a motion. May not matter at all.

                prep.current_spindle_rpm = rpm;
            } else {
                sys.spindle_speed        = 0.0;
                prep.current_spindle_rpm = 0.0;
            }
            sys.step_control.updateSpindleRpm = false;
        }
        prep_segment->spindle_rpm = prep.current_spindle_rpm;  // Reload segment PWM value

        /* -----------------------------------------------------------------------------------
           Compute segment step rate, steps to execute, and apply necessary rate corrections.
           NOTE: Steps are computed by direct scalar conversion of the millimeter distance
           remaining in the block, rather than incrementally tallying the steps executed per
           segment. This helps in removing floating point round-off issues of several additions.
           However, since floats have only 7.2 significant digits, long moves with extremely
           high step counts can exceed the precision of floats, which can lead to lost steps.
           Fortunately, this scenario is highly unlikely and unrealistic in CNC machines
           supported by Grbl (i.e. exceeding 10 meters axis travel at 200 step/mm).
        */
        float step_dist_remaining    = prep.step_per_mm * mm_remaining;             // Convert mm_remaining to steps
        float n_steps_remaining      = ceil(step_dist_remaining);                   // Round-up current steps remaining
        float last_n_steps_remaining = ceil(prep.steps_remaining);                  // Round-up last steps remaining
        prep_segment->n_step         = last_n_steps_remaining - n_steps_remaining;  // Compute number of steps to execute.

        // Bail if we are at the end of a feed hold and don't have a step to execute.
        if (prep_segment->n_step == 0) {
            if (sys.step_control.executeHold) {
                // Less than one step to decelerate to zero speed, but already very close. AMASS
                // requires full steps to execute. So, just bail.
                sys.step_control.endMotion = true;
#ifdef PARKING_ENABLE
                if (!(prep.recalculate_flag.parking)) {
                    prep.recalculate_flag.holdPartialBlock = 1;
                }
#endif
                return;  // Segment not generated, but current step data still retained.
            }
        }

        // Compute segment step rate. Since steps are integers and mm distances traveled are not,
        // the end of every segment can have a partial step of varying magnitudes that are not
        // executed, because the stepper ISR requires whole steps due to the AMASS algorithm. To
        // compensate, we track the time to execute the previous segment's partial step and simply
        // apply it with the partial step distance to the current segment, so that it minutely
        // adjusts the whole segment rate to keep step output exact. These rate adjustments are
        // typically very small and do not adversely effect performance, but ensures that Grbl
        // outputs the exact acceleration and velocity profiles as computed by the planner.

        dt += prep.dt_remainder;  // Apply previous segment partial step execute time
        // dt is in minutes so inv_rate is in minutes
        float inv_rate = dt / (last_n_steps_remaining - step_dist_remaining);  // Compute adjusted step rate inverse

        // Compute CPU cycles per step for the prepped segment.
        // fStepperTimer is in units of timerTicks/sec, so the dimensional analysis is
        // timerTicks/sec * 60 sec/minute * minutes = timerTicks
        uint32_t timerTicks = ceil((fStepperTimer * 60) * inv_rate);  // (timerTicks/step)
        int      level;

        // Compute step timing and multi-axis smoothing level.
        for (level = 0; level < maxAmassLevel; level++) {
            if (timerTicks < amassThreshold) {
                break;
            }
            timerTicks >>= 1;
        }
        prep_segment->amass_level = level;
        prep_segment->n_step <<= level;
        // isrPeriod is stored as 16 bits, so limit timerTicks to the
        // largest value that will fit in a uint16_t.
        prep_segment->isrPeriod = timerTicks > 0xffff ? 0xffff : timerTicks;

        // Segment complete! Increment segment buffer indices, so stepper ISR can immediately execute it.
        segment_buffer_head = segment_next_head;
        if (++segment_next_head == SEGMENT_BUFFER_SIZE) {
            segment_next_head = 0;
        }
        // Update the appropriate planner and segment data.
        pl_block->millimeters = mm_remaining;
        prep.steps_remaining  = n_steps_remaining;
        prep.dt_remainder     = (n_steps_remaining - step_dist_remaining) * inv_rate;
        // Check for exit conditions and flag to load next planner block.
        if (mm_remaining == prep.mm_complete) {
            // End of planner block or forced-termination. No more distance to be executed.
            if (mm_remaining > 0.0) {  // At end of forced-termination.
                // Reset prep parameters for resuming and then bail. Allow the stepper ISR to complete
                // the segment queue, where realtime protocol will set new state upon receiving the
                // cycle stop flag from the ISR. Prep_segment is blocked until then.
                sys.step_control.endMotion = true;
#ifdef PARKING_ENABLE
                if (!(prep.recalculate_flag.parking)) {
                    prep.recalculate_flag.holdPartialBlock = 1;
                }
#endif
                return;  // Bail!
            } else {     // End of planner block
                // The planner block is complete. All steps are set to be executed in the segment buffer.
                if (sys.step_control.executeSysMotion) {
                    sys.step_control.endMotion = true;
                    return;
                }
                pl_block = NULL;  // Set pointer to indicate check and load next planner block.
                plan_discard_current_block();
            }
        }
    }
}

// Called by realtime status reporting to fetch the current speed being executed. This value
// however is not exactly the current speed, but the speed computed in the last step segment
// in the segment buffer. It will always be behind by up to the number of segment blocks (-1)
// divided by the ACCELERATION TICKS PER SECOND in seconds.
float st_get_realtime_rate() {
    switch (sys.state) {
        case State::Cycle:
        case State::Homing:
        case State::Hold:
        case State::Jog:
        case State::SafetyDoor:
            return prep.current_speed;
        default:
            return 0.0f;
    }
}

// The argument is in units of ticks of the timer that generates ISRs
void IRAM_ATTR Stepper_Timer_WritePeriod(uint16_t timerTicks) {
    if (current_stepper == ST_I2S_STREAM) {
#ifdef USE_I2S_STEPS
        // 1 tick = fTimers / fStepperTimer
        // Pulse ISR is called for each tick of alarm_val.
        // The argument to i2s_out_set_pulse_period is in units of microseconds
        i2s_out_set_pulse_period(((uint32_t)timerTicks) / ticksPerMicrosecond);
#endif
    } else {
        timer_set_alarm_value(STEP_TIMER_GROUP, STEP_TIMER_INDEX, (uint64_t)timerTicks);
    }
}

void IRAM_ATTR Stepper_Timer_Init() {
    timer_config_t config;
    config.divider     = fTimers / fStepperTimer;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en  = TIMER_PAUSE;
    config.alarm_en    = TIMER_ALARM_EN;
    config.intr_type   = TIMER_INTR_LEVEL;
    config.auto_reload = true;
    timer_init(STEP_TIMER_GROUP, STEP_TIMER_INDEX, &config);
    timer_set_counter_value(STEP_TIMER_GROUP, STEP_TIMER_INDEX, 0x00000000ULL);
    timer_enable_intr(STEP_TIMER_GROUP, STEP_TIMER_INDEX);
    timer_isr_register(STEP_TIMER_GROUP, STEP_TIMER_INDEX, onStepperDriverTimer, NULL, 0, NULL);
}

void IRAM_ATTR Stepper_Timer_Start() {
#ifdef ESP_DEBUG
    //Serial.println("ST Start");
#endif
    if (current_stepper == ST_I2S_STREAM) {
#ifdef USE_I2S_STEPS
        i2s_out_set_stepping();
#endif
    } else {
        timer_set_counter_value(STEP_TIMER_GROUP, STEP_TIMER_INDEX, 0x00000000ULL);
        timer_start(STEP_TIMER_GROUP, STEP_TIMER_INDEX);
        TIMERG0.hw_timer[STEP_TIMER_INDEX].config.alarm_en = TIMER_ALARM_EN;
    }
}

void IRAM_ATTR Stepper_Timer_Stop() {
#ifdef ESP_DEBUG
    //Serial.println("ST Stop");
#endif
    if (current_stepper == ST_I2S_STREAM) {
#ifdef USE_I2S_STEPS
        i2s_out_set_passthrough();
#endif
    } else {
        timer_pause(STEP_TIMER_GROUP, STEP_TIMER_INDEX);
    }
}

bool get_stepper_disable() {  // returns true if steppers are disabled
    bool disabled = false;
#ifdef STEPPERS_DISABLE_PIN
    disabled = digitalRead(STEPPERS_DISABLE_PIN);
#else
    return false;  // thery are never disabled if there is no pin defined
#endif
    if (step_enable_invert->get()) {
        disabled = !disabled;  // Apply pin invert.
    }
    return disabled;
}
