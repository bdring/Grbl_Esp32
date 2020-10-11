#pragma once

#include "WString.h"

#ifdef ESP32
class AssertionFailed {
public:
    String stackTrace;

    AssertionFailed(const char* condition) : AssertionFailed(condition, "Assertion failed.") {}
    AssertionFailed(const char* condition, const char* msg);
};

#else
#    include <exception>

class AssertionFailed : std::exception {
public:
    String stackTrace;

    AssertionFailed(const char* condition) : AssertionFailed(condition, "Assertion failed.") {}
    AssertionFailed(const char* condition, const char* msg);
};

#endif
