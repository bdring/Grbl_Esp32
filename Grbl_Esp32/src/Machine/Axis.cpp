#include "Axis.h"

#include <cstring>

namespace Machine {
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

            handler.section(tmp, _gangs[g], g);
        }
    }

    void Axis::afterParse() {
        for (size_t i = 0; i < MAX_NUMBER_GANGED; ++i) {
            if (_gangs[i] == nullptr) {
                _gangs[i] = new Gang(i);
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
}
