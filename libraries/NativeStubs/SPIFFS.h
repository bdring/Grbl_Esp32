#pragma once
#include <Arduino.h>
#if EPOXY_DUINO_VERSION < 10000

#    include <FS.h>

namespace fs {
    class SPIFFSFS : public FS {
    public:
        SPIFFSFS() = default;

        ~SPIFFSFS() = default;

        bool begin(bool formatOnFail, const char* basePath, uint8_t maxOpenFiles, const char* partitionLabel) { return true; }

        void end() {}

        bool format() { return true; }

        size_t totalBytes() { return 0; }

        size_t usedBytes() { return 0; }
    };
}

extern fs::SPIFFSFS SPIFFS;
#endif
