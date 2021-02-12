#include "Logging.h"

#ifndef ESP32

#include <iostream>

DebugStream::DebugStream(const char* name) {
    std::cout << '[' << name << ": ";
}
void DebugStream::add(char c)
{
    std::cout << c;
}

DebugStream::~DebugStream() { std::cout << ']' << std::endl; }

#else

DebugStream::DebugStream(const char* name) {
    Serial.print("[");
    Serial.print(name);
    Serial.print(": ");
}

void DebugStream::add(char c) { Serial.print(c); }

DebugStream::~DebugStream() { Serial.println("]"); }

#endif
