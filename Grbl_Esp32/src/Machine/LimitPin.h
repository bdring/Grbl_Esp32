#pragma once

#include "../Pin.h"
#include "../System.h"  // MotorMask

#include <esp_attr.h>  // IRAM_ATTR

namespace Machine {
    class LimitPin {
    private:
        int _axis;
        int _gang;

        bool      _value   = 0;
        MotorMask _bitmask = 0;

        // _pHardLimits is a reference so the shared variable at the
        // Endstops level can be changed at runtime to control the
        // limit behavior dynamically.
        bool&               _pHardLimits;
        volatile MotorMask* _posLimits = nullptr;
        volatile MotorMask* _negLimits = nullptr;

        void IRAM_ATTR handleISR();

    public:
        LimitPin(Pin& pin, int axis, int gang, int direction, bool& phardLimits);

        Pin& _pin;

        String _legend;

        void init();
        bool get() { return _value; }

        void expandMask();

        ~LimitPin();
    };
}
