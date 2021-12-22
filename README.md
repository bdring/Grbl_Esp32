

# Grbl (CNC Controller) For ESP32

## Get the Next Generation!

<img src="https://github.com/bdring/FluidNC/wiki/images/logos/FluidNC.svg" width="200">

The next generation of Grbl_ESP32 was such a massive upgrade we decided to change the name. It is called **FluidNC** and is [available here](https://github.com/bdring/FluidNC). Please check it out and give us a star. It is compatible with all Grbl_ESP32 hardware.

This version is only being maintained with existing features. All new features are targeted at FluidNC.

### Project Overview

Grbl_ESP32 started as a port of [Grbl](https://github.com/gnea/grbl) to the ESP32. The power of the ESP32 has allowed this firmware to grow far beyond the limitations of 8-bit AVR controllers. Here are some of the current features

- **Motors**
  - Control up to 6 coordinated axes (XYZABC)
  - Each axis can have 1 or 2 motors each for a total of 12 motors
  - Dual motors axes can optionally auto square using a home switch and independent control for each motor.
  - Motor drivers can be dynamically assigned to axes, so a 4 motor XYZA controller could be converted to a XYYZ (dual motor Y axis) without any hardware changes.
  - Step rates up to 120,000 per/second.
  - Trinamic (SPI controlled) stepper motors are supported including StealthChop, CoolStep and StallGuard modes. Sensorless homing can be used.
  - Unipolar stepper motor can be directly driven
  - RC hobby servos can be used as coordinated motors with acceleration and speed control. 
- **Peripherals**
  - Limit/Homing Switches with debouncing
  - User input buttons (hold, resume, reset)
  - Coolant control (Mist, Flood)
  - Z Probe (any axis)
  - Safety Door (open door safely retracts and stops spindle, can be resumed)
  - Additional I/O via shift register and I/O expanders supported (on dev branches)
- **Job Control**
  - Instant feed hold and resume
  - Feed rate override
  - Spindle speed override
- **Spindles**
  - PWM
  - RS485 Modus
  - DAC (analog voltage) 0-10V
  - Relay Based
  - RC type Brushless DC motors using low cost BESCs
  - Laser PWM with power/speed compensation
  - Easy to create custom spindles
- **Connectivity**
  - USB/Serial
  - Bluetooth/Serial Creates a virtual serial port on your phone or PC. Standard serial port applications can use Bluetooth.
  - WIFI
    - Creates its own access point or connects to yours.
    - Built in web server. The server has full featured CNC control app that will run on your phone or PC in a browser. No app required.
    - Telnet sending of gcode
    - Push notifications (like...job done, get a text/email)
    - OTA (over the air) firmware upgrades.
- SD card. Gcode can be loaded and run via WIFI.
- **Compatibility** 
  - Grbl_ESP32 is fully backward compatible with Grbl and can use all gcode senders.
- **Customizable**
  - Easy to map pins to any functions.  
  - Custom machines can be designed without touching the main code.   
  - Custom initialization
    - Kinematics
    - Custom homing
    - Tool changer sequences
    - Button macros (run gcode sequence, etc.)
    - Custom end of Job sequence
    - RTOS Real time operating system allows background monitoring and  control without affecting motion control performance
  
- Fast boot
  
  - It boots in about 2 seconds (unlike Raspberry Pi, Beagle Bone). Does not need to be formally shut down. Just kill the power

### Test Drive It

Grbl_ESP32 has a test drive mode. If you just compile it and load it onto an ESP32, it will create a virtual machine without any pins mapped. This allows you to safely test drive it without any attached hardware. Everything is functional including the WIFI and web user interface. Things like homing, that require feedback from actual switches cannot be done.

### Using It

Important compiling instructions are [in the wiki](https://github.com/bdring/Grbl_Esp32/wiki/Compiling-the-firmware)

The code should be compiled using the latest Arduino IDE. [Follow instructions here](https://github.com/espressif/arduino-esp32) on how to setup ESP32 in the IDE. The choice was made to use the Arduino IDE over the ESP-IDF to make the code a little more accessible to novices trying to compile the code.

I use the ESP32 Dev Module version of the ESP32. I suggest starting with that if you don't have hardware yet.

For basic instructions on using Grbl use the [gnea/grbl wiki](https://github.com/gnea/grbl/wiki). That is the Arduino version of Grbl, so keep that in mind regarding hardware setup. If you have questions ask via the GitHub issue system for this project.

### Roadmap

The roadmap is now [on the wiki](https://github.com/bdring/Grbl_Esp32/wiki/Development-Roadmap).

### Credits

The original [Grbl](https://github.com/gnea/grbl) is an awesome project by Sungeon (Sonny) Jeon. I have known him for many years and he is always very helpful. I have used Grbl on many projects. I only ported because of the limitation of the processors it was designed for. The core engine design is virtually unchanged.

The Wifi and WebUI is based on [this project.](https://github.com/luc-github/ESP3D-WEBUI)  

### Contribute

<img src="https://discord.com/assets/e05ead6e6ebc08df9291738d0aa6986d.png" width="8%"> There is a Discord server for the development this project. Ask for an invite

### FAQ

Start asking questions...I'll put the frequent ones here.



### <a name="donation"></a>Donation

This project requires a lot of work and often expensive items for testing. Please consider a safe, secure and highly appreciated donation via the PayPal link below or via the Github sponsor link at the top of the page.

[![](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=TKNJ9Z775VXB2)
