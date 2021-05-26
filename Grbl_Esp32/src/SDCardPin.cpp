#include "Grbl.h"
#include "SDCardPin.h"

void SDCardPin::init() {
    static bool init_message = true;  // used to show messages only once.

    if (init_message) {
        if (_cardDetect.defined()) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "SD Card Detect on pin %s", _cardDetect.name().c_str());
        }
        init_message = false;
    }

    _cardDetect.setAttr(Pin::Attr::Output);
}

void SDCardPin::validate() const {}

void SDCardPin::handle(Configuration::HandlerBase& handler) {
    handler.handle("card_detect", _cardDetect);
}
