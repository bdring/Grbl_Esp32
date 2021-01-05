#define MACHINE_NAME "DXL Test"

#define N_AXIS 3

#define X_MOTOR_TYPE MotorType::Dynamixel

#define DYNAMIXEL_TXD "gpio.4"
#define DYNAMIXEL_RXD "gpio.13"
#define DYNAMIXEL_RTS "gpio.17"

#define X_MOTOR_TYPE MotorType::Dynamixel
#define Y_MOTOR_TYPE MotorType::Dynamixel
#define Z_MOTOR_TYPE MotorType::Dynamixel
#define A_MOTOR_TYPE MotorType::Dynamixel

/*
$X/Motor/Address=1
$Y/Motor/Address=2
$Z/Motor/Address=3
$A/Motor/Address=4

$X/Motor/Cal/Min=0.325
$Y/Motor/Cal/Min=0.325
$Z/Motor/Cal/Min=0.325
$A/Motor/Cal/Min=0.325

$X/Motor/Cal/Max=0.742
$Y/Motor/Cal/Max=0.742
$Z/Motor/Cal/Max=0.742
$A/Motor/Cal/Max=0.742


$X/Home/Mpos=10
$Y/Home/Mpos=10
$Z/Home/Mpos=10
$A/Home/Mpos=10

$X/MaxTravel=10
$Y/MaxTravel=10
$Z/MaxTravel=10
$A/MaxTravel=10



*/

#define DEFAULT_X_MAX_TRAVEL 10  //
#define DEFAULT_Y_MAX_TRAVEL 10  //
#define DEFAULT_Z_MAX_TRAVEL 10  //

#define DEFAULT_X_HOMING_MPOS 0
#define DEFAULT_Y_HOMING_MPOS 0
#define DEFAULT_Z_HOMING_MPOS 0
