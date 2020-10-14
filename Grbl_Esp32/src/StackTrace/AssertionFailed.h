#pragma once

#include "WString.h"

#ifdef ESP32
class AssertionFailed {
public:
    String stackTrace;

    AssertionFailed(String st) : stackTrace(st) {}

    static AssertionFailed create(const char* condition) {
        return create(condition, "Assertion failed");
    }
    static AssertionFailed create(const char* condition, const char* msg, ...);

};

#else
#    include <exception>

class AssertionFailed {
public:
    String stackTrace;

    static std::exception create(const char* condition) {
        return create(condition, "Assertion failed");
    }
    static std::exception create(const char* condition, const char* msg, ...);
};

#endif
