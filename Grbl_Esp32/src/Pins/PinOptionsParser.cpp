#include "PinOptionsParser.h"

#include <cstring>

namespace Pins {
    PinOption::PinOption(char* start, const char* end) : _start(start), _end(end) {}

    bool PinOption::is(const char* option) const { return !::strcmp(option, _start); }

    PinOption PinOption ::operator++() {
        if (_start != _end) {
            auto newStart = _start + ::strlen(_start);  // to the \0
            if (newStart != _end) {                     // and 1 past it if we're not at the end
                ++newStart;
            }
            _start = newStart;
        }
        return *this;
    }

    PinOptionsParser::PinOptionsParser(char* buffer, char* bufferEnd) : _buffer(buffer), _bufferEnd(bufferEnd) {
        // Do the actual parsing:
        for (auto i = buffer; i != bufferEnd; ++i) {
            if (*i == ':' || *i == ';') {
                *i = '\0';
            } else if (*i >= 'A' && *i <= 'Z') {  // where did cstring->tolower go? Anyways, here goes:
                *i = char(*i + 32);
            }
        }
    }
}
