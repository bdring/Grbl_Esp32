#include "StepStick.h"

namespace Motors {
    void StepStick::init() {
        // If they are not 'undefined', set them as 'on'.
        _MS1.setAttr(Pin::Attr::Output | Pin::Attr::InitialOn);
        _MS2.setAttr(Pin::Attr::Output | Pin::Attr::InitialOn);
        _MS3.setAttr(Pin::Attr::Output | Pin::Attr::InitialOn);

        StandardStepper::init();
    }

    // Configuration handlers:
    void StepStick::validate() const { StandardStepper::validate(); }

    void StepStick::handle(Configuration::HandlerBase& handler) {
        StandardStepper::handle(handler);

        handler.handle("ms1", _MS1);
        handler.handle("ms2", _MS2);
        handler.handle("ms3", _MS3);
        handler.handle("reset", _Reset);
    }

    void StepStick::afterParse() {
        if (!_Reset.undefined()) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Using StepStick Mode");

            // !RESET pin on steppers  (MISO On Schematic)
            _Reset.setAttr(Pin::Attr::Output | Pin::Attr::InitialOn);
            _Reset.on();
        }
    }

    // Name of the configurable. Must match the name registered in the cpp file.
    const char* StepStick::name() const { return "stepstick"; }

    // Configuration registration
    namespace {
        MotorFactory::InstanceBuilder<StepStick> registration("stepstick");
    }
}
