#pragma once

#ifdef ESP32
#    define Assert(condition, msg)                                                                                                         \
        {                                                                                                                                  \
            if (!(condition)) {                                                                                                            \
                throw msg;                                                                                                                 \
            }                                                                                                                              \
        }
#else
#    include <exception>

extern std::exception CreateException(const char* condition, const char* msg);

#    define Assert(condition, msg)                                                                                                         \
        {                                                                                                                                  \
            if (!(condition)) {                                                                                                            \
                throw CreateException(#condition, msg);                                                                                \
            }                                                                                                                              \
        }
#endif
