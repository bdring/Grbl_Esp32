#ifdef ESP32

#include "TestFactory.h"
#include <Arduino.h>
#include <HardwareSerial.h>

void setup() {
    Serial.begin(115200);
}

void loop() {
    // Run all tests:
    TestFactory::instance().runAll();

    // Wait:
    while (1) {
        delay(1000);
    }
}

#endif
