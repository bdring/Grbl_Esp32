#include "MachineConfig.h"

#include "Motors/Motor.h"
#include "Motors/NullMotor.h"

#include "Grbl.h"
#include "Logging.h"

#include "Configuration/ParserHandler.h"
#include "Configuration/Validator.h"
#include "Configuration/ParseException.h"

#include <SPIFFS.h>
#include <cstdio>

// Configuration system helpers:
void Axis::validate() const {
    Assert(motor_ != nullptr, "Motor should be defined when an axis is defined.");
}

void Axis::handle(Configuration::HandlerBase& handler) {
    Motors::MotorFactory::handle(handler, motor_);
}

// Checks if a motor matches this axis:
bool Axis::hasMotor(const Motors::Motor* const motor) const {
    return motor_ == motor;
}

Axis::~Axis() {
    delete motor_;
}

Axes::Axes() : axis_() {
    for (int i = 0; i < MAX_NUMBER_AXIS; ++i) {
        for (int j = 0; j <= MAX_NUMBER_GANGED; ++j) {
            axis_[i][j] = nullptr;
        }
    }
}

void Axes::init() {
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Init Motors");

#ifdef USE_STEPSTICK

    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Using StepStick Mode");

    Pin ms3_pins[MAX_N_AXIS][2] = { { StepStickMS3[X_AXIS][0]->get(), StepStickMS3[X_AXIS][1]->get() },
                                    { StepStickMS3[Y_AXIS][0]->get(), StepStickMS3[Y_AXIS][1]->get() },
                                    { StepStickMS3[Z_AXIS][0]->get(), StepStickMS3[Z_AXIS][1]->get() },
                                    { StepStickMS3[A_AXIS][0]->get(), StepStickMS3[A_AXIS][1]->get() },
                                    { StepStickMS3[B_AXIS][0]->get(), StepStickMS3[B_AXIS][1]->get() },
                                    { StepStickMS3[C_AXIS][0]->get(), StepStickMS3[C_AXIS][1]->get() } };

    for (int axis = 0; axis < MAX_NUMBER_AXIS; axis++) {
        for (int gang_index = 0; gang_index < MAX_NUMBER_GANGED; gang_index++) {
            auto a = axis_[axis][gang_index];
            if (a != nullptr && a->motor_ != nullptr) {
                Pin pin = ms3_pins[axis][gang_index];
                if (pin != Pin::UNDEFINED) {
                    pin.setAttr(Pin::Attr::Output | Pin::Attr::InitialOn);
                }
            }
        }
    }

    if (StepperResetPin->get() != Pin::UNDEFINED) {
        // !RESET pin on steppers  (MISO On Schematic)
        StepperResetPin->get().setAttr(Pin::Attr::Output | Pin::Attr::InitialOn);
    }

#endif

    if (SteppersDisablePin->get() != Pin::UNDEFINED) {
        SteppersDisablePin->get().setAttr(Pin::Attr::Output);  // global motor enable pin
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Global stepper disable pin:%s", SteppersDisablePin->get().name());
    }

    // certain motors need features to be turned on. Check them here
    for (uint8_t axis = X_AXIS; axis < MAX_NUMBER_AXIS; axis++) {
        for (uint8_t gang_index = 0; gang_index < MAX_NUMBER_GANGED; gang_index++) {
            auto a = axis_[axis][gang_index];

            if (a->motor_ == nullptr)
            {
                a->motor_ = new Motors::Nullmotor();
            }

            a->motor_->init();
        }
    }
}

void Axes::set_disable(bool disable) {
    static bool previous_state = true;

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Motors disable %d", disable);

    /*
    if (previous_state == disable) {
        return;
    }
    previous_state = disable;
*/

    // now loop through all the motors to see if they can individually disable
    for (int axis = 0; axis < MAX_NUMBER_AXIS; axis++) {
        for (int gang_index = 0; gang_index < MAX_NUMBER_GANGED; gang_index++) {
            auto a = axis_[axis][gang_index];
            a->motor_->set_disable(disable);
        }
    }

    // invert only inverts the global stepper disable pin.
    if (step_enable_invert->get()) {
        disable = !disable;  // Apply pin invert.
    }
    SteppersDisablePin->get().write(disable);
}

void Axes::read_settings() {
    //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Read Settings");
    for (uint8_t axis = X_AXIS; axis < MAX_NUMBER_AXIS; axis++) {
        for (uint8_t gang_index = 0; gang_index < MAX_NUMBER_GANGED; gang_index++) {
            auto a = axis_[axis][gang_index];
            a->motor_->read_settings();
        }
    }
}

// use this to tell all the motors what the current homing mode is
// They can use this to setup things like Stall
uint8_t Axes::set_homing_mode(uint8_t homing_mask, bool isHoming) {
    uint8_t can_home = 0;

    for (uint8_t axis = X_AXIS; axis < MAX_NUMBER_AXIS; axis++) {
        if (bitnum_istrue(homing_mask, axis)) {
            auto a = axis_[axis][0];
            if (a != nullptr && a->motor_ != nullptr) {
                auto motor = a->motor_;

                if (motor->set_homing_mode(isHoming)) {
                    bitnum_true(can_home, axis);
                }

                for (uint8_t gang_index = 1; gang_index < MAX_NUMBER_GANGED; gang_index++) {
                    auto a2 = axis_[axis][gang_index];
                    a2->motor_->set_homing_mode(isHoming);
                }
            }
        }
    }

    return can_home;
}

void Axes::step(uint8_t step_mask, uint8_t dir_mask) {
    auto n_axis = number_axis->get();
    //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "motors_set_direction_pins:0x%02X", onMask);

    // Set the direction pins, but optimize for the common
    // situation where the direction bits haven't changed.
    static uint8_t previous_dir = 255;  // should never be this value
    if (dir_mask != previous_dir) {
        previous_dir = dir_mask;

        for (int axis = X_AXIS; axis < MAX_NUMBER_AXIS; axis++) {
            bool thisDir = bitnum_istrue(dir_mask, axis);

            for (uint8_t gang_index = 0; gang_index < MAX_NUMBER_GANGED; gang_index++) {
                auto a = axis_[axis][gang_index];

                if (a != nullptr && a->motor_ != nullptr) {
                    a->motor_->set_direction(thisDir);
                }
            }
        }
    }

    // TODO FIXME: SdB This is not really correct... ganged_mode shouldn't be 'A' or 'B', but
    // an index or a wildcard because for the new settings we have not bounded the number of 
    // gangs.

    // Turn on step pulses for motors that are supposed to step now
    for (int axis = X_AXIS; axis < MAX_NUMBER_AXIS; axis++) {
        if (bitnum_istrue(step_mask, axis)) {
            auto a = axis_[axis];

            if ((ganged_mode == SquaringMode::Dual) || (ganged_mode == SquaringMode::A)) {
                a[0]->motor_->step();
            }
            if ((ganged_mode == SquaringMode::Dual) || (ganged_mode == SquaringMode::B)) {
                a[1]->motor_->step();
            }
        }
    }

    for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
        if (bitnum_istrue(step_mask, axis)) {
            auto a = axis_[axis];

            if ((ganged_mode == SquaringMode::Dual) || (ganged_mode == SquaringMode::A)) {
                a[0]->motor_->step();
            }
            if ((ganged_mode == SquaringMode::Dual) || (ganged_mode == SquaringMode::B)) {
                a[1]->motor_->step();
            }
        }
    }
}
// Turn all stepper pins off
void Axes::unstep() {
    auto n_axis = number_axis->get();
    for (uint8_t axis = X_AXIS; axis < MAX_NUMBER_AXIS; axis++) {
        for (uint8_t gang_index = 0; gang_index < MAX_NUMBER_GANGED; gang_index++) {
            auto a = axis_[axis][gang_index];
            a->motor_->unstep();
            a->motor_->unstep();
        }
    }
}

// Some small helpers to find the axis index and axis ganged index for a given motor. This
// is helpful for some motors that need this info, as well as debug information.
size_t Axes::findAxisIndex(const Motors::Motor* const motor) const {
    for (int i = 0; i < MAX_NUMBER_AXIS; ++i) {
        for (int j = 0; j <= MAX_NUMBER_GANGED; ++j) {
            if (axis_[i][j] != nullptr && axis_[i][j]->hasMotor(motor)) {
                return i;
            }
        }
    }

    Assert(false, "Cannot find axis for motor. Something wonky is going on here...");
    return SIZE_MAX;
}

size_t Axes::findAxisGanged(const Motors::Motor* const motor) const {
    for (int i = 0; i < MAX_NUMBER_AXIS; ++i) {
        for (int j = 0; j <= MAX_NUMBER_GANGED; ++j) {
            if (axis_[i][j] != nullptr && axis_[i][j]->hasMotor(motor)) {
                return j;
            }
        }
    }

    Assert(false, "Cannot find axis for motor. Something wonky is going on here...");
    return SIZE_MAX;
}

// Configuration helpers:
void Axes::validate() const {}

void Axes::handle(Configuration::HandlerBase& handler) {
    const char* allAxis = "xyzabc";

    char tmp[3];
    tmp[2] = '\0';

    for (size_t a = 0; a < MAX_NUMBER_AXIS; ++a) {
        tmp[0] = allAxis[a];
        tmp[1] = '\0';

        if (handler.handlerType() == Configuration::HandlerType::Runtime || handler.handlerType() == Configuration::HandlerType::Parser) {
            // 'x' is a shorthand for 'x1', so we don't generate it.
            handler.handle(tmp, axis_[a][1]);
        }

        for (size_t g = 1; g <= MAX_NUMBER_GANGED; ++g) {
            tmp[1] = char('0' + g);
            handler.handle(tmp, axis_[a][g]);
        }
    }
}

Axes::~Axes() {
    for (int i = 0; i < MAX_NUMBER_AXIS; ++i) {
        for (int j = 0; j <= MAX_NUMBER_GANGED; ++j) {
            delete axis_[i][j];
        }
    }
}

void MachineConfig::validate() const {}

void MachineConfig::handle(Configuration::HandlerBase& handler) {
    handler.handle("axes", axes_);
}

bool MachineConfig::load(const char* filename) {
    if (!SPIFFS.begin(true)) {
        error("An error has occurred while mounting SPIFFS");
        return false;
    }

    FILE *file = fopen(filename, "rb");
    if (!file) {
        error("There was an error opening the config file for reading");
        return false;
    }

    // Let's just read the entire file in one chunk for now. If we get 
    // in trouble with this, we can cut it in pieces and read it per chunk.
    fseek(file, 0, SEEK_END);
    auto filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = new char[filesize];
    
    long pos = 0;
    while (pos < filesize)
    {
        auto read = fread(buffer + pos, 1, filesize - pos, file);
        if (read == 0) { break; }
        pos += read;
    }

    fclose(file);

    if (pos != filesize) {
        delete[] buffer;

        error("There was an error reading the config file");
        return false;
    }

    // Process file:
    StringRange input(buffer, buffer + filesize);
    bool succesful = false;
    try {
        Configuration::Parser parser(input.begin(), input.end());
        Configuration::ParserHandler handler(parser);

        // Instance is by reference, so we can just get rid of an old instance and 
        // create a new one here:
        if (instance() != nullptr)
        {
            delete instance();
        }
        instance() = new MachineConfig();
        MachineConfig *machine = instance();

        for (; !parser.isEndSection(); parser.moveNext())
        {
            info("Parsing key " << parser.key().str());
            machine->handle(handler);
        }

        info("Done parsing machine config.");

        try {
            Configuration::Validator validator;
            machine->handle(validator);
        }
        catch (std::exception& ex)
        {
            info("Validation error: " << ex.what());
        }

        info("Done validating machine config.");

        succesful = true;
    }
    catch (const Configuration::ParseException& ex) {
        error("Configuration parse error: " << ex.What() << " @ " << ex.LineNumber() << ":" << ex.ColumnNumber());
    }
    catch (const AssertionFailed& ex) {
        // Get rid of buffer and return
        error("Configuration loading failed: " << ex.what());
    }
    catch (std::exception& ex)
    {
        error("Configuration validation error: " << ex.what());
    }
    catch (...) {
        // Get rid of buffer and return
        error("Unknown error occurred while processing configuration file.");
    }

    // Get rid of buffer and return
    delete[] buffer;
    return succesful;
}

MachineConfig::~MachineConfig() {
    delete axes_;
}
