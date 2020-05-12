/*
    motor_class_test.h
    Part of Grbl_ESP32

    Pin assignments for a 4-axis machine using Triaminic drivers
    in daisy-chained SPI mode.
    https://github.com/bdring/4_Axis_SPI_CNC

    2019    - Bart Dring
    2020    - Mitch Bradley

    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Grbl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.

    Standard Steppers


    Trinamic Motors

    Setup Pins
        #define X_STEP_PIN          GPIO_NUM_xx
        #define X_DIRECTION_PIN     GPIO_NUM_xx
        #define X_CS_PIN            GPIO_NUM_xx

    Setup Driver Type
        #define X_TRINAMIC_DRIVER       2130 (driver number)       // Says this axis is a trinamic driver
        #define X_RSENSE                TMC2130_RSENSE_DEFAULT     // The current sense reistor used

    Options
        #define DAISY CHAIN  // TODO
        #define TRINAMIC_RUN_MODE           TRINAMIC_RUN_MODE_COOLSTEP // (default) or TRINAMIC_RUN_MODE_STEALTHCHOP
        #define TRINAMIC_HOMING_MODE        TRINAMIC_HOMING_NONE // (default) or TRINAMIC_HOMING_STALLGUARD
        #define ENABLE_STALLGUARD_TUNING    // This enables you to use the $32=1 (laser mode) to display realtime stallguard feedback
                                            // This feedback should not be displayed will running normal CNC jobs (to much overhead)                    

    Hardware
        For StallGuard you need a path from the diag1 pin to a limit switch input. Do not also have a switch on
        that circuit.

    Stallguard
        StallGuard is a feature of Trinamic drivers that can sense the load on a motor. It can use this to detect when a 
        motor is about to or has stalled. It can output a signal on one of its diagnostic pins and that can be used
        to trigger a limit switch in Grbl_ESP32.

        Stallguard does this by looking at the efficiency of the motor. It looks at how much energy 
        sent to the motor and how much gets returned to the driver. A running, but lightly loaded motor returns more energy than a motor
        that is highly loaded.

        The problem is, the motor motion is not the source of all inefficiencies. Wiring length and gauge, motor size, motor design and speed all 
        can add inefficiencies. You need to tell the driver what ratio of energy sent and enegrgy returned indicates a stall on your machine. Also, a 
        little tuning of the speeds, etc will help.

        The motors have 2 primary modes of operation. The first is StealthChop. This is a the super quiet mode the drivers are famous for. The
        other mode is CoolStep. CoolStep is the mode where the efficiency can be monitored. It uses that to determine the power the motor needs.
        If it looks like it is under a high load it boosts the power. This will allow you to raise the current, but typically not have the temperature
        go up as much, because that higher current is rarely fully used. Therefore, the motor must be in CoolStep mode to use Stallguard. Grbl_ESP32
        will switch to this mode during homing is you are using StallGuard homing.

        Is Stallguard right for your machine? Stallguard is only accurate to about 1-2 full steps of your motor. Depending on your machine. that
        could be 0.25mm (0.01"). A switch will generally be more accurate. This may be fine for 3D printer X,Y axes, pen plotters or lasers, but may 
        not be OK for mills or routers, where repeated accuracy is very important.
    
    
    Tuning
        Compiling: Make sure #define ENABLE_STALLGUARD_TUNING was enabled. You also need to have #define TRINAMIC_RUN_MODE TRINAMIC_RUN_MODE_COOLSTEP
        to during tuning. You can reset to StealthChop after tuning.

        Determine Homing Speed: Stallguard will not work effectively at low or high speeds. You need to
        determine what speed it is most effective. Do this by doing long moves at various speed with 
        "G1 Fxxxx X200" type moves. Where Fxxxx is the speed you want to try. Try on the StallGuard display
        with $32=1. When you move you will see lines like "". 
        
        As you try to stop the motor, you will see the SG number drop. If it drops to zero you should see the 
        xxx number change from 0 to 1 (Stall Detected). Play with the speed until you see that number change a lot when adding a load.

        Try Stallguard Values.

        Adjust the $160 - $165 values until you can effective detect stalls. In my experiance a typical value is between about 5 and 30. 

        Reboot with $32=1
        
        
*/

#define MACHINE_NAME "MotorClass Test TMC2130 Pen board"

//#undef USE_RMT_STEPS



/* 
========================================================================
#ifdef N_AXIS
        #undef N_AXIS
#endif
#define N_AXIS 3 // can be 3 or 4. (if 3 install bypass jumper next to the A driver)


#define SPINDLE_TYPE                SPINDLE_TYPE_NONE

#define ENABLE_STALLGUARD_TUNING

#define TRINAMIC_RUN_MODE           TRINAMIC_RUN_MODE_STEALTHCHOP
//#define TRINAMIC_RUN_MODE           TRINAMIC_RUN_MODE_COOLSTEP
//#define TRINAMIC_RUN_MODE           TRINAMIC_RUN_MODE_STALLGUARD

#define TRINAMIC_HOMING_MODE        TRINAMIC_HOMING_STALLGUARD

#define STEPPERS_DISABLE_PIN    GPIO_NUM_13

#define X_STEP_PIN              GPIO_NUM_12
#define X_DIRECTION_PIN         GPIO_NUM_26
#define X_TRINAMIC_DRIVER       2130
#define X_CS_PIN                GPIO_NUM_17  //chip select
#define X_RSENSE                TMC2130_RSENSE_DEFAULT

#define Y_STEP_PIN              GPIO_NUM_14
#define Y_DIRECTION_PIN         GPIO_NUM_25
#define Y_TRINAMIC_DRIVER       2130
#define Y_CS_PIN                GPIO_NUM_16  //chip select
#define Y_RSENSE                TMC2130_RSENSE_DEFAULT

#define X_LIMIT_PIN             GPIO_NUM_32
#define Y_LIMIT_PIN             GPIO_NUM_4
#define LIMIT_MASK              B11

======================================================
*/


// =============== Unipolar Test =================

#define N_AXIS 2

#define SPINDLE_TYPE                SPINDLE_TYPE_NONE

#define X_UNIPOLAR
#define X_PIN_PHASE_0 GPIO_NUM_13
#define X_PIN_PHASE_1 GPIO_NUM_21
#define X_PIN_PHASE_2 GPIO_NUM_16
#define X_PIN_PHASE_3 GPIO_NUM_22

#define Y_UNIPOLAR
#define Y_PIN_PHASE_0 GPIO_NUM_25
#define Y_PIN_PHASE_1 GPIO_NUM_27
#define Y_PIN_PHASE_2 GPIO_NUM_26
#define Y_PIN_PHASE_3 GPIO_NUM_32

#define DEFAULT_X_STEPS_PER_MM 10
#define DEFAULT_Y_STEPS_PER_MM 10
#define DEFAULT_Z_STEPS_PER_MM 100.0 // This is percent in servo mode

#define DEFAULT_X_MAX_RATE 5000.0   // mm/min
#define DEFAULT_Y_MAX_RATE 5000.0   // mm/min
#define DEFAULT_Z_MAX_RATE 200000.0 // mm/min

#define DEFAULT_X_ACCELERATION (500.0 * 60 * 60) // 10*60*60 mm/min^2 = 10 mm/sec^2
#define DEFAULT_Y_ACCELERATION (500.0 * 60 * 60) // 10*60*60 mm/min^2 = 10 mm/sec^2
#define DEFAULT_Z_ACCELERATION (500.0 * 60 * 60)

#define DEFAULT_X_MAX_TRAVEL 120.0   // mm NOTE: Must be a positive value.
#define DEFAULT_Y_MAX_TRAVEL 20000.0 // mm NOTE: Must be a positive value.
#define DEFAULT_Z_MAX_TRAVEL 10.0    // This is percent in servo mode