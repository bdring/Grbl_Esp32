#include <Arduino.h>
#if EPOXY_DUINO_VERSION < 10000
#    include <SPIFFS.h>

fs::SPIFFSFS SPIFFS;
#endif
