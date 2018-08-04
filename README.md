

# Grbl (CNC Controller) For ESP32

![ESP32](http://www.buildlog.net/blog/wp-content/uploads/2018/07/20180718_175943.jpg)

### Status: Functional Beta (See minor issues below)



### Project Overview

This is a port of [Grbl](https://github.com/gnea/grbl) for the ESP32. The ESP32 is potentially a great target for Grbl for the following reasons

- **Faster** - Faster step rates and additional features could be added
- **Lower Cost** - 
- **Small footprint** -
- **More Flash and RAM** - A larger planner buffer could be used and more features could be added.
- **I/O** - It has just about the same number of pins as an Arduino UNO, the original target for Grbl
- **Peripherals** - It has more timers and advanced features than an UNO.  These can also be mapped to pins more flexibly.
- **Connectivity** - Bluetooth and WiFi built in.

### Issues / Changes

1. **Direction pin delay** - Not implemented yet. Some drivers require a couple of microseconds after the direction pin is set before you start the step pulse. The original plan was to [use the RMT feature](http://www.buildlog.net/blog/?s=rmt), but that has issues when trying to use it in an Interrupt.  **This is typically a option in Grbl that is not used.**
2. **Limit Switch debouncing** is not supported yet. It does not seem
   to be a problem on my test rigs. It might be better to us an R/C
   filter for now.
3. **Floating inputs** On older ESP32s, some pins have nonfunctional
   internal pullups.   If you see a spew of messages about the Door
   Pin, try fitting pullup resistor to the button and probe inputs.

### Using It

The code should be compiled using the latest Arduino IDE. [Follow instructions here](https://github.com/espressif/arduino-esp32) on how to setup ESP32 in the IDE. The choice was made to use the Arduino IDE over the ESP-IDF to make the code a little more accessible to novices trying to compile the code.

I use the NodeMCU 32S version of the ESP32. I suggest starting with that if you don't have hardware yet.

For basic instructions on using Grbl use the [gnea/grbl wiki](https://github.com/gnea/grbl/wiki). That is the Arduino version of Grbl, so keep that in mind regarding hardware setup. If you have questions ask via the GitHub issue system for this project.

Note: Unlike Grbl on Arduinos, the controller does not reboot when you connect to it via USB. This may cause confusion on some older senders. Most modern Grbl senders will send a reset (Ctrl-X or 0x18) instead of relying on the reboot. I do not plan on changing this. It is better to do reboot when a connection is opened.

### TODO List

- RMT. The RMT feature is a ideal for direction and step features, but apparently has issues working in interrupts. See [this forum post](https://www.esp32.com/viewtopic.php?f=19&t=6397&hilit=grbl) and [this blog post](http://www.buildlog.net/blog/?s=rmt). It would be great to get it working.
- Add spindle enable and direction. 
- [Bluetooth](https://github.com/bdring/Grbl_Esp32/issues/3) - Add it so phones and PCs can use it to stream gcode. It would be great if it looks like a bluetooth serial port, that helps with compatibility with existing apps. ([Android Grbl Controller](https://play.google.com/store/apps/details?id=in.co.gorest.grblcontroller&hl=en_US) is best!)

### Credits

The original [Grbl](https://github.com/gnea/grbl) is an awesome project by Sungeon (Sonny) Jeon. I have known him for many years and he is always very helpful. I have used Grbl on many projects. I only ported because of the limitation of the processors it was designed for. The core engine design is virtually unchanged.

### Contribute

I would love to have help with this project. There are many things that need to be improved and added, especially BlueTooth and/or WiFi. If you need hardware to test with, I might be able to help with a test PCB.

![](http://www.buildlog.net/blog/wp-content/uploads/2018/07/slack_hash_128.png)   [Slack channel](https://join.slack.com/t/buildlog/shared_invite/enQtNDA1ODM5MzI3MjE2LWYxNzMwZmNmMWVhYmUzMDdiYWQxMjk2MWQ1NzJhYzc2Mjg5NmRjMWI2MmM3OGE4M2JiZWQ2MThjMjQ3Y2U2OTE) for this project. 

### FAQ

Start asking questions...I'll put the frequent ones here.





