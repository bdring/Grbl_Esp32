#pragma once

class Pin;  // Forward declaration

namespace Pins {
    // Pin options are passed as PinOption object. This is a simple C++ forward iterator,
    // which will implicitly convert pin options to lower case, so you can simply do
    // stuff like this:
    //
    // for (auto it : options) {
    //   const char* currentOption = it();
    //   ...
    //   if (currentOption.is("pu")) { /* configure pull up */ }
    //   ...
    // }
    //
    // This is a very light-weight parser for pin options, configured as 'pu:high:etc'
    // (full syntax f.ex.: gpio.12:pu:high)

    class PinOptionsParser;

    class PinOption {
        friend class PinOptionsParser;

        char*       _start;
        const char* _end;

        PinOption(char* start, const char* end);

    public:
        inline const char* operator()() const { return _start; }

        bool is(const char* option) const;

        // Iterator support:
        inline PinOption const* operator->() const { return this; }
        inline PinOption        operator*() const { return *this; }
        PinOption               operator++() const;

        bool operator==(const PinOption& o) const { return _start == o._start; }
        bool operator!=(const PinOption& o) const { return _start != o._start; }
    };

    // Options parser. This basically parses the options passed to the Pin class. Destroys
    // the original options, and passes the options as lower case items to the enumerator.
    // For the lazy people that want safe, reliable, easy parsing :-)
    class PinOptionsParser {
        char* _buffer;
        char* _bufferEnd;

    public:
        PinOptionsParser(char* buffer, char* endBuffer);

        inline PinOption begin() const { return PinOption(_buffer, _bufferEnd); }
        inline PinOption end() const { return PinOption(_bufferEnd, _bufferEnd); }
    };
}
