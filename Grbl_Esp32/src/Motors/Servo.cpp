/*
    Servo.cpp

    This is a base class for servo-type motors - ones that autonomously
    move to a specified position, instead of being moved incrementally
    by stepping.  Specific kinds of servo motors inherit from it.

    Part of Grbl_ESP32

    2020 -	Bart Dring

    The servo's travel will be mapped against the axis with $X/MaxTravel

    The rotation can be inverted with by $Stepper/DirInvert

    Homing simply sets the axis Mpos to the endpoint as determined by $Homing/DirInvert

    Calibration is part of the setting (TBD) fixed at 1.00 now

    Grbl_ESP32 is free software: you can redistribute it and/or modify
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

#include "Servo.h"

namespace Motors {
    Servo* Servo::List = NULL;

    Servo::Servo(uint8_t axis_index) : Motor(axis_index) {
        link = List;
        List = this;
    }

    void Servo::startUpdateTask() {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Servo Update Task Started");
        if (this == List) {
            xTaskCreatePinnedToCore(updateTask,         // task
                                    "servoUpdateTask",  // name for task
                                    4096,               // size of task stack
                                    NULL,               // parameters
                                    1,                  // priority
                                    NULL,               // handle
                                    SUPPORT_TASK_CORE   // core
            );
        }
    }

    void Servo::updateTask(void* pvParameters) {
        TickType_t       xLastWakeTime;
        const TickType_t xUpdate = SERVO_TIMER_INTERVAL;  // in ticks (typically ms)
        auto             n_axis  = number_axis->get();

        xLastWakeTime = xTaskGetTickCount();  // Initialise the xLastWakeTime variable with the current time.
        vTaskDelay(2000);                     // initial delay
        while (true) {                        // don't ever return from this or the task dies
            for (Servo* p = List; p; p = p->link) {
                p->update();
            }

            vTaskDelayUntil(&xLastWakeTime, xUpdate);

            static UBaseType_t uxHighWaterMark = 0;
#ifdef DEBUG_TASK_STACK
            reportTaskStackSize(uxHighWaterMark);
#endif
        }
    }

}
