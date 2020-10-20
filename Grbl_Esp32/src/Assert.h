#pragma once

#undef Assert

#define Stringify(x) #x
#define Stringify2(x) Stringify(x)
#define Assert(condition, ...)                                                                                                             \
    {                                                                                                                                      \
        if (!(condition)) {                                                                                                                \
            const char* ch = #condition " (@line " Stringify2(__LINE__) ")";                                                               \
            throw ch;                                                                                                                      \
        }                                                                                                                                  \
    }
