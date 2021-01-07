#include "PwmPin.h"

#include "../Pin.h"
#include "../Assert.h"
#include "LimitedResource.h"

#include <Arduino.h>

namespace PinUsers {
    class NativePwm : public PwmDetail {
        static NativePwm** PwmChannelResources() {
            // The ESP32 chip has 16 PWM channels, but there are 4 timers, 2 hardware instances per timer and 2
            // software instances per timer.

            static NativePwm* instances_[16] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                 nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
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

        static int TryGrabChannel(uint32_t frequency) {
            auto instances = PwmChannelResources();

            // 1. Attempt to re-use the same frequency
            for (int i = 0; i < 4; ++i) {
                if (instances[i * 2] != nullptr && instances[i * 2 + 1] == nullptr) {
                    // See if this is a frequency match:
                    if (instances[i * 2]->frequency_ == frequency) {
                        return i * 2 + 1;
                    }
                }
            }

            // 2. No luck. Check if we have free slots in the hardware PWM channels / timers
            for (int i = 0; i < 4; ++i) {
                if (instances[i * 2] == nullptr) {
                    return i * 2;
                }
            }

            // 3. Check if we have free slots in the software PWM channels with the same frequency
            for (int i = 0; i < 4; ++i) {
                if (instances[i * 2]->frequency_ == frequency) {
                    if (instances[i * 2 + 8] == nullptr) {
                        return i * 2 + 8;
                    } else if (instances[i * 2 + 9] == nullptr) {
                        // See if this is a frequency match:
                        return i * 2 + 9;
                    }
                }
            }

            // 4. Re-assign hardware PWM channels to software channels, because we ran out of hardware pwm.
            for (int i = 0; i < 3; ++i) {
                for (int j = i + 1; j < 4; ++j) {
                    if (instances[i * 2]->frequency_ == instances[j * 2]->frequency_) {
                        // Only make sense if we can clear up all the hardware channel in [j],
                        // otherwise we save nothing:
                        int count = 0;
                        for (int chan = 0; chan < 2; ++chan) {
                            count += (instances[i * 2 + chan] != nullptr) ? 1 : 0;
                            count += (instances[i * 2 + chan + 8] != nullptr) ? 1 : 0;
                            count += (instances[j * 2 + chan] != nullptr) ? 1 : 0;
                            count += (instances[j * 2 + chan + 8] != nullptr) ? 1 : 0;
                        }

                        if (count <= 4) {
                            // We can move PWM channels and free up some space for more frequencies:
                            NativePwm* tmp[4] = { nullptr, nullptr, nullptr, nullptr };
                            int        n      = 0;
                            for (int chan = 0; chan < 2; ++chan) {
                                if (instances[i * 2 + chan] != nullptr) {
                                    tmp[n++]                = instances[i * 2 + chan];
                                    instances[i * 2 + chan] = nullptr;
                                }
                                if (instances[i * 2 + chan + 8] != nullptr) {
                                    tmp[n++]                    = instances[i * 2 + chan + 8];
                                    instances[i * 2 + chan + 8] = nullptr;
                                }
                                if (instances[j * 2 + chan] != nullptr) {
                                    tmp[n++]                = instances[j * 2 + chan];
                                    instances[j * 2 + chan] = nullptr;
                                }
                                if (instances[j * 2 + chan + 8] != nullptr) {
                                    tmp[n++]                    = instances[j * 2 + chan + 8];
                                    instances[j * 2 + chan + 8] = nullptr;
                                }
                            }

                            // Reassign:
                            tmp[0]->reassign(i * 2 + 0);
                            tmp[1]->reassign(i * 2 + 1);
                            tmp[2]->reassign(i * 2 + 8);
                            tmp[3]->reassign(i * 2 + 9);

                            // Now that we cleared up some space, we can try again:
                            return TryGrabChannel(frequency);
                        }
                    }
                }
            }

            // 5. Nothing is available.
            return -1;
        }

        void reassign(int target) {
            auto native = pin_.getNative(Pin::Capabilities::PWM | Pin::Capabilities::Native);

            // grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Assigning PWM Output Pin:%s Freq:%0.0fHz to channel %d", _pin.name().c_str(), _pwm_frequency, target);

            ledcWrite(pwmChannel_, 0);
            ledcDetachPin(native);

            auto instances = PwmChannelResources();
            instances[target] = this;

            pwmChannel_ = target;
            setupPwm(native);
        }

        void setupPwm(uint8_t native) {
            resolutionBits_ = calculatePwmPrecision(frequency_);

            ledcSetup(pwmChannel_, frequency_, resolutionBits_);
            ledcAttachPin(native, pwmChannel_);
            ledcWrite(pwmChannel_, 0);
        }

    public:
        NativePwm(Pin pin, uint32_t frequency, uint32_t maxDuty) : pin_(pin), frequency_(frequency), maxDuty_(maxDuty) {
            auto native = pin.getNative(Pin::Capabilities::PWM | Pin::Capabilities::Native);

            pwmChannel_ = TryGrabChannel(frequency);
            Assert(pwmChannel_ != -1, "PWM Channel could not be claimed. Are all PWM channels in use?");

            setupPwm(native);

            // Store instance:
            auto instances         = PwmChannelResources();
            instances[pwmChannel_] = this;
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
            auto native = pin_.getNative(Pin::Capabilities::PWM | Pin::Capabilities::Native);

            ledcWrite(pwmChannel_, 0);
            ledcDetachPin(native);

            // Release resource:
            auto instances = PwmChannelResources();
            for (int i = 0; i < 16; ++i) {
                if (instances[i] == this) {
                    instances[i] = nullptr;
                }
            }
        }
    };

    PwmPin::PwmPin(Pin pin, uint32_t frequency, uint32_t maxDuty) {
        Assert(pin.capabilities().has(Pin::Capabilities::PWM | Pin::Capabilities::Native), "Pin does not support PWM");

        // For now, we only support Native pins. In the future, we might support other pins.
        _detail = new NativePwm(pin, frequency, maxDuty);
    }
}
