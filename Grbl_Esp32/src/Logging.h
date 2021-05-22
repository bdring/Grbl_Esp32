#pragma once

#include "SimpleOutputStream.h"

// How to use logging? Well, the basics are pretty simple:
//
// - The syntax is like standard iostream's.
// - It is simplified though, so no ios or iomanip. But should be sufficient.
// - But, you wrap it in an 'info', 'debug', 'warn', 'error' or 'fatal'.
//
// The streams here ensure the data goes where it belongs, without too much
// buffer space being wasted.
//
// Example:
//
// log_info("Twelve is written as " << 12 << ", isn't it");

class DebugStream : public SimpleOutputStream {
public:
    DebugStream(const char* name);
    void add(char c) override;
    ~DebugStream();
};

#include "StringStream.h"

// Note: these '{'..'}' scopes are here for a reason: the destructor should flush.
#define log_debug(x)                                                                                                                       \
    {                                                                                                                                      \
        DebugStream ss("DBG");                                                                                                             \
        ss << x;                                                                                                                           \
    }

#define log_info(x)                                                                                                                        \
    {                                                                                                                                      \
        DebugStream ss("INFO");                                                                                                            \
        ss << x;                                                                                                                           \
    }

#define log_warn(x)                                                                                                                        \
    {                                                                                                                                      \
        DebugStream ss("WARN");                                                                                                            \
        ss << x;                                                                                                                           \
    }

#define log_error(x)                                                                                                                       \
    {                                                                                                                                      \
        DebugStream ss("ERR");                                                                                                             \
        ss << x;                                                                                                                           \
    }

#define log_fatal(x)                                                                                                                       \
    {                                                                                                                                      \
        DebugStream ss("FATAL");                                                                                                           \
        ss << x;                                                                                                                           \
        Assert(false, "A fatal error occurred.");                                                                                          \
    }
