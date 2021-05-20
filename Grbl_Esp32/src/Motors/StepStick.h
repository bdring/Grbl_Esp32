#pragma once

#include "StandardStepper.h"

namespace Motors {
    class StepStick : public StandardStepper {
        Pin _MS1;
        Pin _MS2;
        Pin _MS3;
        Pin _Reset;

    public:
        StepStick() = default;

        void init() override;

        // Configuration handlers:
        void validate() const override;
        void handle(Configuration::HandlerBase& handler) override;

        void afterParse() override;

        // Name of the configurable. Must match the name registered in the cpp file.
        const char* name() const override;

        ~StepStick() = default;
    };
}
