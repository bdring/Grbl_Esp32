// Pin assignments (or lack thereof) for testing Grbl_ESP32.

// It creates a basic 3 axis machine without actually driving
// I/O pins.  Grbl will report that axes are moving, but no physical
// motor motion will occur.

// This can be uploaded to an unattached ESP32 or attached to
// unknown hardware with no risk of pins trying to output signals
// into a short, etc that could dmamge the ESP32

// It can also be used to get the basic program running so OTA
// (over the air) firmware loading can be done.


#define MACHINE_NAME "MACHINE_DEFAULT - Demo Only No I/O!"

#define LIMIT_MASK 0  // no limit pins
