#include "SimpleOutputStream.h"

#include <cstring>

char* SimpleOutputStream::intToBuf(int value, char* dst)
{
#ifdef ESP32
    return itoa(value, dst, 10);
#else 
    _itoa_s(value, dst, 10, 10);
    return dst + strlen(dst);
#endif
}

void SimpleOutputStream::add(const char* s) {
    for (; *s; ++s) { add(*s); }
}

void SimpleOutputStream::add(int value) {
    char buf[10];
    intToBuf(value, buf);
    add(buf);
}

void SimpleOutputStream::add(double value, int numberDigits, int precision)
{
    if (isnan(value)) {
        add("NaN");
    }
    else if (isinf(value)) {
        add("Inf");
    }

    char buf[30]; // that's already quite big
    char fmt[10];
    fmt[0] = '%';
    fmt[1] = '0';

    char* next = intToBuf(numberDigits, fmt + 2);
    *next++ = '.';
    intToBuf(precision, next);

    snprintf(buf, sizeof(buf) - 1, fmt, value);
    add(buf);
}

void SimpleOutputStream::add(StringRange range)
{
    for (auto ch : range) { add(ch); }
}

void SimpleOutputStream::add(const Pin& pin)
{
    add(pin.str());
}
