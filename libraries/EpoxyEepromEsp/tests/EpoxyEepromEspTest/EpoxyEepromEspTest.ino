#line 2 "EpoxyEepromEspTest.ino"

#include <Arduino.h>
#include <AUnit.h>
#include <EpoxyEepromEsp.h>

using aunit::TestRunner;

#define EEPROM EpoxyEepromEspInstance

//---------------------------------------------------------------------------

test(writeAndReadTest) {
  EEPROM.begin(1024);
  assertEqual((size_t) 1024, EEPROM.length());

  // Write using put()
  uint32_t output = 0x04030201;
  EEPROM.put(0, output);

  // Write at the end of the EEPROM space, using write() and operator[].
  EEPROM.write(1020, 1);
  EEPROM.write(1021, 2);
  EEPROM[1022] = 3;
  EEPROM[1023] = 4;

  // Save to disk
  EEPROM.commit();
  EEPROM.end();

  // Read back again.
  EEPROM.begin(1024);

  // Read using get()
  uint32_t input = 0;
  EEPROM.get(0, input);
  assertEqual(output, input);

  // Read using read() and operator[].
  assertEqual(1, EEPROM.read(1020));
  assertEqual(2, EEPROM.read(1021));
  assertEqual(3, EEPROM[1022]);
  assertEqual(4, EEPROM[1023]);

  EEPROM.end();
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // needed for Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
