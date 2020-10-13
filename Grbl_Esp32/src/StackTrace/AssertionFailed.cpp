#include "AssertionFailed.h"

#ifdef ESP32
#    ifdef UNIT_TEST

#        include "debug_helpers.h"
#        include "WString.h"

AssertionFailed::AssertionFailed(const char* condition, const char* msg) {
    String st = condition;
    st += ": ";
    st += msg;
    st += " at: ";
    st += esp_backtrace_print(10);

    stackTrace = st;
}

#    else

AssertionFailed::AssertionFailed(const char* condition, const char* msg) {
    String st = "\r\nError ";
    st += condition;
    st += " failed: ";
    st += msg;

    stackTrace = st;
}

#    endif

#else

#    include <iostream>
#    include <string>
#    include <sstream>
#    include "WString.h"

extern void DumpStackTrace(std::ostringstream& builder);

String stackTrace;

AssertionFailed::AssertionFailed(const char* condition, const char* msg) {
    static std::string container;  // Exception data _must_ be stored in a static string!
    std::ostringstream oss;
    oss << std::endl;
    oss << "Error: " << std::endl;
    DumpStackTrace(oss);

    container = oss.str();
}

#endif
