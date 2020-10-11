#pragma once

#include "Arduino.h"
#include <src/Assert.h>

struct SoftwarePin {
    SoftwarePin() : callback(), argument(nullptr), mode(0), value(false), pinMode(0) {}

    void (*callback)(void*);
    void* argument;
    int   mode;

    bool value;
    int  pinMode;

    void handleISR() { callback(argument); }
    void reset() {
        callback = nullptr;
        argument = nullptr;
        mode     = 0;
        value    = false;
        pinMode  = 0;
    }
};

class SoftwareGPIO {
    SoftwareGPIO() {}
    SoftwareGPIO(const SoftwareGPIO&) = delete;

    SoftwarePin pins[256];

public:
    static SoftwareGPIO& instance() {
        static SoftwareGPIO instance_;
        return instance_;
    }

    static void reset() {
        auto& inst = instance();
        for (int i = 0; i < 256; ++i) {
            inst.pins[i].reset();
        }
    }

    bool testMode(int index, int mode) {
        auto& pin = pins[index];
        return (pin.pinMode & mode) == mode;
    }

    void setMode(int index, int mode) {
        auto& pin   = pins[index];
        pin.pinMode = mode;
    }

    void set(int index, bool value) {
        auto& pin = pins[index];
        auto  old = pin.value;
        pin.value = value;

        if (old != value) {
            switch (pin.mode) {
                case RISING:
                    if (!old && value) {
                        pin.handleISR();
                    }
                    break;
                case FALLING:
                    if (old && !value) {
                        pin.handleISR();
                    }
                    break;
                case CHANGE:
                    if (old != value) {
                        pin.handleISR();
                    }
                    break;
                    // These two are a bit weird I'd say:
                    // case ONLOW:
                    //     if (old && !value) {
                    //         isr.handleISR();
                    //     }
                    //     break;
                    // case ONHIGH:
                    //     if (!old && value) {
                    //         isr.handleISR();
                    //     }
                    //     break;
            }
        }
    }

    bool get(int index) const { return pins[index].value; }

    void attachISR(int index, void (*callback)(void* arg), void* arg, int mode) {
        auto& pin = pins[index];
        Assert(pin.mode == 0, "ISR mode should be 0 when attaching interrupt. Another interrupt is already attached.");

        pin.callback = callback;
        pin.argument = arg;
        pin.mode     = mode;
    }

    void detachISR(int index) {
        auto& pin    = pins[index];
        pin.mode     = 0;
        pin.argument = nullptr;
    }
};
