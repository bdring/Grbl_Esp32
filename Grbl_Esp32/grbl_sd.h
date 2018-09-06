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

#define FILE_TYPE_COUNT 3   // number of acceptable gcode file types in array

extern bool SD_file_running; // a file has started but not completed
extern bool SD_ready_next; // Grbl has processed a line and is waiting for another

bool sd_mount();
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
boolean openFile(fs::FS &fs, const char * path);
boolean closeFile();
boolean readFileLine(char *line);
void readFile(fs::FS &fs, const char * path);
float sd_report_perc_complete();

#endif