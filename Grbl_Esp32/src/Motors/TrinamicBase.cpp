/*
    TrinamicBase.cpp

    Part of Grbl_ESP32
    2021 - Stefan de Bruijn

    Grbl is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    Grbl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "TrinamicBase.h"
#include "../Machine/MachineConfig.h"
#include <atomic>

namespace Motors {
    TrinamicBase* TrinamicBase::List = NULL;  // a static list of all drivers for stallguard reporting

    uint8_t TrinamicBase::get_next_index() {
        static uint8_t index = 1;  // they start at 1
        return index++;
    }

    // Prints StallGuard data that is useful for tuning.
    void TrinamicBase::readSgTask(void* pvParameters) {
        auto trinamicDriver = static_cast<TrinamicBase*>(pvParameters);

        TickType_t       xLastWakeTime;
        const TickType_t xreadSg = 200;  // in ticks (typically ms)
        auto             n_axis  = config->_axes->_numberAxis;

        xLastWakeTime = xTaskGetTickCount();  // Initialise the xLastWakeTime variable with the current time.
        while (true) {                        // don't ever return from this or the task dies
            std::atomic_thread_fence(std::memory_order::memory_order_seq_cst);  // read fence for settings
            if (sys.state == State::Cycle || sys.state == State::Homing || sys.state == State::Jog) {
                for (TrinamicBase* p = List; p; p = p->link) {
                    if (p->_stallguardDebugMode) {
                        //info_serial("SG:%d", p->_stallguardDebugMode);
                        p->debug_message();
                    }
                }
            }  // sys.state

            vTaskDelayUntil(&xLastWakeTime, xreadSg);

            static UBaseType_t uxHighWaterMark = 0;
#ifdef DEBUG_TASK_STACK
            reportTaskStackSize(uxHighWaterMark);
#endif
        }
    }
    
    // calculate a tstep from a rate
    // tstep = TRINAMIC_FCLK / (time between 1/256 steps)
    // This is used to set the stallguard window from the homing speed.
    // The percent is the offset on the window
    uint32_t TrinamicBase::calc_tstep(float speed, float percent) {
        double tstep = speed / 60.0 * config->_axes->_axis[axis_index()]->_stepsPerMm * (256.0 / _microsteps);
        tstep        = TRINAMIC_FCLK / tstep * percent / 100.0;

        return static_cast<uint32_t>(tstep);
    }


}
