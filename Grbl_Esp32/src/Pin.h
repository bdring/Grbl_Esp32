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

#include "Pins/PinDetail.h"
#include "Pins/PinCapabilities.h"
#include "Pins/PinAttributes.h"
#include "Pins/VoidPinDetail.h"
#include "StringRange.h"

#include <Arduino.h>  // for IRAM_ATTR
#include <cstdint>
#include <cstring>
#include "Assert.h"

// TODO: ENABLE_CONTROL_SW_DEBOUNCE should end up here with a shared task.

// #define PIN_DEBUG  // Pin debugging. WILL spam you with a lot of data!

// Forward declarations:
class String;

extern Pins::PinDetail* undefinedPin;

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

    Pins::PinDetail* _detail;

    static bool parse(StringRange str, Pins::PinDetail*& detail);

public:
    using Capabilities = Pins::PinCapabilities;
    using Attr         = Pins::PinAttributes;

    //    inline Pin(Pins::PinDetail* detail) : _detail(detail) {}
    inline Pin() : _detail(undefinedPin) {}
    
    inline void define(Pins::PinDetail* implementation) { _detail = implementation; }

    static const bool On  = true;
    static const bool Off = false;

    inline static Pins::PinDetail* create(const char* str) { return create(StringRange(str)); };

    static Pins::PinDetail* create(const StringRange& str);
    static Pins::PinDetail* create(const String& str);
    static bool             validate(const String& str);

    inline Pin(const Pin& o) = delete;
    inline Pin(Pin&& o)      = default;

    inline Pin& operator=(const Pin& o) = delete;
    inline Pin& operator=(Pin&& o) = default;

    // Some convenience operators:
    inline bool operator==(const Pin& o) const { return _detail == o._detail; }
    inline bool operator!=(const Pin& o) const { return _detail != o._detail; }

    inline bool undefined() const { return _detail == undefinedPin; }
    inline bool defined() const { return !undefined(); }

    inline uint8_t getNative(Capabilities expectedBehavior) const {
        Assert(_detail->capabilities().has(expectedBehavior), "Requested pin does not have the expected behavior.");
        return _detail->_index;
    }

    inline void write(bool value) const { _detail->write(value); }

    inline bool read() const { return _detail->read() != 0; }

    inline void setAttr(Attr attributes) const { _detail->setAttr(attributes); }

    inline Attr getAttr() const { return _detail->getAttr(); }

    inline void on() const { write(1); }
    inline void off() const { write(0); }

    // ISR handlers. Map methods on 'this' types.

    template <typename ThisType, void (ThisType::*Callback)()>
    void attachInterrupt(ThisType* arg, int mode) {
        _detail->attachInterrupt(InterruptCallbackHelper<ThisType, Callback>::callback, arg, mode);
    }

    // Backward compatibility ISR handler:
    void attachInterrupt(void (*callback)(void*), int mode, void* arg = nullptr) const { _detail->attachInterrupt(callback, arg, mode); }

    void detachInterrupt() const { _detail->detachInterrupt(); }

    // Other functions:
    Capabilities capabilities() const { return _detail->capabilities(); }

    inline String name() const { return _detail->toString(); }

    void report(const char* legend);

    inline ~Pin() = default;
};
