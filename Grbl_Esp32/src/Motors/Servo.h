#pragma once

/*
    Servo.h

    This is a base class for servo-type motors - ones that autonomously
    move to a specified position, instead of being moved incrementally
    by stepping.  Specific kinds of servo motors inherit from it.

    Part of Grbl_ESP32

    2020 -	Bart Dring

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

#include "Motor.h"

namespace Motors {
    class Servo : public Motor {
    public:
        Servo(uint8_t axis_index);
#if 0
        // Overrides for inherited methods
        void init() override;
        void read_settings() override;
        bool set_homing_mode(bool isHoming) override;
        void set_disable(bool disable) override;
#endif
        virtual void update() = 0;  // This must be implemented by derived classes

    protected:
        // Start the servo update task.  Each derived subclass instance calls this
        // during init(), which happens after all objects have been constructed.
        // startUpdateTask() ignores all such calls except for the last one, where
        // it starts the task.
        void startUpdateTask();

    private:
        // Linked list of servo instances, used by the servo task
        static Servo* List;
        Servo*        link;
        static void   updateTask(void*);
    };
}
