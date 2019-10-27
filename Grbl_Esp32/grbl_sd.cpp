/*
  grbl_sd.cpp - Adds SD Card Features to Grbl_ESP32
  Part of Grbl_ESP32

  Copyright (c) 2018 Barton Dring Buildlog.net

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "grbl_sd.h"

// Define line flags. Includes comment type tracking and line overflow detection.
#define LINE_FLAG_OVERFLOW bit(0)
#define LINE_FLAG_COMMENT_PARENTHESES bit(1)
#define LINE_FLAG_COMMENT_SEMICOLON bit(2)

File myFile;
bool SD_ready_next = false; // Grbl has processed a line and is waiting for another
uint8_t SD_client = CLIENT_SERIAL;
uint32_t sd_current_line_number; // stores the most recent line number read from the SD
static char comment[LINE_BUFFER_SIZE]; // Line to be executed. Zero-terminated.

// attempt to mount the SD card
/*bool sd_mount()
{
  if(!SD.begin()) {
    report_status_message(STATUS_SD_FAILED_MOUNT, CLIENT_SERIAL);
    return false;
  }
  return true;
}*/

void listDir(fs::FS &fs, const char * dirname, uint8_t levels, uint8_t client)
{
  //char temp_filename[128]; // to help filter by extension	TODO: 128 needs a definition based on something

  File root = fs.open(dirname);
  if(!root) {
    report_status_message(STATUS_SD_FAILED_OPEN_DIR, client);
    return;
  }
  if(!root.isDirectory()) {
    report_status_message(STATUS_SD_DIR_NOT_FOUND, client);
    return;
  }

  File file = root.openNextFile();
  while(file) {
    if(file.isDirectory()) {
      if(levels) {
        listDir(fs, file.name(), levels -1, client);
      }
    } else {      
      grbl_sendf(CLIENT_ALL, "[FILE:%s|SIZE:%d]\r\n", file.name(), file.size());
    }
    file = root.openNextFile();
  }
}

boolean openFile(fs::FS &fs, const char * path)
{
  myFile = fs.open(path);

  if(!myFile) {
    //report_status_message(STATUS_SD_FAILED_READ, CLIENT_SERIAL);
    return false;
  }

  set_sd_state(SDCARD_BUSY_PRINTING);
  SD_ready_next = false; // this will get set to true when Grbl issues "ok" message
  sd_current_line_number = 0;
  return true;
}

boolean closeFile()
{
  if(!myFile) {
    return false;
  }

  set_sd_state(SDCARD_IDLE);
  SD_ready_next = false;
  sd_current_line_number = 0;
  myFile.close();
  return true;
}

/*
 read a line from the SD card
 strip whitespace
 strip comments per http://linuxcnc.org/docs/ja/html/gcode/overview.html#gcode:comments
 make uppercase
 return true if a line is
*/
boolean readFileLine(char *line)
{
  char c;
  uint8_t index = 0;
  uint8_t line_flags = 0;
  uint8_t comment_char_counter = 0;

  if (!myFile) {
    report_status_message(STATUS_SD_FAILED_READ, SD_client);
    return false;
  }

  sd_current_line_number += 1;

  while(myFile.available()) {
    c = myFile.read();
		
	if (line_flags & LINE_FLAG_COMMENT_PARENTHESES) {  // capture all characters into a comment buffer
		comment[comment_char_counter++] = c;
	}
	
    if (c == '\r' || c == ' ' ) {
      // ignore these whitespace items
    } else if (c == '(') {
      line_flags |= LINE_FLAG_COMMENT_PARENTHESES;
    } else if (c == ')') {
      // End of '()' comment. Resume line allowed.
	  if (line_flags & LINE_FLAG_COMMENT_PARENTHESES) { 
			line_flags &= ~(LINE_FLAG_COMMENT_PARENTHESES); 
			comment[comment_char_counter] = 0; // null terminate								
			report_gcode_comment(comment);								
		}
    } else if (c == ';') {
      // NOTE: ';' comment to EOL is a LinuxCNC definition. Not NIST.
      if (!(line_flags & LINE_FLAG_COMMENT_PARENTHESES)) { // semi colon inside parentheses do not mean anything
        line_flags |= LINE_FLAG_COMMENT_SEMICOLON;
      }
    } else if (c == '%') {
		// discard this character
	} else if (c == '\n') { // found the newline, so mark the end and return true
      line[index] = '\0';
      return true;
    } else { // add characters to the line
      if (!line_flags) {
        c = toupper(c); // make upper case
        line[index] = c;
        index++;
      }
    }

    if (index == 255) { // name is too long so return false
      line[index] = '\0';
      report_status_message(STATUS_OVERFLOW, SD_client);
      return false;
    }
  }
  // some files end without a newline
  if (index !=0) {
    line[index] = '\0';
    return true;
  } else { // empty line after new line
    return false;
  }
}

// return a percentage complete 50.5 = 50.5%
float sd_report_perc_complete()
{
  if (!myFile) {
    return 0.0;
  }

  return  ((float)myFile.position() /  (float)myFile.size() * 100.0);
}

uint32_t sd_get_current_line_number()
{
  return sd_current_line_number;
}


uint8_t sd_state = SDCARD_IDLE;

uint8_t get_sd_state(bool refresh)
{
#if defined(SDCARD_DET_PIN) && SDCARD_SD_PIN != -1
  //no need to go further if SD detect is not correct
  if (!((digitalRead (SDCARD_DET_PIN) == SDCARD_DET_VAL) ? true : false)) {
    sd_state = SDCARD_NOT_PRESENT;
    return sd_state;
  }
#endif
  //if busy doing something return state
  if (!((sd_state == SDCARD_NOT_PRESENT) || (sd_state == SDCARD_IDLE))) {
    return sd_state;
  }
  if (!refresh) {
    return sd_state;  //to avoid refresh=true + busy to reset SD and waste time
  }
  //SD is idle or not detected, let see if still the case

    SD.end();
    sd_state = SDCARD_NOT_PRESENT;
    //using default value for speed ? should be parameter
    //refresh content if card was removed
    if (SD.begin((GRBL_SPI_SS == -1)?SS:GRBL_SPI_SS, SPI, GRBL_SPI_FREQ)) {
      if ( SD.cardSize() > 0 )sd_state = SDCARD_IDLE;
    }
  return sd_state;
}

uint8_t set_sd_state(uint8_t flag)
{
  sd_state =  flag;
  return sd_state;
}

void sd_get_current_filename(char* name)
{

  if (myFile) {
    strcpy(name, myFile.name());
  } else {
    name[0] = 0;
  }
}


