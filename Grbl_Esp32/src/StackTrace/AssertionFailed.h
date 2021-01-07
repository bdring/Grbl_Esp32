#pragma once

#include "WString.h"

#ifdef ESP32
class AssertionFailed {
public:
    String stackTrace;
    String error;

    AssertionFailed(String st, String err) : stackTrace(st), error(err) {}

    static AssertionFailed create(const char* condition) {
        return create(condition, "Unknown error (assertion failed).");
    }
    static AssertionFailed create(const char* condition, const char* msg, ...);

};

#else
#    include <exception>

class AssertionFailed {
public:
    String stackTrace;

    static std::exception create(const char* condition) {
        return create(condition, "Unknown error (assertion failed).");
    }
    static std::exception create(const char* condition, const char* msg, ...);
};

#endif
