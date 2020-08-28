#pragma once

#include "Pins/PinLookup.h"
#include "Pins/PinDetail.h"
#include "Pins/PinCapabilities.h"
#include "Pins/PinAttributes.h"

#include <Arduino.h>  // for IRAM_ATTR
#include <cstdint>
#include <cstring>

// Forward declarations:
class String;

class Pin {
    // Helper for handling callbacks and mapping them to the proper class:
    template <typename ThisType, void (ThisType::*Callback)()>
    struct InterruptCallbackHelper {
        static void IRAM_ATTR callback(void* ptr) { static_cast<ThisType*>(ptr)->*Callback(); }
    };

    // Helper for handling callbacks and mapping them to the proper class. This one is just meant
    // for backward compatibility:
    template <void (*Callback)()>
    struct InterruptCallbackHelper2 {
        static void IRAM_ATTR callback(void* /*ptr*/) { Callback(); }
    };

    // There are a few special indices:
    //
    // - 0-N = GPIO pins. These map 1:1 to native GPIO pins. N is 64 here.
    // - 254 = undefined pin, maps to VoidPinDetail
    // - 255 = fault pin (if you use it, it gives an error)
    uint8_t _index;

    inline Pin(uint8_t index) : _index(index) {}

    static bool parse(String str, Pins::PinDetail*& detail, int& pinNumber);

public:
    using Capabilities = Pins::PinCapabilities;
    using Attr         = Pins::PinAttributes;

    static Pin UNDEFINED;
    static Pin ERROR;

    static const bool On = true;
    static const bool Off = false;

    static Pin  create(const String& str);
    static bool validate(const String& str);

    inline Pin() : _index(255) {}

    inline Pin(const Pin& o) = default;
    inline Pin(Pin&& o)      = default;

    inline Pin& operator=(const Pin& o) = default;
    inline Pin& operator=(Pin&& o) = default;

    // Some convenience operators:
    inline bool operator==(Pin o) const { return _index == _index; }
    inline bool operator!=(Pin o) const { return _index != _index; }

    inline uint8_t getNative(Capabilities expectedBehavior) const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        Assert(detail->capabilities().has(expectedBehavior), "Requested pin does not have the expected behavior.");
        return _index;
    }

    inline void write(bool value) const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        detail->write(value ? 1 : 0);
    }

    inline bool read() const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        return detail->read() != 0;
    }

    inline void setAttr(Attr attributes) const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        detail->setAttr(attributes);
    }

    inline void on() const { write(1); }
    inline void off() const { write(0); }

    // PWM

    inline bool initPWM(uint32_t frequency, uint32_t maxDuty) const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        return detail->initPWM(frequency, maxDuty);
    }

    // Returns actual frequency which might not be exactly the same as requested(nearest supported value)
    inline uint32_t getPWMFrequency() const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        return detail->getPWMFrequency();
    }

    // Returns actual maxDuty which might not be exactly the same as requested(nearest supported value)
    inline uint32_t getPWMMaxDuty() const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        return detail->getPWMMaxDuty();
    }

    inline void setPWMDuty(uint32_t duty) const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        return detail->setPWMDuty(duty);
    }

    // ISR handlers. Map methods on 'this' types.

    template <typename ThisType, void (ThisType::*Callback)()>
    void attachInterrupt(ThisType* arg, int mode) {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        detail->attachInterrupt(InterruptCallbackHelper<ThisType, Callback>::callback, arg, mode);
    }

    // Backward compatibility ISR handler:
    void attachInterrupt(void (*callback)(void*), int mode) const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        detail->attachInterrupt(callback, nullptr, mode);
    }

    void detachInterrupt() const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        detail->detachInterrupt();
    }

    // Other functions:
    Capabilities capabilities() const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        return detail->capabilities();
    }

    inline String name() const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        return detail->toString();
    }

    inline ~Pin() = default;
};
