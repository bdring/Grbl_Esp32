// Stepping is the interface class for operations that
// apply to all steppers at once.

class Stepping {
protected:
    const char* _name;

public:
    Stepping(const char* name) : _name(name) {}
    const char*  name() { return _name; }
    virtual void init()                         = 0;
    virtual void reset()                        = 0;
    virtual void setPeriod(uint16_t timerTicks) = 0;
    virtual void start()                        = 0;
    virtual void stop()                         = 0;
    void         lowLatency() {};
    void         normalLatency() {};
    void         backoffDelay() {};
};

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

class RMTStepping : public Stepping {
public:
    RMTStepping() { Stepping("RMT"); }
    void init { StepperTimerInit(); }
    void reset() {}
    void setPeriod(uint16_t timerTicks) { timer_set_alarm_value(STEP_TIMER_GROUP, STEP_TIMER_INDEX, (uint64_t)timerTicks); }
    void start() {
        timer_set_counter_value(STEP_TIMER_GROUP, STEP_TIMER_INDEX, 0x00000000ULL);
        timer_start(STEP_TIMER_GROUP, STEP_TIMER_INDEX);
        TIMERG0.hw_timer[STEP_TIMER_INDEX].config.alarm_en = TIMER_ALARM_EN;
    }
    void stop() { timer_pause(STEP_TIMER_GROUP, STEP_TIMER_INDEX); }
    void finishStep(uint64_t startTime) {}
}

void wait_pulse_time(uint64_t startTime) {
    while (esp_timer_get_time() - startTime < pulse_microseconds->get()) {
        NOP();  // spin here until time to turn off step
    }
}

class TimedStepping : public Stepping {
public:
    TimedStepping() { Stepping("Timed"); }
    void init { StepperTimerInit(); }
    void reset() {}
    void setPeriod(uint16_t timerTicks) { timer_set_alarm_value(STEP_TIMER_GROUP, STEP_TIMER_INDEX, (uint64_t)timerTicks); }
    void start() {
        timer_set_counter_value(STEP_TIMER_GROUP, STEP_TIMER_INDEX, 0x00000000ULL);
        timer_start(STEP_TIMER_GROUP, STEP_TIMER_INDEX);
        TIMERG0.hw_timer[STEP_TIMER_INDEX].config.alarm_en = TIMER_ALARM_EN;
    }
    void stop() { timer_pause(STEP_TIMER_GROUP, STEP_TIMER_INDEX); }
    void finishStep(uint64_t startTime) {
        wait_pulse_time(startTime);
        motors_unstep();
    }
}

class I2SStepping : public Stepping {
private:
    bool _streaming;

public:
    I2SStepping() : Stepping("I2S"), _streaming(true) {}
    void init() { i2s_out_set_pulse_callback(stepper_pulse_func); }
    void reset() {
        if (_streaming) {
            i2s_out_reset();
        }
    }
    void setPeriod(uint16_t timerTicks) {
        if (_streaming) {
            // 1 tick = fTimers / fStepperTimer
            // Pulse ISR is called for each tick of alarm_val.
            // The argument to i2s_out_set_pulse_period is in units of microseconds
            i2s_out_set_pulse_period(((uint32_t)timerTicks) / ticksPerMicrosecond);
        }
    }
    void start() {
        if (_streaming) {
            i2s_out_set_stepping();
        }

        timer_set_counter_value(STEP_TIMER_GROUP, STEP_TIMER_INDEX, 0x00000000ULL);
        timer_start(STEP_TIMER_GROUP, STEP_TIMER_INDEX);
        TIMERG0.hw_timer[STEP_TIMER_INDEX].config.alarm_en = TIMER_ALARM_EN;
    }
    void stop() {
        if (_streaming) {
            i2s_out_set_passthrough();
        }
    }
    void finishStep(uint64_t startTime) {
        if (_streaming) {
            i2s_out_push_sample(pulse_microseconds->get());
        } else {
            wait_pulse_time(startTime);
        }
        motors_unstep();
    }
    void lowLatency() {
        if (_streaming) {
            if (i2s_out_get_pulser_status() != PASSTHROUGH) {
                // Called during streaming. Stop streaming.
                grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Debug, "Stop the I2S streaming and switch to the passthrough mode.");
                i2s_out_set_passthrough();
                i2s_out_delay();  // Wait for a change in mode.
            }
        }
    }
    void          normalLatency() {}
    override void backoffDelay() { delay_ms(I2S_OUT_DELAY_MS); }
}
