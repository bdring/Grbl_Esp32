#pragma once

#include "StackTrace/AssertionFailed.h"

#define Assert(condition, msg)                                                                                                             \
    {                                                                                                                                      \
        if (!(condition)) {                                                                                                                \
            throw AssertionFailed(#condition, msg);                                                                                        \
        }                                                                                                                                  \
    }

#define Assert(condition)                                                                                                                  \
    {                                                                                                                                      \
        if (!(condition)) {                                                                                                                \
            throw AssertionFailed(#condition, "Assertion failed.");                                                                        \
        }                                                                                                                                  \
    }
