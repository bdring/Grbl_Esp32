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

#include "Configuration/Configurable.h"
#include "WebUI/Authentication.h"
#include "Pin.h"

#include <cstdint>


// Forward declaration:
namespace fs {
    class FS;
}

// XXX This should be a configuration parameter of the SPI bus
const int32_t SPIfreq = 4000000;

class SDCard : public Configuration::Configurable {
public:
    enum class State : uint8_t {
        Idle          = 0,
        NotPresent    = 1,
        Busy          = 2,
        BusyPrinting  = 2,
        BusyUploading = 3,
        BusyParsing   = 4,
    };

    enum class ReadResult_t : uint8_t {    // return value from readFileLine()
        OK                  = 0,
        EndOfFile           = 1,
        FSError             = 2,
        ImplementationError = 3,
        UserError           = 4,
    };

    class FileWrap;  // holds a single 'File'; we don't want to include <FS.h> here

private:
    static const int COMMENT_SIZE = 256;

    FileWrap* _pImpl;                 // this is actually a 'File'; we don't want to include <FS.h>
    uint32_t  _current_line_number;   // the most recent line number read
    char      comment[COMMENT_SIZE];  // Line to be executed. Zero-terminated.

    State _state;
    Pin   _cardDetect;

public:
    bool                       _readyNext = false;
    uint8_t                    _client;
    WebUI::AuthenticationLevel _auth_level;

    SDCard();
    SDCard(const SDCard&) = delete;
    SDCard& operator=(const SDCard&) = delete;

    //bool mount();
    SDCard::State get_state(bool refresh);
    SDCard::State set_state(SDCard::State state);

    void     listDir(fs::FS& fs, const char* dirname, uint8_t levels, uint8_t client);
    bool     openFile(fs::FS& fs, const char* path);
    bool     closeFile();
    ReadResult_t  readFileLine(char* line, int len);
    float    report_perc_complete();
    uint32_t get_current_line_number();
    void     get_current_filename(char* name);

    // Initializes pins.
    void init();

    // Configuration handlers.
    void group(Configuration::HandlerBase& handler) override { handler.item("card_detect", _cardDetect); }

    ~SDCard();
};
