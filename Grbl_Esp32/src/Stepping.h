#pragma once
// Stepping is the interface class for operations that
// apply to all steppers at once.
#include "Grbl.h"

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
    void         lowLatency() {};
    void         normalLatency() {};
    virtual void  backoffDelay() {};
    void          finishStep(uint64_t startTime) {};
};

class RMTStepping : public Stepping {
public:
    RMTStepping() : Stepping("RMT") {};
    void init();
    void reset() {}
    void setPeriod(uint16_t timerTicks);
    void start();
    void stop();
    void finishStep(uint64_t startTime) {}
};

class TimedStepping : public Stepping {
public:
    TimedStepping() : Stepping("Timed") {};
    void init();
    void reset() {}
    void setPeriod(uint16_t timerTicks);
    void start();
    void stop();
    void finishStep(uint64_t startTime);
};

class I2SStepping : public Stepping {
private:
    bool _streaming;

public:
    I2SStepping() : Stepping("I2S"), _streaming(true) {}
    void init();
    void reset();
    void setPeriod(uint16_t timerTicks);
    void start();
    void stop();
    void finishStep(uint64_t startTime);
    void lowLatency();
    void normalLatency() {}
    void backoffDelay() override;
};

extern I2SStepping* stepping;
