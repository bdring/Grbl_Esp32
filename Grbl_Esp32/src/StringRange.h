#pragma once

#include <cstring>

#ifndef ESP32
#    include <string>
#else
#    include "WString.h"
#endif

class StringRange {
    const char* start_;
    const char* end_;

public:
    StringRange() : start_(nullptr), end_(nullptr) {}

    StringRange(const char* str) : start_(str), end_(str + strlen(str)) {}

    StringRange(const char* start, const char* end) : start_(start), end_(end) {}

    StringRange(const StringRange& o) = default;
    StringRange(StringRange&& o)      = default;

    StringRange(const String& str) : StringRange(str.begin(), str.end()) {}

    StringRange& operator=(const StringRange& o) = default;
    StringRange& operator=(StringRange&& o) = default;

    int find(char c) const {
        const char* s = start_;
        for (; s != end_ && *s != c; ++s) {}
        return (*s) ? (s - start_) : -1;
    }

    StringRange subString(int index, int length) const {
        const char* s = start_ + index;
        if (s > end_) {
            s = end_;
        }
        const char* e = s + length;
        if (e > end_) {
            e = end_;
        }
        return StringRange(s, e);
    }

    bool equals(const StringRange& o) const {
        auto l = length();
        return l == o.length() && !strncmp(start_, o.start_, l);
    }

    bool equals(const char* o) const {
        const char* c  = start_;
        const char* oc = o;
        for (; *c != '\0' && *oc != '\0' && *c == *oc; ++c, ++oc) {}
        return c == end_ && *oc == '\0';
    }

    int length() const { return end_ - start_; }

    // Iterator support:
    const char* begin() const { return start_; }
    const char* end() const { return end_; }

#ifndef ESP32
    std::string str() const { return std::string(begin(), end()); }
#else
    String str() const {
        // TODO: Check if we can eliminate this function. I'm pretty sure we can.
        auto len = length();
        char* buf = new char[len + 1];
        memcpy(buf, begin(), len);
        buf[len] = 0;
        String tmp(buf);
        delete[] buf;
        return tmp;
    }
#endif
};
