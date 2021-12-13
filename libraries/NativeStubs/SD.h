#pragma once
#include <SPI.h>
#include <FS.h>

#if EPOXY_DUINO_VERSION < 10000
class SDFS : public FS {
#else
class SDFS {
#endif
public:
    bool   begin(uint8_t sspin, SPIClass& spi, int freq, const char* mountpoint, int n_files) { return false; };
    void   end() {}
    size_t cardSize() { return 0; }
};

extern SDFS SD;
