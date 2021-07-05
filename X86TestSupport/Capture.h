#pragma once

#include <string>
#include <cstdint>
#include <vector>

// Capture here defines everything that we want to know. Specifically, we want to capture per ID:
// 1. Timings. *When* did something happen?
// 2. Data. This can be a simple '1' or '0', or a character stream. For simplicity, we store a vector of integers.
//
// An ID itself is a string. This can be a pin ID (gpio.1), an uart (uart.0), an ledc, or whatever.

struct CaptureEvent {
    uint32_t              time;
    std::string           id;
    std::vector<uint32_t> data;
};

class Capture {
    Capture() {}

    std::vector<CaptureEvent> events;
    uint32_t                  currentTime;

public:
    static Capture& instance() {
        static Capture instance;
        return instance;
    }

    void reset() { events.clear(); }

    void write(std::string id, uint32_t value) {
        CaptureEvent evt;
        evt.time = currentTime;
        evt.id   = id;
        evt.data.reserve(1);
        evt.data.push_back(value);
        events.push_back(evt);
    }

    void wait(uint32_t delay) { currentTime += delay; }
    void yield() { wait(1); }
};
