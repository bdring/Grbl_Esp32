#pragma once

#define Assert(condition, msg)                                                                                                             \
    {                                                                                                                                      \
        if (!(condition)) {                                                                                                                \
            throw msg;                                                                                                                     \
        }                                                                                                                                  \
    }
