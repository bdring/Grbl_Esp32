#pragma once

// Grbl setting that are common to all machines
// It should not be necessary to change anything herein

#ifndef GRBL_SPI_FREQ
// You can override these by defining them in a board file.
// To override, you must set all of them
//-1 means use the default board pin
#    define GRBL_SPI_SS -1
#    define GRBL_SPI_MOSI -1
#    define GRBL_SPI_MISO -1
#    define GRBL_SPI_SCK -1
#    define GRBL_SPI_FREQ 4000000
#endif

// ESP32 CPU Settings
const uint32_t fTimers = 80000000;  // a reference to the speed of ESP32 timers

// =============== Don't change or comment these out ======================
// They are for legacy purposes and will not affect your I/O

const int STEP_MASK = B111111;

const int PROBE_MASK = 1;
