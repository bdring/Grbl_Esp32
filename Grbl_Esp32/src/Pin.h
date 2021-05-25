/*
    Part of Grbl_ESP32
    2021 -  Stefan de Bruijn

    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Grbl_ESP32 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Pins/PinLookup.h"
#include "Pins/PinDetail.h"
#include "Pins/PinCapabilities.h"
#include "Pins/PinAttributes.h"
#include "StringRange.h"

#include <Arduino.h>  // for IRAM_ATTR
#include <cstdint>
#include <cstring>

// #define PIN_DEBUG  // Pin debugging. WILL spam you with a lot of data!

// Forward declarations:
class String;

class Pin {
    // Helper for handling callbacks and mapping them to the proper class:
    template <typename ThisType, void (ThisType::*Callback)()>
    struct InterruptCallbackHelper {
        static void IRAM_ATTR callback(void* ptr) { (static_cast<ThisType*>(ptr)->*Callback)(); }
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

    static bool parse(StringRange str, Pins::PinDetail*& detail);

public:
    using Capabilities = Pins::PinCapabilities;
    using Attr         = Pins::PinAttributes;

    static Pin UNDEFINED;
    static Pin ERROR;

    static const bool On  = true;
    static const bool Off = false;

    inline static Pin create(const char* str) { return create(StringRange(str)); }

    static Pin  create(const StringRange& str);
    static Pin  create(const String& str);
    static bool validate(const String& str);

    inline Pin() : _index(254) {}  // Default to UNDEFINED

    inline Pin(const Pin& o) = default;
    inline Pin(Pin&& o)      = default;

    inline Pin& operator=(const Pin& o) = default;
    inline Pin& operator=(Pin&& o) = default;

    // Some convenience operators:
    inline bool operator==(const Pin& o) const { return _index == o._index; }
    inline bool operator!=(const Pin& o) const { return _index != o._index; }

    inline bool undefined() const { return (*this) == UNDEFINED; }
    inline bool defined() const { return (*this) != UNDEFINED; }

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

    inline Attr getAttr() const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        return detail->getAttr();
    }

    inline void on() const { write(1); }
    inline void off() const { write(0); }

    // ISR handlers. Map methods on 'this' types.

    template <typename ThisType, void (ThisType::*Callback)()>
    void attachInterrupt(ThisType* arg, int mode) {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        detail->attachInterrupt(InterruptCallbackHelper<ThisType, Callback>::callback, arg, mode);
    }

    // Backward compatibility ISR handler:
    void attachInterrupt(void (*callback)(void*), int mode, void* arg = nullptr) const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        detail->attachInterrupt(callback, arg, mode);
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
