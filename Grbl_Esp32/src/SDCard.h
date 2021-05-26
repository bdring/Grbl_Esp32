#pragma once

/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS
 *    CMD      MOSI
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK
 *    VSS      GND
 *    D0       MISO
 *    D1       -
 */

#include "Grbl.h"
#include <FS.h>
#include <SD.h>
#include <SPI.h>

class SDCard {
public:
    enum class State : uint8_t {
        Idle          = 0,
        NotPresent    = 1,
        Busy          = 2,
        BusyPrinting  = 2,
        BusyUploading = 3,
        BusyParsing   = 4,
    };

private:
    File     _file;
    uint32_t _current_line_number;       // the most recent line number read
    char     comment[LINE_BUFFER_SIZE];  // Line to be executed. Zero-terminated.

    State _state;

public:
    bool                       _readyNext;
    uint8_t                    _client;
    WebUI::AuthenticationLevel _auth_level;

    SDCard();

    bool _ready_next;  // Grbl has processed a line and is waiting for another

    //bool mount();
    SDCard::State get_state(bool refresh);
    SDCard::State set_state(SDCard::State state);
    void          listDir(fs::FS& fs, const char* dirname, uint8_t levels, uint8_t client);
    bool          openFile(fs::FS& fs, const char* path);
    bool          closeFile();
    bool          readFileLine(char* line, int len);
    void          readFile(fs::FS& fs, const char* path);
    float         report_perc_complete();
    uint32_t      get_current_line_number();
    void          get_current_filename(char* name);
    ~SDCard() = default;
};
extern SDCard sdCard;
