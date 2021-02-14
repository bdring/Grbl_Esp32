#pragma once

#include "WString.h"

#ifdef ESP32
class AssertionFailed {
public:
    String stackTrace;
    String msg;

    AssertionFailed(String st) : stackTrace(st) {}

    static AssertionFailed create(const char* condition) {
        return create(condition, "Assertion failed");
    }
    static AssertionFailed create(const char* condition, const char* msg, ...);

    const char* what() const { return msg.c_str(); }
};

#else
#    include <exception>

class AssertionFailed {
public:
    String stackTrace;
    String msg;

    static std::exception create(const char* condition) {
        return create(condition, "Assertion failed");
    }
    static std::exception create(const char* condition, const char* msg, ...);

    const char* what() const { return msg.c_str(); }
};

#endif
