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
 
#ifndef grbl_sd_h
#define grbl_sd_h


  #include "grbl.h"  
 	#include "FS.h"
	#include "SD.h"
	#include "SPI.h"

#define FILE_TYPE_COUNT 5   // number of acceptable gcode file types in array

#define SDCARD_DET_PIN -1
#define SDCARD_DET_VAL 0

#define SDCARD_IDLE 0
#define SDCARD_NOT_PRESENT 1
#define SDCARD_BUSY_PRINTING 2
#define SDCARD_BUSY_UPLOADING 4
#define SDCARD_BUSY_PARSING 8



extern bool SD_ready_next; // Grbl has processed a line and is waiting for another
extern  uint8_t SD_client;

//bool sd_mount();
uint8_t get_sd_state(bool refresh);
uint8_t set_sd_state(uint8_t flag);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels, uint8_t client);
boolean openFile(fs::FS &fs, const char * path);
boolean closeFile();
boolean readFileLine(char *line);
void readFile(fs::FS &fs, const char * path);
float sd_report_perc_complete();
uint32_t sd_get_current_line_number();
void sd_get_current_filename(char* name);

#endif
