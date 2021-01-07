#pragma once
// Stepping is the interface class for operations that
// apply to all steppers at once.
#include "Grbl.h"

void stepping_select();

class Stepping {
protected:
    const char* _name;

public:
    Stepping(const char* name) : _name(name) {}
    const char*  name() { return _name; }
    virtual void init()                         = 0;
    virtual void reset()                        = 0;
    virtual void setPeriod(uint16_t timerTicks) = 0;
    virtual void start()                        = 0;
    virtual void stop()                         = 0;
    virtual void lowLatency() {};
    virtual void normalLatency() {};
    virtual void backoffDelay() {};
    virtual void finishStep(uint64_t startTime) {};
    ~Stepping() {};
};

class RMTStepping : public Stepping {
public:
    RMTStepping() : Stepping("RMT") {};
    void init() override;
    void reset() override {}
    void setPeriod(uint16_t timerTicks) override;
    void start() override;
    void stop() override;
    void finishStep(uint64_t startTime) override {}
};

class TimedStepping : public Stepping {
public:
    TimedStepping() : Stepping("Timed") {};
    void init() override;
    void reset() override {} 
    void setPeriod(uint16_t timerTicks) override;
    void start() override;
    void stop() override;
    void finishStep(uint64_t startTime) override;
};

class I2SStepping : public Stepping {
private:
    bool _streaming;

public:
    I2SStepping() : Stepping("I2S"), _streaming(true) {}
    void init() override;
    void reset() override;
    void setPeriod(uint16_t timerTicks) override;
    void start() override;
    void stop() override;
    void finishStep(uint64_t startTime) override;
    void lowLatency() override;
    // void normalLatency() {}
    void backoffDelay() override;
};

extern Stepping* stepping;
