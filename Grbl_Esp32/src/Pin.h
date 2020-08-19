#pragma once

#include <cstdint>
#include <cstring>
#include <string>

class Pin;

namespace Pins {
    class PinDetail {
    private:
        friend class Pin;
        int referenceCount_ = 0;

    public:
        PinDetail()                   = default;
        PinDetail(const PinDetail& o) = delete;
        PinDetail(PinDetail&& o)      = delete;
        PinDetail& operator=(const PinDetail& o) = delete;
        PinDetail& operator=(PinDetail&& o) = delete;

        virtual void Write(bool high)    = 0;
        virtual int  Read()              = 0;
        virtual void Mode(uint8_t value) = 0;

        virtual String ToString() = 0;

        virtual ~PinDetail() {}
    };
}

class Pin {
    Pins::PinDetail* detail = nullptr;

    inline Pin(Pins::PinDetail* detail) {
        this->detail = detail;
        ++detail->referenceCount_;
    }

    static uint8_t ParseUI8(const char* str, const char* end) {
        // Parses while ignoring '_', '(' and ')'. Kinda retains backward compatibility
        uint8_t value = 0;
        for (; str != end; ++str) {
            if (*str >= '0' && *str <= '9') {
                value = value * 10 + uint8_t(*str - '0');
            } else if (*str == '(' || *str == ')' || *str == '_') {
            } else {
                // Some error.
            }
        }
        return value;
    }

public:
    inline Pin(const Pin& o) {}

    inline Pin(Pin&& o) {
        // Swap
        auto tmp = o.detail;
        o.detail = detail;
        detail   = tmp;
    }

    inline Pin& operator=(const Pin& o) {
        if (this != &o) {
            if (o.detail != nullptr) {
                o.detail->referenceCount_++;
            }

            this->~Pin();

            this->detail = o.detail;
        }
        return *this;
    }

    inline Pin& operator=(Pin&& o) {
        // Swap
        auto tmp = o.detail;
        o.detail = detail;
        detail   = tmp;
    }

    inline void Write(bool high) {
        if (detail) {
            detail->Write(high);
        } else {
            // ???
        }
    }

    inline int Read() {
        if (detail) {
            return detail->Read();
        } else {
            // ???
            return -1;
        }
    }

    inline void Mode(uint8_t value) {
        if (detail) {
            detail->Mode(value);
        }
    }

    inline ~Pin() {
        if (detail != nullptr) {
            --detail->referenceCount_;
            if (detail->referenceCount_ == 0) {
                delete detail;
            }
        }
    }

    static Pin Create(String str);
};
