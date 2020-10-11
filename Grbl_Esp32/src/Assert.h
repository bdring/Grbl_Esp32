#pragma once

#include "StackTrace/AssertionFailed.h"

#define Assert(condition, msg)                                                                                                             \
    {                                                                                                                                      \
        if (!(condition)) {                                                                                                                \
            throw AssertionFailed(#condition, msg);                                                                                        \
        }                                                                                                                                  \
    }
