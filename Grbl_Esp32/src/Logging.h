#pragma once

#include "SimpleOutputStream.h"

class DebugStream : public SimpleOutputStream
{
public:
    DebugStream(const char* name);
    void add(char c) override;
    ~DebugStream();
};

#include "StringStream.h"

#define debug(x) { DebugStream ss("DBG "); ss << x; }
#define info(x)  { DebugStream ss("INFO"); ss << x; }
#define warn(x)  { DebugStream ss("WARN"); ss << x; }
#define error(x) { DebugStream ss("ERR "); ss << x; }

