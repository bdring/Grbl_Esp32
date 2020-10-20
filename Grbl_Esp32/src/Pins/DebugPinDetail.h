#pragma once

#include "PinDetail.h"

namespace Pins {
    class DebugPinDetail : public PinDetail {
        PinDetail* _implementation;

        int _lastEvent;
        int _eventCount;

        struct CallbackHandler {
            void (*callback)(void* arg);
            void*           argument;
            DebugPinDetail* _myPin;

            static void handle(void* arg);
        } _isrHandler;

        friend void CallbackHandler::handle(void* arg);

        bool shouldEvent();

    public:
        DebugPinDetail(PinDetail* implementation) : _implementation(implementation), _lastEvent(0), _eventCount(0) {}

        PinCapabilities capabilities() const override { return _implementation->capabilities(); }

        // I/O:
        void write(int high) override;
        int  read() override;
        void setAttr(PinAttributes value) override;

        // ISR's:
        void attachInterrupt(void (*callback)(void*), void* arg, int mode) override;
        void detachInterrupt() override;

        String toString() const override { return _implementation->toString(); }

        ~DebugPinDetail() override {}
    };

}
