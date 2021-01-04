## Dynamixel2

This allows Dynamixel Servo motors to be used as an axis motor. This document was written assuming XL430-250T servos were used, but other servo types that use [Robotis Protocol 2](https://emanual.robotis.com/docs/en/dxl/protocol2/) can probably be used (not tested).

They can be used in place of any lettered axis, but only one per axis. These servos are limited to a single 360° of rotation. This is mapped to the MaxTravel of the axis. If you want map 300mm of travel to an X axis servo, you set $X/MaxTravel=300.

This sets a range in machine space. You are allowed to zero the axis anywhere in that travel. If you do a move that exceeds the travel, the servo will move to the end of of travel and stay there. The axis position will still be running in Grbl, but the servo will stop. You can use $Limits/Soft=On to alarm if the range is exceeded.

You should set the the axis speed and acceleration to what the servo can handle. If those settings are too high, the motor will still move, but will lag behind where Grbl thinks it is.

The resolution of the range is sepcific to the servo type. This is currently fixed at this valu

const int DXL_COUNT_MAX = 4095;

## Machine Definition

```
#define DYNAMIXEL_TXD           gpio.4
#define DYNAMIXEL_RXD           gpio.13
#define DYNAMIXEL_RTS           gpio.17

$X/Motor/Address=1
$Y/Motor/Address=2
$Z/Motor/Address=3

#define SERVO_TIMER_INTERVAL    75   // milliseconds (an override...not required)

$<axis>/Motor/Cal/Min=0.325    
$<axis>/Motor/Cal/Max=0.725

```

You need to specify the TXD, RXD and RTS pins you want to use for the half duplex communications bus.

The `SERVO_TIMER_INTERVAL` sets the time in milliseconds between updates. At each interval 1 message per servo is sent. If you try to update too fast you will see errors reported to the USB/Serial port. 75ms seems like a good rate for 3 servos. Adjust per your count.

You assign servos to axes with a definition like `$<axis>/Motor/Address=1` The servos should be programmed with unique IDs using Dynamixel software.

You can limit the servo rotational range of travel using `$<axis>/Motor/Cal/Min` and `$<axis>/Motor/Cal/Max` The value is the percentage of the  full count. The full range of an XT430-250T servo is 0-4095.

You can reverse the motor by making Min higher than Max

## Homing

Dynamixel servos, don't typically need to be homed. This is because they can determine their location from their encoders and report it to Grbl_ESP32. If you do home them they will travel as fast as they can to the end of travel in the direction controlled by $Homing/DirInvert= setting.

## Manual Moves

Whenever Grbl has the motors disabled, you can manually home the servos. Since they are closed loop and report current position, they will report back where they have been moved to and update Grbl. This is controlled by the $Stepper/IdleDelay setting. 

The resolution is 4096 units per 360°. This is likely to be a lot coarser than Grbl's internal representation. Therefore if you send a servo to 15. It may change a little after being update from the servo when disabled. The servo may report back 14.8 after being sent to 15.







