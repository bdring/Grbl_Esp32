# Pin Definitions

Pin definitions can be done several way.

Most require a reboot, **$System/Control=Restart**, to take effect

#### Machine Definition file

They are set with **#defines** in your machine definition file (your_def.h). They act as the default settings. You can override them with run time settings. **$RST=$** will restore all your pins to the defaults. The default of any pin not set in this file is **UNDEFINED_PIN** and disables the feature it was assigned to.

#### $ Settings

You can configure any pin at run time with $ settings. These override the values you put in your machine definition file.

#### Config File

## Input pins

### Pin number (required)

#### GPIO

GPIO is on the main CPU. You specify it with **gpio.nn** where nn is the GPIO pin number.

#### I2SO cannot be used for inputs

### Pull Up/Down Resistors (optional)

The microcontroller can add pull up/down to most pins. You specify this by adding **:pu** or **:pd**. The default is none (floating).

### Active State (optional)

The active state of an input can be either high or low. You can set that by adding **:high** or **:low**. The default is active high if you do not specify.

### Examples

```C++
// Machine definition file
#define	X_LIMIT_PIN	"gpio.14"         // X limit on GPIO_NUM_14. Floating, Active high
#define Y_LIMIT_PIN "gpio.15:pu"      // Y limit on GPIO_NUM_15. Pull up, Active ligh
#define Z_LIMIT_PIN "gpio.16:pd:low"  // Z limit on GPIO_NUM_16. Pull down, Active low

// $ Settings
$X/Limit/Pin=gpio.14     	          // X limit on GPIO_NUM_14. Floating, Active high
$Y/Limit/Pin=gpio.15:pu               // Y limit on GPIO_NUM_15. Pull up, Active ligh
$Z/Limit/Pin=gpio.16:pd:low           // Z limit on GPIO_NUM_16. Pull down, Active low
```

## Digital Output Pins

### Pin number (required)

#### GPIO

GPIO is on the main CPU. You specify it with **gpio.nn** where nn is the GPIO pin number.

#### I2SO

I2SO is a external chip on your control PCB. If you have it, you specify it with **i2so:nn** where nn is the output number.

### Active State (optional)

The state of an input can be either high or low. You can set that by adding **:high** or **:low**. The default is active high if you do not specify. 

### Examples

~~~C++
// Machine Definition File
#define	COOLANT_MIST_PIN	"gpio.2"
#define USER_DIGITAL_PIN_0  "gpio.26:low"


$Coolant/Flood/Pin=gpio.2
$User/Digital/0/Pin=gpio.26:low
$X/Direction/Pin=i2so.1:low               // Change direction of X motor
~~~

## PWM Output Pins

...coming soon

## Spindles



#### 0-10V (Tested...Works)

```
#define SPINDLE_TYPE      SpindleType::PWM
#define SPINDLE_OUTPUT_PIN    "gpio.14"
#define SPINDLE_ENABLE_PIN    "gpio.12" // optional
#define SPINDLE_DIRECTION_PIN "gpio.15" // optional
#define SPINDLE_FORWARD_PIN   "gpio.13" // optional
#define SPINDLE_REVERSE_PIN   "gpio.26" // optional

$Spindle/Type=10V
$Spindle/Output/Pin=gpio.14
$Spindle/Enable/Pin=gpio.12 
$Spindle/Direction/Pin=gpio.15
$Spindle/Forward/Pin=gpio.13
$Spindle/Reverse/Pin=gpio.26

$Spindle/Enable/Pin=gpio.12:low // to invert enable pin
```

#### BESC (Tested...Works)

```C++
#define  SPINDLE_TYPE          SpindleType::BESC
##define SPINDLE_OUTPUT_PIN    "gpio.14"

$Spindle/Type=BESC
$Spindle/Output/Pin=gpio.14
```

#### DAC (Tested...Works!)

```
#define SPINDLE_TYPE      SpindleType::DAC
#define SPINDLE_OUTPUT_PIN    "gpio.26"  // 25 or 25 only
#define SPINDLE_ENABLE_PIN    "gpio.14"
#define SPINDLE_DIRECTION_PIN "gpio:13"

$Spindle/Type=DAC
$Spindle/Output/Pin=gpio.26
$Spindle/Enable/Pin=gpio.14
$Spindle/Direction/Pin=gpio.13
```

#### RELAY (Tested...Works!)

```
#define SPINDLE_TYPE      SpindleType::RELAY
#define SPINDLE_OUTPUT_PIN    "gpio.14"
#define SPINDLE_ENABLE_PIN    "gpio.12" // optional
#define SPINDLE_DIRECTION_PIN "gpio:15" // optional

$Spindle/Type=RELAY
$Spindle/Output/Pin=gpio.14
$Spindle/Enable/Pin=gpio.12 
$Spindle/Direction/Pin=gpio.15
```

#### LASER (Tested ...Works!)

```
#define SPINDLE_TYPE      SpindleType::LASER
#define LASER_OUTPUT_PIN    "gpio.14"
#define LASER_ENABLE_PIN    "gpio.12" // optional

$Spindle/Type=RELAY
$Spindle/Output/Pin=gpio.14
$Spindle/Enable/Pin=gpio.12
```



#### PWM (Tested...Works)

```
#define SPINDLE_TYPE      SpindleType::PWM
#define SPINDLE_OUTPUT_PIN    "gpio.14"
#define SPINDLE_ENABLE_PIN    "gpio.12" // optional
#define SPINDLE_DIRECTION_PIN "gpio.15" // optional

$Spindle/Type=RELAY
$Spindle/Output/Pin=gpio.14
$Spindle/Enable/Pin=gpio.12 
$Spindle/Direction/Pin=gpio.15

$Spindle/Enable/Pin=gpio.13:low   // This inverts the enable pin
$Spindle/Output/Pin=gpio.14:low   // This inverts pwm signal
```



#### HUANYANG 

```
#define SPINDLE_TYPE      SpindleType::HUANYANG
```



#### HY2

```
#define SPINDLE_TYPE      SpindleType::HY2
```

------

## Motors

#### StepStick/External

```

```

#### Trinamic SPI 2130/5160 (Works!)

```
#define TRINAMIC_RUN_MODE           TrinamicMode :: StallGuard
#define TRINAMIC_HOMING_MODE        TrinamicMode ::StallGuard

#define X_MOTOR_TYPE            MotorType::TMC2130
#define X_STEP_PIN              "gpio.12"
#define X_DIRECTION_PIN         "gpio.26"
#define X_CS_PIN                "gpio.17"  //chip select
#define X_DRIVER_RSENSE         0.11f

#define Y_MOTOR_TYPE            MotorType::TMC2130
#define Y_STEP_PIN              "gpio.14"
#define Y_DIRECTION_PIN         "gpio.25"
#define Y_CS_PIN                "gpio.16"  //chip select
#define Y_DRIVER_RSENSE         0.11f

$X/Trinamic/CS/Pin=gpio.17
$Y/Trinamic/CS/Pin=gpio.16
$Trinamic/HomingMode=StallGuard
$Trinamic/RunMode=StealthChop
```

#### Trinamic UART 2209

```

```



#### RCServo (Tested....Works!)

```
#define Z_MOTOR_TYPE             MotorType::RCServo
#define Z_RCSERVO_PIN             "gpio.27"


```

#### Dynamixel2

```

```



------

## Creating a Machine Definition File

### Machine Name

It is a good idea to give a name to your machine definition. This can be helpful for support when viewing startup messages. It should contain the type of hardware you are using and a simple description of your machine.

#define MACHINE_NAME "6 Pack MPCNC VFD Spindle"

### I/O Expanders

### Motors

Motors in Grbl refer to devices used on an axis. For each motor, you need to specify the type. The valid types are.

- None
- StepStick
- External
- TMC2130
- TMC5160
- TMC2208
- TMC2209
- RCServo
- Dynamixel

Specify them like this.

`#define SPINDLE_TYPE MotorType::StepStick`



