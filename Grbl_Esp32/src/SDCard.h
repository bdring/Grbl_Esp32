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

const int SDCARD_IDLE           = 0;
const int SDCARD_NOT_PRESENT    = 1;
const int SDCARD_BUSY_PRINTING  = 2;
const int SDCARD_BUSY_UPLOADING = 4;
const int SDCARD_BUSY_PARSING   = 8;

extern bool                       SD_ready_next;  // Grbl has processed a line and is waiting for another
extern uint8_t                    SD_client;
extern WebUI::AuthenticationLevel SD_auth_level;

//bool sd_mount();
uint8_t  get_sd_state(bool refresh);
uint8_t  set_sd_state(uint8_t flag);
void     listDir(fs::FS& fs, const char* dirname, uint8_t levels, uint8_t client);
boolean  openFile(fs::FS& fs, const char* path);
boolean  closeFile();
boolean  readFileLine(char* line, int len);
void     readFile(fs::FS& fs, const char* path);
float    sd_report_perc_complete();
uint32_t sd_get_current_line_number();
void     sd_get_current_filename(char* name);
