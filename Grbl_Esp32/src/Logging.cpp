#include "Logging.h"

#ifndef ESP32

#    include <iostream>

DebugStream::DebugStream(const char* name) {
    std::cout << '[MSG:' << name << ": ";
}
void DebugStream::add(char c) {
    std::cout << c;
}

DebugStream::~DebugStream() {
    std::cout << ']' << std::endl;
}

#else

#    include "Serial.h"

DebugStream::DebugStream(const char* name) {
    client_write(CLIENT_ALL, "[MSG:");
    client_write(CLIENT_ALL, name);
    client_write(CLIENT_ALL, ": ");
}

void DebugStream::add(char c) {
    char txt[2];
    txt[0] = c;
    txt[1] = '\0';
    client_write(CLIENT_ALL, txt);
}

DebugStream::~DebugStream() {
    client_write(CLIENT_ALL, "]");
    client_write(CLIENT_ALL, "\r\n");
}

#endif
