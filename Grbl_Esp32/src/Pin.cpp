#include "Pin.h"
#include "I2SOut.h"

#include "Pins/PinOptionsParser.h"

// Pins:
#include "Pins/GPIOPinDetail.h"
#include "Pins/I2SPinDetail.h"

Pin Pin::Create(String str) {
    // Parse the definition: [GPIO].[pinNumber]:[attributes]

    auto nameStart = str.begin();
    auto idx       = nameStart;
    for (; idx != str.end() && *idx != '.' && *idx != ':'; ++idx) {
        *idx = char(::tolower(*idx));
    }
    String prefix = str.substring(0, idx - str.begin());

    if (idx != str.end()) {  // skip '.'
        ++idx;
    }
    Assert(idx != str.end(), "Incorrect pin definition.");

    int pinNumber = 0;
    for (; idx != str.end() && *idx >= '0' && *idx <= '9'; ++idx) {
        pinNumber = pinNumber * 10 + int(*idx - '0');
    }
    Assert(pinNumber >= 0 && pinNumber <= 255, "Pin number has to be between [0,255].");

    String options;
    if (idx != str.end()) {
        Assert(*idx == ':', "Pin definition attributes or EOF expected.");
        ++idx;

        options = str.substring(idx - str.begin());
    }

    // What would be a simple, practical way to parse the options? I figured, why not 
    // just use the C-style string, convert it to lower case, and change the separators 
    // into 'nul' tokens. We then pass the number of 'nul' tokens, and the first char*
    // which is pretty easy to parse.

    // Build this pin:
    Pins::PinDetail* pinImplementation = nullptr;

    // Build an options parser:
    Pins::PinOptionsParser parser(options.begin(), options.end());

    if (prefix == "gpio") {
        pinImplementation = new Pins::GPIOPinDetail(uint8_t(pinNumber), parser);
    }
#ifdef USE_I2S_OUT
    else if (prefix == "i2s") {
        pinImplementation = new Pins::I2SPinDetail(uint8_t(pinNumber), parser);
    }
#endif

    // Register:
    Pins::PinLookup::_instance.SetPin(uint8_t(pinNumber), pinImplementation);

    return Pin(uint8_t(pinNumber));
}

Pin Pin::ERROR(255);
Pin Pin::UNDEFINED(254);
