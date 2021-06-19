#pragma once

// Grbl setting that are common to all machines
// It should not be necessary to change anything herein

const int32_t GRBL_SPI_FREQ = 4000000;

    // ESP32 CPU Settings
const uint32_t fTimers = 80000000;  // a reference to the speed of ESP32 timers

// =============== Don't change or comment these out ======================
// They are for legacy purposes and will not affect your I/O

const int STEP_MASK  = 0x3F;  // 111111
const int PROBE_MASK = 1;
