#pragma once

#include "WString.h"

struct AssertionFailed
{
    String stackTrace;

    AssertionFailed(const char* condition, const char* msg);
};
