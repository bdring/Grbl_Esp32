#pragma once

/*
  Stepper.h - stepper motor driver: executes motion plans of planner.c using the stepper motors
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

#ifndef SEGMENT_BUFFER_SIZE
#    define SEGMENT_BUFFER_SIZE 6
#endif

#include "Grbl.h"
#include "Config.h"

// Some useful constants.
const double DT_SEGMENT              = (1.0 / (ACCELERATION_TICKS_PER_SECOND * 60.0));  // min/segment
const double REQ_MM_INCREMENT_SCALAR = 1.25;
const int    RAMP_ACCEL              = 0;
const int    RAMP_CRUISE             = 1;
const int    RAMP_DECEL              = 2;
const int    RAMP_DECEL_OVERRIDE     = 3;

struct PrepFlag {
    uint8_t recalculate : 1;
    uint8_t holdPartialBlock : 1;
    uint8_t parking : 1;
    uint8_t decelOverride : 1;
};

// fStepperTimer should be an integer divisor of the bus speed, i.e. of fTimers
const uint32_t fStepperTimer = 20000000; // frequency of step pulse timer
const int ticksPerMicrosecond = fStepperTimer / 1000000;

// Define Adaptive Multi-Axis Step-Smoothing(AMASS) levels and cutoff frequencies. The highest level
// frequency bin starts at 0Hz and ends at its cutoff frequency. The next lower level frequency bin
// starts at the next higher cutoff frequency, and so on. The cutoff frequencies for each level must
// be considered carefully against how much it over-drives the stepper ISR, the accuracy of the 16-bit
// timer, and the CPU overhead. Level 0 (no AMASS, normal operation) frequency bin starts at the
// Level 1 cutoff frequency and up to as fast as the CPU allows (over 30kHz in limited testing).
// For efficient computation, each cutoff frequency is twice the previous one.
// NOTE: AMASS cutoff frequency multiplied by ISR overdrive factor must not exceed maximum step frequency.
// NOTE: Current settings are set to overdrive the ISR to no more than 16kHz, balancing CPU overhead
// and timer accuracy.  Do not alter these settings unless you know what you are doing.

const uint32_t amassThreshold = fStepperTimer / 8000;
const int maxAmassLevel = 3;  // Each level increase doubles the threshold

const timer_group_t STEP_TIMER_GROUP = TIMER_GROUP_0;
const timer_idx_t   STEP_TIMER_INDEX = TIMER_0;

// esp32 work around for diable in main loop
extern uint64_t stepper_idle_counter;
extern bool     stepper_idle;

//extern uint8_t ganged_mode;

enum stepper_id_t {
    ST_TIMED = 0,
    ST_RMT,
    ST_I2S_STREAM,
    ST_I2S_STATIC,
};

#ifndef DEFAULT_STEPPER
#    if defined(USE_I2S_STEPS)
#        define DEFAULT_STEPPER ST_I2S_STREAM
#    elif defined(USE_RMT_STEPS)
#        define DEFAULT_STEPPER ST_RMT
#    else
#        define DEFAULT_STEPPER ST_TIMED
#    endif
#endif

extern const char*  stepper_names[];
extern stepper_id_t current_stepper;

// -- Task handles for use in the notifications
void IRAM_ATTR onSteppertimer();
void IRAM_ATTR onStepperOffTimer();

void stepper_init();
void stepper_switch(stepper_id_t new_stepper);

// Enable steppers, but cycle does not start unless called by motion control or realtime command.
void st_wake_up();

// Immediately disables steppers
void st_go_idle();

// Reset the stepper subsystem variables
void st_reset();

// Changes the run state of the step segment buffer to execute the special parking motion.
void st_parking_setup_buffer();

// Restores the step segment buffer to the normal run state after a parking motion.
void st_parking_restore_buffer();

// Reloads step segment buffer. Called continuously by realtime execution system.
void st_prep_buffer();

// Called by planner_recalculate() when the executing block is updated by the new plan.
void st_update_plan_block_parameters();

// Called by realtime status reporting if realtime rate reporting is enabled in config.h.
float st_get_realtime_rate();

// disable (or enable) steppers via STEPPERS_DISABLE_PIN
bool get_stepper_disable();  // returns the state of the pin

void set_stepper_pins_on(uint8_t onMask);
void set_direction_pins_on(uint8_t onMask);

void Stepper_Timer_WritePeriod(uint16_t timerTicks);
void Stepper_Timer_Init();
void Stepper_Timer_Start();
void Stepper_Timer_Stop();
