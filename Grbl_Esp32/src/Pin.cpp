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
#    include "Grbl.h"  // grbl_sendf
#    define pin_error(...) grbl_sendf(CLIENT_ALL, __VA_ARGS__)
#else
#    define pin_error(...)                                                                                                                 \
        {}
#endif

#if defined PIN_DEBUG
#    define pin_debug(...) pin_error(__VA_ARGS__)
#else
#    define pin_debug(...)                                                                                                                 \
        {}
#endif

Pins::PinDetail* Pin::undefinedPin = new Pins::VoidPinDetail();
Pins::PinDetail* Pin::errorPin     = new Pins::ErrorPinDetail();

bool Pin::parse(StringRange tmp, Pins::PinDetail*& pinImplementation) {
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
        return true;
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
            return false;
        }

        for (; idx != str.end() && *idx >= '0' && *idx <= '9'; ++idx) {
            pinNumber = pinNumber * 10 + int(*idx - '0');
        }
        if (pinNumber < 0 || pinNumber > 253) {
            // Pin number has to be between [0,253].
            return false;
        }
    }

    while (idx != str.end() && ::isspace(*idx)) {
        ++idx;
    }

    String options;
    if (idx != str.end()) {
        if (*idx != ':') {
            // Pin definition attributes or EOF expected.
            return false;
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
        return true;
    }
    if (prefix == "i2so") {
#ifdef ESP32
        pinImplementation = new Pins::I2SOPinDetail(uint8_t(254), parser);
        return true;
#endif
    }
    if (prefix == "void") {
        // Note: having multiple void pins has its uses for debugging.
        pinImplementation = new Pins::VoidPinDetail();
        return true;
    }
    pin_error("Unknown prefix: \"%s\"\r\n", prefix.c_str());
    return false;
}

Pin Pin::create(const String& str) {
    return create(StringRange(str));
}

Pin Pin::create(const StringRange& str) {
    Pins::PinDetail* pinImplementation = nullptr;
    try {
        pin_debug("Setting up pin: [%s]\r\n", str.str().c_str());

        if (!parse(str, pinImplementation)) {
            pin_debug("Setting up pin: '%s' failed.", str.str().c_str());
        }

        return Pin(pinImplementation);
    } catch (const AssertionFailed& ex) {  // We shouldn't get here under normal circumstances.
        pin_error("Setting up pin failed. Details: %s\r\n", ex.what());
        (void)ex;  // Get rid of compiler warning

        return Pin(pinImplementation);
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
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "%s on %s", legend, name().c_str());
#endif
    }
}

Pin::~Pin() {
    if (_detail != undefinedPin && _detail != errorPin) {
        delete _detail;
    }
}
