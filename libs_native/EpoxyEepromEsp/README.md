# EpoxyEepromEsp

An implementation of `EEPROM` that runs on Linux (or other POSIX-like system)
using EpoxyDuino (https://github.com/bxparks/EpoxyDuino). There are at least two
versions of the `EEPROM` API:

* AVR flavor: AVR, STM32, Teensy, and potentially others
    * https://github.com/arduino/ArduinoCore-avr/blob/master/libraries/EEPROM/src/EEPROM.h
* ESP flavor: ESP8266 and ESP32,
    * https://github.com/esp8266/Arduino/blob/master/libraries/EEPROM/EEPROM.h

This library implements the **ESP** flavor of the EEPROM API. The EEPROM
contents are buffered in memory, then saved to a file called `epoxyeepromdata`
in the current directory upon the `EEPROM.commit()` or `EEPROM.end()` methods.

See [EpoxyEepromAvr](../EpoxyEepromAvr/) for the AVR version.

## Usage

### EEPROM API

This library provides most of the functions listed in
[EEPROM.h](https://github.com/esp8266/Arduino/blob/master/libraries/EEPROM/EEPROM.h)
in the Arduino ESP8226 Core.

### Makefile

When using the EpoxyDuino Makefile, the `EpoxyEepromEsp` library must be added to
the `ARDUINO_LIBS` parameter, like this:

```
APP_NAME := EpoxyEepromEspTest
ARDUINO_LIBS := EpoxyEepromEsp ...
MORE_CLEAN := more_clean
include ../../../../EpoxyDuino.mk
```

### Sample Code

```C++
#include <EpoxyEepromEsp.h>
#define EEPROM EpoxyEepromEspInstance

void write() {
  int address = 0;
  uint8_t value = 10;

  // write() method
  EEPROM.write(address, value);

  // operator[]
  EEPROM[address+1] = value+1;

  // commit() or end() required to persist to disk
  EEPROM.commit();
}

void read() {
  int address = 0;

  // read()
  uint8_t value = EEPROM.read(address);

  // operator[]
  value = EEPROM[address];
}

void detectAPI() {
  #if EPOXY_DUINO_EPOXY_EEPROM_ESP
    #warning Using EpoxyEepromEsp version of EEPROM
    EEPROM.begin(...);
    EEPROM.write(...);
    EEPROM.commit(...);
    EEPROM.read(...);
  #endif
}

void setup() {
  const int SIZE = 1024;

  // begin() required
  EEPROM.begin(SIZE);
  ...
}

void loop() {
}
```

## Difference between ESP and AVR APIs

### AVR Flavor

The AVR version of `EEPROM` provides 2 write functions:
```C++
EEPROM.write(address, value);
EEPROM.update(address, value);
```
where the `update()` only writes to the EEPROM if the new value is different
than the old value stored in the EEPROM. This saves wear on the EEPROM.

The `EEPROM.begin()` and `EEPROM.end()` methods return iterators, which allows
looping for the content of the EEPROM like this:

```C++
for (EEPtr p = EEPROM.begin(); p != EEPROM.end(); p++) {
  ...
}
```

### ESP Flavor

The ESP8266 and ESP32 versions provide only the `write()` function:

```C++
EEPROM.write(address, value)
```

The `begin()` and `end()` methods mean completely different things on the ESP
processors:

```C++
EEPROM.begin(size);
```
initializes the EEPROM storage space. The data written to the EEPROM using
the `write()` method is buffered in memory until one of:

```C++
EEPROM.commit();
EEPROM.end();
```
is executed.

## Environment Variable

By default, the content of the `EEPROM` is saved to a file named
`epoxyeepromdata` in the current directory. You can change this by setting the
`EPOXY_EEPROM_DATA` environment variable:

```
$ export EPOXY_EEPROM_DATA=/tmp/epoxyeepromdata
```
