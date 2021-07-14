#include <TMCStepper.h>

// Implementation of weak functions in libraries
// The compiler use for native compilation doesn't support
// __attribute__ ((weak))
void TMC2130Stepper::switchCSpin(bool state) {
    // digitalWrite(_pinCS, state);
}
void TMC2130Stepper::setSPISpeed(uint32_t speed) {
    // spi_speed = speed;
}
void TMC2130Stepper::write(uint8_t addressByte, uint32_t config) {}

uint32_t TMC2130Stepper::read(uint8_t addressByte) {
    return 0UL;
}
