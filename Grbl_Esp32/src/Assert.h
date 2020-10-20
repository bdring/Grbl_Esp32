#pragma once

extern void grbl_msg_sendf(uint8_t client, MsgLevel level, const char* format, ...);

#undef Assert

#define Stringify(x) #x
#define Stringify2(x) Stringify(x)
#define Assert(condition, ...)                                                                                                             \
    {                                                                                                                                      \
        if (!(condition)) {                                                                                                                \
            const char* ch = #condition " (@line " Stringify2(__LINE__) ")";                                                               \
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Error, ch);                                                                            \
            throw ch;                                                                                                                      \
        }                                                                                                                                  \
    }
