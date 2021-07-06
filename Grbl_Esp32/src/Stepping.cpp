#include <Arduino.h>
#include "I2SOut.h"
#include "EnumItem.h"
#include "Stepping.h"
#include "Stepper.h"
#include "Report.h"

#include <atomic>

namespace Machine {

    EnumItem stepTypes[] = { { Stepping::TIMED, "Timed" },
                             { Stepping::RMT, "RMT" },
                             { Stepping::I2S_STATIC, "I2S_static" },
                             { Stepping::I2S_STREAM, "I2S_stream" },
                             EnumItem(Stepping::RMT) };

    static std::atomic<bool> busy;

    void Stepping::init() {
        info_serial("Step type:%s Pulse:%dus Dsbl Delay:%dus Dir Delay:%dus Idle Delay:%dms",
                    // stepping->name(),
                    stepTypes[_engine].name,
                    _pulseUsecs,
                    _disableDelayUsecs,
                    _directionDelayUsecs,
                    _idleMsecs);

        // Prepare stepping interrupt callbacks.  The one that is actually
        // used is determined by timerStart() and timerStop()

        const bool isEdge  = false;
        const bool countUp = true;

        // Setup a timer for direct stepping
        stepTimer = timerBegin(stepTimerNumber, fTimers / fStepperTimer, countUp);
        timerAttachInterrupt(stepTimer, onStepperDriverTimer, isEdge);

        // Register pulse_func with the I2S subsystem
        // This could be done via the linker.
        //        i2s_out_set_pulse_callback(Stepper::pulse_func);

        busy.store(false);
    }

    // Wait for motion to complete; the axes can still be moving
    // after the data has been sent to the stepping engine, due
    // to queuing delays.
    void Stepping::synchronize() {
        if (_engine == I2S_STREAM) {
            i2s_out_delay();
            // XXX instead of a delay, we could sense when the DMA and
            // FIFO have drained. It might be as simple as waiting for
            // I2SO.conf1.tx_start == 0, while yielding.
            // delay_ms(I2S_OUT_DELAY_MS);
        }
    }
    void Stepping::reset() {
        if (_engine == I2S_STREAM) {
            i2s_out_reset();
        }
    }
    void Stepping::beginLowLatency() {
        _switchedStepper = _engine == I2S_STREAM;
        if (_switchedStepper) {
            _engine = I2S_STATIC;
        }
    }
    void Stepping::endLowLatency() {
        if (_switchedStepper) {
            if (i2s_out_get_pulser_status() != PASSTHROUGH) {
                // Called during streaming. Stop streaming.
                // debug_serial("Stop the I2S streaming and switch to the passthrough mode.");
                i2s_out_set_passthrough();
                i2s_out_delay();  // Wait for a change in mode.
            }
            _engine = I2S_STREAM;
        }
    }
    void Stepping::spinDelay(int64_t start_time, uint32_t durationUs) {
        int64_t endTime = start_time + durationUs;
        while ((esp_timer_get_time() - endTime) < 0) {
            NOP();
        }
    }
    void Stepping::waitPulse() {
        uint64_t pulseEndTime;
        switch (_engine) {
            case I2S_STREAM:
                // Generate the number of pulses needed to span pulse_microseconds
                i2s_out_push_sample(_pulseUsecs);
                break;
            case I2S_STATIC:
            case TIMED:
                spinDelay(_stepPulseStartTime, _pulseUsecs);
                break;
            case RMT:
                // RMT generates the trailing edges in hardware
                return;
        }
    }
    void Stepping::waitDirection() {
        if (_directionDelayUsecs) {
            // Stepper drivers need some time between changing direction and doing a pulse.
            switch (_engine) {
                case stepper_id_t::I2S_STREAM:
                    i2s_out_push_sample(_directionDelayUsecs);
                    break;
                case stepper_id_t::I2S_STATIC:
                case stepper_id_t::TIMED: {
                    // wait for step pulse time to complete...some time expired during code above
                    //
                    // If we are using GPIO stepping as opposed to RMT, record the
                    // time that we turned on the direction pins so we can delay a bit.
                    // If we are using RMT, we can't delay here.
                    spinDelay(esp_timer_get_time(), _directionDelayUsecs);
                    break;
                    case stepper_id_t::RMT:
                        break;
                }
            }
        }
    }

    void Stepping::startPulseTimer() {
        switch (_engine) {
            case stepper_id_t::I2S_STREAM:
                break;
            case stepper_id_t::I2S_STATIC:
            case stepper_id_t::TIMED:
                _stepPulseStartTime = esp_timer_get_time();
                break;
            case stepper_id_t::RMT:
                break;
        }
    }

    // The argument is in units of ticks of the timer that generates ISRs
    void IRAM_ATTR Stepping::setTimerPeriod(uint16_t timerTicks) {
        if (_engine == I2S_STREAM) {
            // 1 tick = fTimers / fStepperTimer
            // Pulse ISR is called for each tick of alarm_val.
            // The argument to i2s_out_set_pulse_period is in units of microseconds
            i2s_out_set_pulse_period(((uint32_t)timerTicks) / ticksPerMicrosecond);
        } else {
            timerAlarmWrite(stepTimer, (uint64_t)timerTicks, autoReload);
        }
    }
    void Stepping::startTimer() {
        if (_engine == I2S_STREAM) {
            i2s_out_set_stepping();
        } else {
            timerWrite(stepTimer, 0ULL);
            timerAlarmEnable(stepTimer);
        }
    }
    void Stepping::stopTimer() {
        if (_engine == I2S_STREAM) {
            i2s_out_set_passthrough();
        } else if (stepTimer) {
            timerAlarmDisable(stepTimer);
        }
    }

    // Stepper timer configuration
    hw_timer_t* Stepping::stepTimer = nullptr;  // Handle

    // Counts stepper ISR invocations.  This variable can be inspected
    // from the mainline code to determine if the stepper ISR is running,
    // since printing from the ISR is not a good idea.
    uint32_t Stepping::isr_count = 0;

    // Used to avoid ISR nesting of the "Stepper Driver Interrupt". Should never occur though.
    // TODO: Replace direct updating of the int32 position counters in the ISR somehow. Perhaps use smaller
    // int8 variables and update position counters only when a segment completes. This can get complicated
    // with probing and homing cycles that require true real-time positions.
    void IRAM_ATTR Stepping::onStepperDriverTimer() {
        // Timer ISR, normally takes a step.

        // The intermediate handler clears the timer interrupt so we need not do it here

        bool expected = false;
        if (busy.compare_exchange_strong(expected, true)) {
            ++isr_count;

            // Using autoReload results is less timing jitter so it is
            // probably best to have it on.  We keep the variable for
            // convenience in debugging.
            if (!autoReload) {
                timerWrite(stepTimer, 0ULL);
            }

            // It is tempting to defer this until after pulse_func(),
            // but if pulse_func() determines that no more stepping
            // is required and disables the timer, then that will be undone
            // if the re-enable happens afterwards.

            timerAlarmEnable(stepTimer);

            Stepper::pulse_func();

            busy.store(false);
        }
    }

    void Stepping::group(Configuration::HandlerBase& handler) {
        handler.item("engine", _engine, stepTypes);
        handler.item("idle_ms", _idleMsecs);
        handler.item("pulse_us", _pulseUsecs);
        handler.item("dir_delay_us", _directionDelayUsecs);
        handler.item("disable_delay_us", _disableDelayUsecs);
    }
}
