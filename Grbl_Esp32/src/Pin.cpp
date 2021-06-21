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

#include "Pin.h"

// Pins:
#include "Pins/PinOptionsParser.h"
#include "Pins/GPIOPinDetail.h"
#include "Pins/VoidPinDetail.h"
#include "Pins/I2SOPinDetail.h"
#include "Pins/ErrorPinDetail.h"

#ifdef ESP32
#    include "Grbl.h"  // serial output
#    define pin_error(...) error_serial(__VA_ARGS__)
#else
#    define pin_error(...)                                                                                                                 \
        {}
#endif

#ifdef DEBUG_PIN_DUMP
#    define pin_debug(...) debug_serial(__VA_ARGS__)
#    include "Pins/DebugPinDetail.h"
#else
#    define pin_debug(...)                                                                                                                 \
        {}
#endif

Pins::PinDetail* Pin::undefinedPin = new Pins::VoidPinDetail();
Pins::PinDetail* Pin::errorPin     = new Pins::ErrorPinDetail("unknown");

const char* Pin::parse(StringRange tmp, Pins::PinDetail*& pinImplementation) {
    String str = tmp.str();

    // Initialize pinImplementation first! Callers might want to delete it, and we don't want a random pointer.
    pinImplementation = nullptr;

    // Parse the definition: [GPIO].[pinNumber]:[attributes]

    // Skip whitespaces at the start
    auto nameStart = str.begin();
    for (; nameStart != str.end() && ::isspace(*nameStart); ++nameStart) {}

    if (nameStart == str.end()) {
        // Re-use undefined pins happens in 'create':
        pinImplementation = new Pins::VoidPinDetail();
        return nullptr;
    }

    auto idx = nameStart;
    for (; idx != str.end() && *idx != '.' && *idx != ':'; ++idx) {
        *idx = char(::tolower(*idx));
    }

    String prefix = str.substring(0, int(idx - str.begin()));

    if (idx != str.end()) {  // skip '.'
        ++idx;
    }

    int pinNumber = 0;
    if (prefix != "") {
        if (idx == str.end()) {
            // Incorrect pin definition.
            return "Pin definition is missing.";
        }

        for (int n = 0; idx != str.end() && n <= 4 && *idx >= '0' && *idx <= '9'; ++idx, ++n) {
            pinNumber = pinNumber * 10 + int(*idx - '0');
        }
        if ((idx != str.end() && *idx >= '0' && *idx <= '9') || (pinNumber < 0 || pinNumber > 253)) {
            // Pin number has to be between [0,253].
            return "Pin number has to be between [0,253>";
        }
    }

    while (idx != str.end() && ::isspace(*idx)) {
        ++idx;
    }

    String options;
    if (idx != str.end()) {
        if (*idx != ':') {
            // Pin definition attributes or EOF expected.
            return "Pin attributes (':') were expected.";
        }
        ++idx;

        options = str.substring(int(idx - str.begin()));
    }

    // What would be a simple, practical way to parse the options? I figured, why not
    // just use the C-style string, convert it to lower case, and change the separators
    // into 'nul' tokens. We then pass the number of 'nul' tokens, and the first char*
    // which is pretty easy to parse.

    // Build an options parser:
    Pins::PinOptionsParser parser(options.begin(), options.end());

    pin_debug("Attempting to set up pin: %s, index %d\r\n", prefix.c_str(), int(pinNumber));

    // Build this pin:
    if (prefix == "gpio") {
        pinImplementation = new Pins::GPIOPinDetail(uint8_t(pinNumber), parser);
    }
#ifdef ESP32
    if (prefix == "i2so") {
        pinImplementation = new Pins::I2SOPinDetail(uint8_t(pinNumber), parser);
    }
#endif
    if (prefix == "void") {
        // Note: having multiple void pins has its uses for debugging.
        pinImplementation = new Pins::VoidPinDetail();
    }

    if (pinImplementation == nullptr) {
        pin_error("ERR: Unknown prefix: \"%s\"\r\n", prefix.c_str());
        return "Unknown pin prefix";
    } else {
#if DEBUG_PIN_DUMP
        pinImplementation = new Pins::DebugPinDetail(pinImplementation);
#endif

        return nullptr;
    }
}

Pin Pin::create(const String& str) {
    return create(StringRange(str));
}

Pin Pin::create(const StringRange& str) {
    Pins::PinDetail* pinImplementation = nullptr;
    try {
        pin_debug("Setting up pin: [%s]\r\n", str.str().c_str());

        const char* err = parse(str, pinImplementation);
        if (err) {
            if (pinImplementation) {
                delete pinImplementation;
            }

            pin_debug("ERR: Setting up pin: '%s' failed: %s.", str.str().c_str(), err);
            return Pin(new Pins::ErrorPinDetail(str.str()));
        } else {
            return Pin(pinImplementation);
        }
    } catch (const AssertionFailed& ex) {  // We shouldn't get here under normal circumstances.
        pin_error("ERR: Setting up pin [%s] failed. Details: %s", str.str().c_str(), ex.what());
        (void)ex;  // Get rid of compiler warning

        return Pin(new Pins::ErrorPinDetail(str.str()));
    }
}

bool Pin::validate(const String& str) {
    Pins::PinDetail* pinImplementation;

    auto valid = parse(str, pinImplementation);
    if (pinImplementation) {
        delete pinImplementation;
    }

    return valid;
}

void Pin::report(const char* legend) {
    if (defined()) {
#if ESP32
        info_serial("%s on %s", legend, name().c_str());
#endif
    }
}

Pin::~Pin() {
    if (_detail != undefinedPin && _detail != errorPin) {
        delete _detail;
    }
}
