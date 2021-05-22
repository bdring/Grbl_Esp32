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
#    define LOG_CLIENT CLIENT_SERIAL

DebugStream::DebugStream(const char* name) {
    client_write(LOG_CLIENT, "[MSG:");
    client_write(LOG_CLIENT, name);
    client_write(LOG_CLIENT, ": ");
}

void DebugStream::add(char c) {
    char txt[2];
    txt[0] = c;
    txt[1] = '\0';
    client_write(LOG_CLIENT, txt);
}

DebugStream::~DebugStream() {
    client_write(LOG_CLIENT, "]");
    client_write(LOG_CLIENT, "\r\n");
}

#endif
