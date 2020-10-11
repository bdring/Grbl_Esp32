#include "TestFactory.h"

#ifdef ESP32

#    include "ESP32StackTrace/AssertionFailed.h"
#    include <HardwareSerial.h>
#    include <string.h>

void TestFactory::runAll() {
    Serial.begin(115200);

    const int Indent = 80;

    char spaces[Indent];
    memset(spaces, ' ', Indent - 1);
    spaces[Indent] = '\0';

    auto        current = first;
    const char* prev    = nullptr;
    while (current) {
        auto curTest = current->unitTestName();

        if (prev == nullptr || !strcmp(prev, curTest)) {
            Serial.println("- Test: %s", curTest);
            prev = curTest;
        }

        Serial.print("  - Case: %s", current->unitTestCase());

        int len = int(strlen(current->unitTestCase()));
        if (len >= (Indent - 5)) {
            len = (Indent - 5);
        }
        Serial.print(spaces + len);  // pad.
        Serial.flush();

        try {
            current->run();
            Serial.println("Passed.");
        } catch (AssertionFailed& ex) {
            Serial.println("FAILED!");
            Serial.println(ex.stackTrace.c_str());
        } catch (...) { Serial.println("FAILED!"); }
        current = current->next;
    }
}

#else

#    include <src/Assert.h>
#    include <iostream>
#    include <string.h>

#    if defined _WIN32 || defined _WIN64
#        define WIN32_LEAN_AND_MEAN
#        include <Windows.h>
void setColor(int colorIndex) { // 10 = green, 12 = red, 7 = gray, 15 = white
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    // you can loop k higher to see more color choices
    // pick the colorattribute k you want
    SetConsoleTextAttribute(hConsole, colorIndex);
}
#    else
void setColor(int colorIndex) {}
#    endif

void TestFactory::runAll() {
    const int Indent = 80;

    char spaces[Indent];
    memset(spaces, ' ', Indent - 1);
    spaces[Indent] = '\0';

    auto        current = first;
    const char* prev    = nullptr;
    while (current) {
        auto curTest = current->unitTestName();

        setColor(15);

        if (prev == nullptr || !strcmp(prev, curTest)) {
            printf("- Test: %s\r\n", curTest);
            prev = curTest;
        }

        setColor(7);

        printf("  - Case: %s", current->unitTestCase());

        int len = int(strlen(current->unitTestCase()));
        if (len >= (Indent - 5)) {
            len = (Indent - 5);
        }
        printf(spaces + len);  // pad.

        try {
            setColor(10);

            current->run();
            printf("Passed.\r\n");
        } catch (AssertionFailed& ex) {
            setColor(12);
            printf("FAILED!\r\n");
            printf(ex.stackTrace.c_str());
            printf("\r\n");
        } catch (...) {
            setColor(12);
            printf("FAILED!\r\n");
            // We don't know where unfortunately...
        }
        current = current->next;

        setColor(7);
    }
}
#endif
