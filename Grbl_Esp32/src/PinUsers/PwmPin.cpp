#include "PwmPin.h"

#include "../Pin.h"
#include "../Assert.h"
#include "LimitedResource.h"

#include <Arduino.h>

namespace PinUsers {
    class NativePwm : public PwmDetail {
        static LimitedResource<8>& PwmChannelResources() {
            // The ESP32 chip has 16 PWM channels, but the second 8 channels share the same timers as the first 8.
            // So, in essense, we use only 8 here, so we can configure them independently.

            static LimitedResource<8> instances_;
            return instances_;
        }

        Pin      pin_;
        int      pwmChannel_;
        uint32_t frequency_;
        uint32_t maxDuty_;
        uint8_t  resolutionBits_;

        /*
            Calculate the highest precision of a PWM based on the frequency in bits

            80,000,000 / freq = period
            determine the highest precision where (1 << precision) < period
        */
        uint8_t calculatePwmPrecision(uint32_t freq) {
            uint8_t precision = 0;

            // increase the precision (bits) until it exceeds allow by frequency the max or is 16
            // TODO is there a named value for the 80MHz?
            while ((1 << precision) < (uint32_t)(80000000 / freq) && precision <= 16) {
                precision++;
            }

            return precision - 1;
        }

    public:
        NativePwm(Pin pin, uint32_t frequency, uint32_t maxDuty) : frequency_(frequency), maxDuty_(maxDuty) {
            auto native = pin.getNative(Pin::Capabilities::PWM | Pin::Capabilities::Native);

            pwmChannel_ = PwmChannelResources().tryClaim();
            Assert(pwmChannel_ != -1, "PWM Channel could not be claimed. Are all PWM channels in use?");

            resolutionBits_ = calculatePwmPrecision(frequency);

            ledcSetup(pwmChannel_, frequency, resolutionBits_);
            ledcAttachPin(native, pwmChannel_);
            ledcWrite(pwmChannel_, 0);

            // grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "PWM Output:%d on Pin:%s Freq:%0.0fHz", _number, _pin.name().c_str(), _pwm_frequency);
        }

        uint32_t getFrequency() const override { return frequency_; }
        uint32_t getMaxDuty() const override { return maxDuty_; }

        void setValue(float value) {
            if (value < 0) {
                value = 0;
            } else if (value > 1) {
                value = 1;
            }

            auto duty = value * (uint32_t(1) << int(resolutionBits_));
            ledcWrite(pwmChannel_, uint32_t(duty));
        }

        ~NativePwm() override {
            ledcWrite(pwmChannel_, 0);
            PwmChannelResources().release(pwmChannel_);
        }
    };

    PwmPin::PwmPin(Pin pin, uint32_t frequency, uint32_t maxDuty) {
        Assert(pin.capabilities().has(Pin::Capabilities::PWM | Pin::Capabilities::Native), "Pin does not support PWM");

        // For now, we only support Native pins. In the future, we might support other pins.
        _detail = new NativePwm(pin, frequency, maxDuty);
    }
}
