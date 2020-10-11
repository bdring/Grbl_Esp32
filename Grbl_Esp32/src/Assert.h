#pragma once

#include "StackTrace/AssertionFailed.h"

class AssertionFailed;

#undef Assert

#define Assert(condition, ...)                                                                                                             \
    {                                                                                                                                      \
        if (!(condition)) {                                                                                                                \
            const char* ch = #condition;                                                                                                   \
            throw AssertionFailed(ch, ##__VA_ARGS__);                                                                                        \
        }                                                                                                                                  \
    }
