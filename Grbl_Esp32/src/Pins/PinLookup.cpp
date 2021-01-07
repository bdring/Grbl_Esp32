#include "PinLookup.h"
#include "PinAttributes.h"

#include "VoidPinDetail.h"
#include "ErrorPinDetail.h"
#include "GPIOPinDetail.h"
#include "PinOptionsParser.h"

#include <Arduino.h>

namespace Pins {
    PinLookup PinLookup::_instance;

    PinLookup::PinLookup() {
        // Initialize pins:
        memset(_pins, 0, sizeof(_pins));
        ResetAllPins();
    }

    void PinLookup::ResetAllPins() {
        auto& inst = _instance;

        // Delete all pins in use:
        for (auto i = 0; i <= 255; ++i) {
            if (inst._pins[i] != nullptr) {
                delete inst._pins[i];
                inst._pins[i] = nullptr;
            }
        }

        // There are a few special indices, which we always have
        // to initialize:
        //
        // - 1 = UART0 TX
        // - 3 = UART0 RX
        // - 254 = undefined pin, maps to VoidPinDetail
        // - 255 = fault pin (if you use it, it gives an error)

        char             stub;
        PinOptionsParser parser(&stub, &stub);

        // Set up pins for Serial:
        inst._pins[1] = new Pins::GPIOPinDetail(1, parser);
        inst._pins[3] = new Pins::GPIOPinDetail(3, parser);

        // Setup void and error pins:
        inst._pins[254] = new Pins::VoidPinDetail(parser);
        inst._pins[255] = new Pins::ErrorPinDetail(parser);
    }
}

void IRAM_ATTR digitalWrite(uint8_t pin, uint8_t val) {
    Pins::PinLookup::_instance.GetPin(pin)->write(val);
}

void IRAM_ATTR pinMode(uint8_t pin, uint8_t mode) {
    Pins::PinAttributes attr = Pins::PinAttributes::None;
    if ((mode & OUTPUT) == OUTPUT) {
        attr = attr | Pins::PinAttributes::Output;
    }
    if ((mode & INPUT) == INPUT) {
        attr = attr | Pins::PinAttributes::Input;
    }
    if ((mode & PULLUP) == PULLUP) {
        attr = attr | Pins::PinAttributes::PullUp;
    }
    if ((mode & PULLDOWN) == PULLDOWN) {
        attr = attr | Pins::PinAttributes::PullDown;
    }

    Pins::PinLookup::_instance.GetPin(pin)->setAttr(attr);
}

int IRAM_ATTR digitalRead(uint8_t pin) {
    return Pins::PinLookup::_instance.GetPin(pin)->read();
}
