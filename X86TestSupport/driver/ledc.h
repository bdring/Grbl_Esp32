#pragma once

#include <cstdint>

void pinMatrixOutAttach(uint8_t pin, uint8_t function, bool invertOut, bool invertEnable);
void pinMatrixOutDetach(uint8_t pin, bool invertOut, bool invertEnable);
