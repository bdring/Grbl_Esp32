#include "Pin.h"
#include "Grbl.h"

// Pins:
#include "Pins/PinOptionsParser.h"
#include "Pins/VoidPinDetail.h"
#include "Pins/GPIOPinDetail.h"
#include "Pins/I2SPinDetail.h"
#ifdef PIN_DEBUG
#    include "Pins/DebugPinDetail.h"
#endif

bool Pin::parse(String str, Pins::PinDetail*& pinImplementation, int& pinNumber) {
    // Initialize pinImplementation first! Callers might want to delete it, and we don't want a random pointer.
    pinImplementation = nullptr;

    if (str == "") {
        pinImplementation = new Pins::VoidPinDetail();
        return pinImplementation;
    }

    // Parse the definition: [GPIO].[pinNumber]:[attributes]
    auto nameStart = str.begin();
    auto idx       = nameStart;
    for (; idx != str.end() && *idx != '.' && *idx != ':'; ++idx) {
        *idx = char(::tolower(*idx));
    }
    String prefix = str.substring(0, int(idx - str.begin()));

    if (idx != str.end()) {  // skip '.'
        ++idx;
    }

    if (prefix != "") {
        if (idx == str.end()) {
            // Incorrect pin definition.
            return false;
        }

        pinNumber = 0;
        for (; idx != str.end() && *idx >= '0' && *idx <= '9'; ++idx) {
            pinNumber = pinNumber * 10 + int(*idx - '0');
        }
        if (pinNumber < 0 || pinNumber > 253) {
            // Pin number has to be between [0,253].
            return false;
        }
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

    // Build this pin:

    try {
        if (prefix == "gpio") {
            pinImplementation = new Pins::GPIOPinDetail(uint8_t(pinNumber), parser);
        } else if (prefix == "i2s") {
#ifdef ESP_32
            pinImplementation = new Pins::I2SPinDetail(uint8_t(pinNumber), parser);
#endif
        } else {
#ifdef PIN_DEBUG
            pinImplementation = new Pins::DebugPinDetail(pinImplementation);
#endif
        }
    } catch (const std::exception& e) {
        grbl_sendf(CLIENT_SERIAL, "%s\r\n", e.what());
        pinImplementation = new Pins::VoidPinDetail();
        return false;
    }
    return pinImplementation;
}

Pin Pin::create(const String& str) {
    Pins::PinDetail* pinImplementation;
    int              pinNumber;

    Assert(parse(str, pinImplementation, pinNumber), "Pin definition is invalid.");

    // Register:
    Pins::PinLookup::_instance.SetPin(uint8_t(pinNumber), pinImplementation);

    return Pin(uint8_t(pinNumber));
}

bool Pin::validate(const String& str) {
    Pins::PinDetail* pinImplementation;
    int              pinNumber;

    auto valid = parse(str, pinImplementation, pinNumber);

    delete pinImplementation;
    return valid;
}

Pin Pin::ERROR(255);
Pin Pin::UNDEFINED(254);
