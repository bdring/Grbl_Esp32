#pragma once

#include <cstring>

#include "StringRange.h"
#include "Pin.h"

class SimpleOutputStream
{
    static char* intToBuf(int value, char* dst);

public:
    SimpleOutputStream() = default;

    SimpleOutputStream(const SimpleOutputStream& o) = delete;
    SimpleOutputStream(SimpleOutputStream&& o) = delete;

    SimpleOutputStream& operator=(const SimpleOutputStream& o) = delete;
    SimpleOutputStream& operator=(SimpleOutputStream&& o) = delete;

    virtual void add(char c) = 0;
    virtual void flush() {}

    void add(const char* s);
    void add(int value);
    void add(double value, int numberDigits, int precision);
    void add(StringRange range);
    void add(const Pin& pin);

    virtual ~SimpleOutputStream() {
    }
};

inline SimpleOutputStream& operator<<(SimpleOutputStream& lhs, char c) {
    lhs.add(c);
    return lhs;
}

inline SimpleOutputStream& operator<<(SimpleOutputStream& lhs, const char* v) {
    lhs.add(v);
    return lhs;
}

inline SimpleOutputStream& operator<<(SimpleOutputStream& lhs, int v) {
    lhs.add(v);
    return lhs;
}

inline SimpleOutputStream& operator<<(SimpleOutputStream& lhs, double v) {
    lhs.add(v, 4, 3);
    return lhs;
}

inline SimpleOutputStream& operator<<(SimpleOutputStream& lhs, StringRange v) {
    lhs.add(v);
    return lhs;
}

inline SimpleOutputStream& operator<<(SimpleOutputStream& lhs, const Pin& v) {
    lhs.add(v);
    return lhs;
}