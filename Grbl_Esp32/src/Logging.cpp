#include "Logging.h"

#ifndef ESP32

#    include <iostream>

DebugStream::DebugStream(const char* name) {
    std::cout << '[' << name << ": ";
}
void DebugStream::add(char c) {
    std::cout << c;
}

DebugStream::~DebugStream() {
    std::cout << ']' << std::endl;
}

#else

#    include "Uart.h"

DebugStream::DebugStream(const char* name) {
    Uart0.print("[");
    Uart0.print(name);
    Uart0.print(": ");
}

void DebugStream::add(char c) {
    Uart0.print(c);
}

DebugStream::~DebugStream() {
    Uart0.println("]");
}

#endif
