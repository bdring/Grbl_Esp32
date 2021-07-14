#pragma once
#include <SPI.h>
#include <FS.h>

class SDFS : public FS {
public:
    bool   begin(uint8_t sspin, SPIClass& spi, int freq, const char* mountpoint, int n_files) { return false; };
    void   end() {}
    size_t cardSize() { return 0; }
};

extern SDFS SD;
