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

...coming soon

