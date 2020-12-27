#include "Stepping.h"
#include "Stepper.h"

static volatile uint8_t busy;

void stepping_select() {
#if defined(USE_I2S_OUT)

#elif defined(USE_RMT_STEPS)

#else

#endif
}

// TODO: Replace direct updating of the int32 position counters in the ISR somehow. Perhaps use smaller
// int8 variables and update position counters only when a segment completes. This can get complicated
// with probing and homing cycles that require true real-time positions.
void IRAM_ATTR onStepperDriverTimer(void* para) {  // ISR It is time to take a step
    //const int timer_idx = (int)para;  // get the timer index
    TIMERG0.int_clr_timers.t0 = 1;
    if (busy) {
        return;  // The busy-flag is used to avoid reentering this interrupt
    }
    busy = true;

    stepper_pulse_func();

    TIMERG0.hw_timer[STEP_TIMER_INDEX].config.alarm_en = TIMER_ALARM_EN;
    busy                                               = false;
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

void wait_pulse_time(uint64_t startTime) {
    while (esp_timer_get_time() - startTime < pulse_microseconds->get()) {
        NOP();  // spin here until time to turn off step
    }
}

void RMTStepping::init() {
    Stepper_Timer_Init();
}

void RMTStepping::setPeriod(uint16_t timerTicks) {
    timer_set_alarm_value(STEP_TIMER_GROUP, STEP_TIMER_INDEX, (uint64_t)timerTicks);
}

void RMTStepping::start() {
    timer_set_counter_value(STEP_TIMER_GROUP, STEP_TIMER_INDEX, 0x00000000ULL);
    timer_start(STEP_TIMER_GROUP, STEP_TIMER_INDEX);
    TIMERG0.hw_timer[STEP_TIMER_INDEX].config.alarm_en = TIMER_ALARM_EN;
}

void RMTStepping::stop() {
    timer_pause(STEP_TIMER_GROUP, STEP_TIMER_INDEX);
    busy = false;
}

// ==========================================================================

void TimedStepping::init() {
    Stepper_Timer_Init();
}

void TimedStepping::setPeriod(uint16_t timerTicks) {
    timer_set_alarm_value(STEP_TIMER_GROUP, STEP_TIMER_INDEX, (uint64_t)timerTicks);
}

void TimedStepping::start() {
    timer_set_counter_value(STEP_TIMER_GROUP, STEP_TIMER_INDEX, 0x00000000ULL);
    timer_start(STEP_TIMER_GROUP, STEP_TIMER_INDEX);
    TIMERG0.hw_timer[STEP_TIMER_INDEX].config.alarm_en = TIMER_ALARM_EN;
}

void TimedStepping::stop() {
    timer_pause(STEP_TIMER_GROUP, STEP_TIMER_INDEX);
    busy = false;
}

void TimedStepping::finishStep(uint64_t startTime) {
    wait_pulse_time(startTime);
    motors_unstep();
}

// =================================================================

void I2SStepping::init() {
    i2s_out_set_pulse_callback(stepper_pulse_func);
}

void I2SStepping::reset() {
    if (_streaming) {
        i2s_out_reset();
    }
}

void I2SStepping::setPeriod(uint16_t timerTicks) {
    if (_streaming) {
        // 1 tick = fTimers / fStepperTimer
        // Pulse ISR is called for each tick of alarm_val.
        // The argument to i2s_out_set_pulse_period is in units of microseconds
        i2s_out_set_pulse_period(((uint32_t)timerTicks) / ticksPerMicrosecond);
    }
}

void I2SStepping::stop() {
    if (_streaming) {
        i2s_out_set_passthrough();
    }
}

void I2SStepping::start() {
    if (_streaming) {
        i2s_out_set_stepping();
    }

    timer_set_counter_value(STEP_TIMER_GROUP, STEP_TIMER_INDEX, 0x00000000ULL);
    timer_start(STEP_TIMER_GROUP, STEP_TIMER_INDEX);
    TIMERG0.hw_timer[STEP_TIMER_INDEX].config.alarm_en = TIMER_ALARM_EN;
}

void I2SStepping::finishStep(uint64_t startTime) {
    if (_streaming) {
        i2s_out_push_sample(pulse_microseconds->get());
    } else {
        wait_pulse_time(startTime);
    }
    motors_unstep();
}

void I2SStepping::lowLatency() {
    if (_streaming) {
        if (i2s_out_get_pulser_status() != PASSTHROUGH) {
            // Called during streaming. Stop streaming.
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Debug, "Stop the I2S streaming and switch to the passthrough mode.");
            i2s_out_set_passthrough();
            i2s_out_delay();  // Wait for a change in mode.
        }
    }
}

void I2SStepping::backoffDelay() {
    delay_ms(I2S_OUT_DELAY_MS);
}

I2SStepping* stepping;
