/*
    Part of Grbl_ESP32
    2021 -  Stefan de Bruijn

    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Grbl_ESP32 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "MachineConfig.h"

#include "Motors/Motor.h"
#include "Motors/NullMotor.h"

#include "Spindles/NullSpindle.h"

#include "Grbl.h"
#include "Logging.h"

#include "Configuration/Parser.h"
#include "Configuration/ParserHandler.h"
#include "Configuration/Validator.h"
#include "Configuration/AfterParse.h"
#include "Configuration/ParseException.h"

#include <SPIFFS.h>
#include <cstdio>
#include <cstring>
#include <atomic>

MachineConfig* config;

// TODO FIXME: Split this file up into several files, perhaps put it in some folder and namespace Machine?

void Endstops::validate() const {
    //    if (_dual.defined()) {
    //        Assert(_positive.undefined(), "If dual endstops are defined, you cannot also define positive and negative endstops");
    //        Assert(_negative.undefined(), "If dual endstops are defined, you cannot also define positive and negative endstops");
    //    }
    //    if (_positive.defined() || _negative.defined()) {
    //        Assert(_positive.undefined(), "If positive or negative endstops are defined, you cannot also define dual endstops");
    //    }
}

void Endstops::group(Configuration::HandlerBase& handler) {
    //     handler.item("positive", _positive);
    //     handler.item("negative", _negative);
    handler.item("dual", _dual);
    handler.item("hard_limits", _hardLimits);
}

void Gang::group(Configuration::HandlerBase& handler) {
    handler.section("endstops", _endstops);
    Motors::MotorFactory::factory(handler, _motor);
}
void Gang::afterParse() {
    if (_motor == nullptr) {
        _motor = new Motors::Nullmotor();
    }
}

Gang::~Gang() {
    delete _motor;
    delete _endstops;
}

void Axis::group(Configuration::HandlerBase& handler) {
    handler.item("steps_per_mm", _stepsPerMm);
    handler.item("max_rate", _maxRate);
    handler.item("acceleration", _acceleration);
    handler.item("max_travel", _maxTravel);
    handler.item("soft_limits", _softLimits);

    handler.section("homing", _homing);

    char tmp[6];
    tmp[0] = 0;
    strcat(tmp, "gang");

    for (size_t g = 0; g < MAX_NUMBER_GANGED; ++g) {
        tmp[4] = char(g + '0');
        tmp[5] = '\0';

        handler.section(tmp, _gangs[g]);
    }
}

void Axis::afterParse() {
    for (size_t i = 0; i < MAX_NUMBER_GANGED; ++i) {
        if (_gangs[i] == nullptr) {
            _gangs[i] = new Gang();
        }
    }
}

// Checks if a motor matches this axis:
bool Axis::hasMotor(const Motors::Motor* const motor) const {
    for (uint8_t gang_index = 0; gang_index < MAX_NUMBER_GANGED; gang_index++) {
        if (_gangs[gang_index]->_motor == motor) {
            return true;
        }
    }
    return false;
}

Axis::~Axis() {
    for (uint8_t gang_index = 0; gang_index < MAX_NUMBER_GANGED; gang_index++) {
        delete _gangs[gang_index];
    }
}

Axes::Axes() : _axis() {
    for (int i = 0; i < MAX_NUMBER_AXIS; ++i) {
        _axis[i] = nullptr;
    }
}

void Axes::init() {
    info_serial("Init Motors");

    if (_sharedStepperDisable.defined()) {
        _sharedStepperDisable.setAttr(Pin::Attr::Output);
        _sharedStepperDisable.report("Shared stepper disable");
    }

    // certain motors need features to be turned on. Check them here
    for (uint8_t axis = X_AXIS; axis < _numberAxis; axis++) {
        for (uint8_t gang_index = 0; gang_index < Axis::MAX_NUMBER_GANGED; gang_index++) {
            auto a = _axis[axis];
            if (a) {
                auto g = a->_gangs[gang_index];
                if (g) {
                    auto m = g->_motor;
                    if (m == nullptr) {
                        m = new Motors::Nullmotor();
                    }
                    m->init();
                }
            }
        }
    }
}

void Axes::set_disable(bool disable) {
    static bool previous_state = true;

    //info_serial("Motors disable %d", disable);

    /*
    if (previous_state == disable) {
        return;
    }
    previous_state = disable;
*/

    // now loop through all the motors to see if they can individually disable
    for (int axis = 0; axis < _numberAxis; axis++) {
        for (int gang_index = 0; gang_index < Axis::MAX_NUMBER_GANGED; gang_index++) {
            auto a = _axis[axis]->_gangs[gang_index]->_motor;
            a->set_disable(disable);
        }
    }

    // invert only inverts the global stepper disable pin.
    _sharedStepperDisable.write(disable);
}

void Axes::read_settings() {
    //info_serial("Read Settings");
    for (uint8_t axis = X_AXIS; axis < _numberAxis; axis++) {
        for (uint8_t gang_index = 0; gang_index < Axis::MAX_NUMBER_GANGED; gang_index++) {
            auto a = _axis[axis];
            if (!a) {
                log_info("No specification for axis " << axis);
                break;
            }
            auto g = a->_gangs[gang_index];
            if (!g) {
                log_info("No specification for axis " << axis << " gang " << gang_index);
                break;
            }
            auto m = g->_motor;
            if (!m) {
                log_info("No motor for axis " << axis << " gang " << gang_index);
            }
            m->read_settings();
        }
    }
}

// use this to tell all the motors what the current homing mode is
// They can use this to setup things like Stall
uint8_t Axes::set_homing_mode(uint8_t homing_mask, bool isHoming) {
    uint8_t can_home = 0;

    for (uint8_t axis = X_AXIS; axis < _numberAxis; axis++) {
        if (bitnum_istrue(homing_mask, axis)) {
            auto a = _axis[axis];
            if (a != nullptr) {
                auto motor = a->_gangs[0]->_motor;

                if (motor->set_homing_mode(isHoming)) {
                    bitnum_true(can_home, axis);
                }

                for (uint8_t gang_index = 1; gang_index < Axis::MAX_NUMBER_GANGED; gang_index++) {
                    auto a2 = _axis[axis]->_gangs[gang_index]->_motor;
                    a2->set_homing_mode(isHoming);
                }
            }
        }
    }

    return can_home;
}

void IRAM_ATTR Axes::step(uint8_t step_mask, uint8_t dir_mask) {
    auto n_axis = _numberAxis;
    //info_serial("motors_set_direction_pins:0x%02X", onMask);

    // Set the direction pins, but optimize for the common
    // situation where the direction bits haven't changed.
    static uint8_t previous_dir = 255;  // should never be this value
    if (dir_mask != previous_dir) {
        previous_dir = dir_mask;

        for (int axis = X_AXIS; axis < n_axis; axis++) {
            bool thisDir = bitnum_istrue(dir_mask, axis);

            for (uint8_t gang_index = 0; gang_index < Axis::MAX_NUMBER_GANGED; gang_index++) {
                auto a = _axis[axis]->_gangs[gang_index]->_motor;

                if (a != nullptr) {
                    a->set_direction(thisDir);
                }
            }
        }
    }

    // Turn on step pulses for motors that are supposed to step now
    for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
        if (bitnum_istrue(step_mask, axis)) {
            auto a = _axis[axis];

            if (bitnum_istrue(ganged_mode, 0)) {
                a->_gangs[0]->_motor->step();
            }
            if (bitnum_istrue(ganged_mode, 1)) {
                a->_gangs[1]->_motor->step();
            }
        }
    }
}
// Turn all stepper pins off
void IRAM_ATTR Axes::unstep() {
    auto n_axis = _numberAxis;
    for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
        for (uint8_t gang_index = 0; gang_index < Axis::MAX_NUMBER_GANGED; gang_index++) {
            auto a = _axis[axis]->_gangs[gang_index]->_motor;
            a->unstep();
            a->unstep();
        }
    }
}

// Some small helpers to find the axis index and axis ganged index for a given motor. This
// is helpful for some motors that need this info, as well as debug information.
size_t Axes::findAxisIndex(const Motors::Motor* const motor) const {
    for (int i = 0; i < _numberAxis; ++i) {
        for (int j = 0; j < Axis::MAX_NUMBER_GANGED; ++j) {
            if (_axis[i] != nullptr && _axis[i]->hasMotor(motor)) {
                return i;
            }
        }
    }

    Assert(false, "Cannot find axis for motor. Something wonky is going on here...");
    return SIZE_MAX;
}

size_t Axes::findAxisGanged(const Motors::Motor* const motor) const {
    for (int i = 0; i < _numberAxis; ++i) {
        if (_axis[i] != nullptr && _axis[i]->hasMotor(motor)) {
            for (int j = 0; j < Axis::MAX_NUMBER_GANGED; ++j) {
                if (_axis[i]->_gangs[j]->_motor == motor) {
                    return j;
                }
            }
        }
    }

    Assert(false, "Cannot find axis for motor. Something wonky is going on here...");
    return SIZE_MAX;
}

// Configuration helpers:

void Axes::group(Configuration::HandlerBase& handler) {
    handler.item("number_axis", _numberAxis);
    handler.item("shared_stepper_disable", _sharedStepperDisable);

    const char* allAxis = "xyzabc";

    char tmp[3];
    tmp[2] = '\0';

    for (size_t a = 0; a < MAX_NUMBER_AXIS; ++a) {
        tmp[0] = allAxis[a];
        tmp[1] = '\0';

        if (handler.handlerType() == Configuration::HandlerType::Runtime || handler.handlerType() == Configuration::HandlerType::Parser ||
            handler.handlerType() == Configuration::HandlerType::AfterParse ||
            handler.handlerType() == Configuration::HandlerType::Generator) {
            handler.section(tmp, _axis[a]);
        }
    }
}

void Axes::afterParse() {
    for (size_t i = 0; i < MAX_NUMBER_AXIS; ++i) {
        if (_axis[i] == nullptr) {
            _axis[i] = new Axis();
        }
    }
}

Axes::~Axes() {
    for (int i = 0; i < MAX_NUMBER_AXIS; ++i) {
        delete _axis[i];
    }
}

void I2SOBus::validate() const {
    if (_bck.defined() || _data.defined() || _ws.defined()) {
        Assert(_bck.defined(), "I2SO BCK pin should be configured once");
        Assert(_data.defined(), "I2SO Data pin should be configured once");
        Assert(_ws.defined(), "I2SO WS pin should be configured once");
    }
}

void I2SOBus::group(Configuration::HandlerBase& handler) {
    handler.item("bck", _bck);
    handler.item("data", _data);
    handler.item("ws", _ws);
}

void SPIBus::validate() const {
    if (_ss.defined() || _miso.defined() || _mosi.defined() || _sck.defined()) {
        Assert(_ss.defined(), "SPI SS pin should be configured once");
        Assert(_miso.defined(), "SPI MISO pin should be configured once");
        Assert(_mosi.defined(), "SPI MOSI pin should be configured once");
        Assert(_sck.defined(), "SPI SCK pin should be configured once");
    }
}

void SPIBus::init() {
    if (_ss.defined()) { // validation ensures the rest is also defined.
        auto ssPin   = _ss.getNative(Pin::Capabilities::Output | Pin::Capabilities::Native);
        auto mosiPin = _mosi.getNative(Pin::Capabilities::Output | Pin::Capabilities::Native);
        auto sckPin  = _sck.getNative(Pin::Capabilities::Output | Pin::Capabilities::Native);
        auto misoPin = _miso.getNative(Pin::Capabilities::Input | Pin::Capabilities::Native);

        SPI.begin(sckPin, misoPin, mosiPin, ssPin);
    }
}

void SPIBus::group(Configuration::HandlerBase& handler) {
    handler.item("ss", _ss);
    handler.item("miso", _miso);
    handler.item("mosi", _mosi);
    handler.item("mosi", _sck);
}

void SPIBus::afterParse() {
    if (_ss.undefined() && _miso.undefined() && _mosi.undefined() && _sck.undefined()) {
        // Default SPI miso, mosi, sck, ss pins to the "standard" gpios 19, 23, 18, 5
        _miso = Pin::create("gpio.19");
        _mosi = Pin::create("gpio.23");
        _sck  = Pin::create("gpio.18");
        _ss   = Pin::create("gpio.5");
    }
}

void UserOutputs::group(Configuration::HandlerBase& handler) {
    handler.item("analog0", _analogOutput[0]);
    handler.item("analog1", _analogOutput[1]);
    handler.item("analog2", _analogOutput[2]);
    handler.item("analog3", _analogOutput[3]);
    handler.item("digital0", _digitalOutput[0]);
    handler.item("digital1", _digitalOutput[1]);
    handler.item("digital2", _digitalOutput[2]);
    handler.item("digital3", _digitalOutput[3]);
}

void MachineConfig::group(Configuration::HandlerBase& handler) {
    handler.item("board", _board);
    handler.item("name", _name);

    handler.section("axes", _axes);
    handler.section("i2so", _i2so);
    handler.section("spi", _spi);
    handler.section("control", _control);
    handler.section("coolant", _coolant);
    handler.section("probe", _probe);
    handler.section("comms", _comms);

    handler.item("pulse_microseconds", _pulseMicroSeconds);
    handler.item("dir_delay_microseconds", _directionDelayMicroSeconds);
    handler.item("disable_delay_us", _disableDelayMicroSeconds);
    handler.item("idle_time", _idleTime);
    handler.section("user_outputs", _userOutputs);
    handler.section("sdcard", _sdCard);
    handler.item("step_type", _stepType, stepTypes);

    // TODO: Consider putting these under a gcode: hierarchy level? Or motion control?
    handler.item("laser_mode", _laserMode);
    handler.item("arc_tolerance", _arcTolerance);
    handler.item("junction_deviation", _junctionDeviation);
    handler.item("verbose_errors", _verboseErrors);
    handler.item("report_inches", _reportInches);
    handler.item("homing_init_lock", _homingInitLock);
    handler.item("enable_parking_override_control", _enableParkingOverrideControl);
    handler.item("deactivate_parking_upon_init", _deactivateParkingUponInit);
    handler.item("check_limits_at_init", _checkLimitsAtInit);
    handler.item("homing_single_axis_commands", _homingSingleAxisCommands);
    handler.item("limits_two_switches_on_axis", _limitsTwoSwitchesOnAxis);

    Spindles::SpindleFactory::factory(handler, _spindle);
}

void MachineConfig::afterParse() {
    if (_axes == nullptr) {
        log_info("Axes config missing; building default axes");
        _axes = new Axes();
    }

    if (_coolant == nullptr) {
        log_info("Coolant control config missing; building default coolant");
        _coolant = new CoolantControl();
    }

    if (_probe == nullptr) {
        log_info("Probe config missing; building default probe");
        _probe = new Probe();
    }

    if (_userOutputs == nullptr) {
        _userOutputs = new UserOutputs();
    }

    if (_sdCard == nullptr) {
        _sdCard = new SDCard();
    }

    if (_spi == nullptr) {
        _spi = new SPIBus();
    }

    if (_i2so == nullptr) {
        _i2so = new I2SOBus();
    }

    if (_control == nullptr) {
        log_info("Control config missing; building default");
        _control = new Control();
    }

    if (_spindle == nullptr) {
        log_info("Using null spindle");
        _spindle = new Spindles::Null();
    }

    if (_comms == nullptr) {
        log_info("Comms config missing; building default comms");
        _comms = new Communications();
#ifdef ENABLE_WIFI
        _comms->_apConfig = new WifiAPConfig();
#endif
    }

#ifdef ENABLE_WIFI
    // This is very helpful for testing YAML config files.  If things
    // screw up, you can still connect and upload a new config.yaml
    // TODO - Consider whether we want this for the long term
    if (!_comms->_apConfig) {
        _comms->_apConfig = new WifiAPConfig();
    }
#endif
}

size_t MachineConfig::readFile(const char* filename, char*& buffer) {
    String path = filename;
    if ((path.length() > 0) && (path[0] != '/')) {
        path = "/" + path;
    }

    File file = SPIFFS.open(path, FILE_READ);

    // There is a subtle problem with the Arduino framework.  If
    // the framework does not find the file, it tries to open the
    // path as a directory.  SPIFFS_opendir(... path ...) always
    // succeeds, regardless of what path is, hence the need to
    // check that it is not a directory.

    if (!file || file.isDirectory()) {
        if (file) {
            file.close();
        }
        log_info("Missing config file " << path);
        return 0;
    }

    auto filesize = file.size();
    if (filesize == 0) {
        log_info("config file " << path << " is empty");
        return 0;
    }
    // log_debug("Configuration file has " << int(filesize) << " bytes");
    buffer = new char[filesize + 1];

    long pos = 0;
    while (pos < filesize) {
        auto read = file.read((uint8_t*)(buffer + pos), filesize - pos);
        if (read == 0) {
            break;
        }
        pos += read;
    }

    file.close();
    buffer[filesize] = 0;

    // log_debug("Read config file:\n" << buffer);

    if (pos != filesize) {
        delete[] buffer;

        log_error("Cannot read the config file");
        return 0;
    }
    return filesize;
}

char defaultConfig[] = "name: Default\nboard: None\n";

bool MachineConfig::load(const char* filename) {
    // log_info("Heap size before load config is " << uint32_t(xPortGetFreeHeapSize()));

    // If the system crashes we skip the config file and use the default
    // builtin config.  This helps prevent reset loops on bad config files.
    size_t             filesize = 0;
    char*              buffer   = nullptr;
    esp_reset_reason_t reason   = esp_reset_reason();
    if (reason == ESP_RST_PANIC) {
        log_debug("Skipping configuration file due to panic");
    } else {
        filesize = readFile(filename, buffer);
    }

    StringRange* input = nullptr;

    if (filesize > 0) {
        input = new StringRange(buffer, buffer + filesize);
        log_info("Using configuration file " << filename);

    } else {
        log_info("Using default configuration");
        input = new StringRange(defaultConfig);
    }
    // Process file:
    bool successful = false;
    try {
        // log_info("Heap size before parsing is " << uint32_t(xPortGetFreeHeapSize()));

        Configuration::Parser        parser(input->begin(), input->end());
        Configuration::ParserHandler handler(parser);

        // instance() is by reference, so we can just get rid of an old instance and
        // create a new one here:
        {
            auto& machineConfig = instance();
            if (machineConfig != nullptr) {
                delete machineConfig;
            }
            machineConfig = new MachineConfig();
        }
        config = instance();

        handler.enterSection("machine", config);

        log_info("Parsed configuration. Running after-parse tasks");

        // log_info("Heap size before after-parse is " << uint32_t(xPortGetFreeHeapSize()));

        try {
            Configuration::AfterParse afterParse;
            config->afterParse();
            config->group(afterParse);
        } catch (std::exception& ex) { log_info("Validation error: " << ex.what()); }

        log_info("Validating configuration");

        // log_info("Heap size before validation is " << uint32_t(xPortGetFreeHeapSize()));

        try {
            Configuration::Validator validator;
            config->validate();
            config->group(validator);
        } catch (std::exception& ex) { log_info("Validation error: " << ex.what()); }

        log_info("Validated configuration");

        // log_info("Heap size after configuation load is " << uint32_t(xPortGetFreeHeapSize()));

        successful = true;

    } catch (const Configuration::ParseException& ex) {
        sys.state      = State::ConfigAlarm;
        auto startNear = ex.Near();
        auto endNear   = (startNear + 10) > (buffer + filesize) ? (buffer + filesize) : (startNear + 10);

        StringRange near(startNear, endNear);
        log_error("Configuration parse error: " << ex.What() << " @ " << ex.LineNumber() << ":" << ex.ColumnNumber() << " near " << near);
    } catch (const AssertionFailed& ex) {
        sys.state = State::ConfigAlarm;
        // Get rid of buffer and return
        log_error("Configuration loading failed: " << ex.what());
    } catch (std::exception& ex) {
        sys.state = State::ConfigAlarm;
        // Log exception:
        log_error("Configuration validation error: " << ex.what());
    } catch (...) {
        sys.state = State::ConfigAlarm;
        // Get rid of buffer and return
        log_error("Unknown error while processing config file");
    }

    if (buffer) {
        delete[] buffer;
    }
    delete[] input;

    std::atomic_thread_fence(std::memory_order::memory_order_seq_cst);

    return successful;
}

MachineConfig::~MachineConfig() {
    delete _axes;
    delete _i2so;
    delete _coolant;
    delete _probe;
}
