#include "MachineConfig.h"

#include "Motors/Motor.h"

// Configuration system helpers:
void Axis::validate() const  {
    Assert(motor_ != nullptr, "Motor should be defined when an axis is defined.");
}

void Axis::handle(Configuration::HandlerBase& handler)  {
    Motors::MotorFactory::handle(handler, motor_);
}

// Checks if a motor matches this axis:
bool Axis::hasMotor(const Motors::Motor* const motor) const {
    return motor_ == motor;
}

Axis::~Axis() {
    delete motor_;
}

Axes::Axes() : axis_()
{
    for (int i = 0; i < MAX_NUMBER_AXIS; ++i)
    {
        for (int j = 0; j <= MAX_NUMBER_GANGED; ++j)
        {
            axis_[i][j] = nullptr;
        }
    }
}

// Some small helpers to find the axis index and axis ganged index for a given motor. This 
// is helpful for some motors that need this info, as well as debug information.
size_t Axes::findAxisIndex(const Motors::Motor* const motor) const {
    for (int i = 0; i < MAX_NUMBER_AXIS; ++i)
    {
        for (int j = 0; j <= MAX_NUMBER_GANGED; ++j)
        {
            if (axis_[i][j] != nullptr && axis_[i][j]->hasMotor(motor)) {
                return i;
            }
        }
    }

    Assert(false, "Cannot find axis for motor. Something wonky is going on here..."); 
    return SIZE_MAX;
}

size_t Axes::findAxisGanged(const Motors::Motor* const motor) const {
    for (int i = 0; i < MAX_NUMBER_AXIS; ++i)
    {
        for (int j = 0; j <= MAX_NUMBER_GANGED; ++j)
        {
            if (axis_[i][j] != nullptr && axis_[i][j]->hasMotor(motor)) {
                return j;
            }
        }
    }

    Assert(false, "Cannot find axis for motor. Something wonky is going on here...");
    return SIZE_MAX;
}

// Configuration helpers:
void Axes::validate() const { }

void Axes::handle(Configuration::HandlerBase& handler) {
    const char* allAxis = "xyzabc";

    char tmp[3];
    tmp[2] = '\0';

    for (size_t a = 0; a < MAX_NUMBER_AXIS; ++a)
    {
        tmp[0] = allAxis[a];
        tmp[1] = '\0';

        if (handler.handlerType() == Configuration::HandlerType::Runtime ||
            handler.handlerType() == Configuration::HandlerType::Parser)
        {
            // 'x' is a shorthand for 'x1', so we don't generate it.
            handler.handle(tmp, axis_[a][1]);
        }

        for (size_t g = 1; g <= MAX_NUMBER_GANGED; ++g)
        {
            tmp[1] = char('0' + g);
            handler.handle(tmp, axis_[a][g]);
        }
    }
}

Axes::~Axes() {
    for (int i = 0; i < MAX_NUMBER_AXIS; ++i)
    {
        for (int j = 0; j <= MAX_NUMBER_GANGED; ++j)
        {
            delete axis_[i][j];
        }
    }
}


void MachineConfig::validate() const  { }

void MachineConfig::handle(Configuration::HandlerBase& handler)  {
    handler.handle("axes", axes_);
}

MachineConfig::~MachineConfig() {
    delete axes_;
}
