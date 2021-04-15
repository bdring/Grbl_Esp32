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

//#define SDCARD_DET_PIN -1
const int SDCARD_DET_VAL = 0;  // for now, CD is close to ground

enum class SDState : uint8_t {
    Idle          = 0,
    NotPresent    = 1,
    Busy          = 2,
    BusyPrinting  = 2,
    BusyUploading = 3,
    BusyParsing   = 4,
};

extern bool                       SD_ready_next;  // Grbl has processed a line and is waiting for another
extern uint8_t                    SD_client;
extern WebUI::AuthenticationLevel SD_auth_level;

//bool sd_mount();
SDState  get_sd_state(bool refresh);
SDState  set_sd_state(SDState state);
void     listDir(fs::FS& fs, const char* dirname, uint8_t levels, uint8_t client);
boolean  openFile(fs::FS& fs, const char* path);
boolean  closeFile();
boolean  readFileLine(char* line, int len);
void     readFile(fs::FS& fs, const char* path);
float    sd_report_perc_complete();
uint32_t sd_get_current_line_number();
void     sd_get_current_filename(char* name);
