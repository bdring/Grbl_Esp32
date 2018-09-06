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
char fileTypes[FILE_TYPE_COUNT][8] = {".NC", ".TXT", ".GCODE"}; // filter out files not of these types (s/b UPPERCASE)
bool SD_file_running = false; // a file has started but not completed
bool SD_ready_next = false; // Grbl has processed a line and is waiting for another

// attempt to mount the SD card
bool sd_mount() {
	if(!SD.begin()){
				report_status_message(STATUS_SD_FAILED_MOUNT);
        return false;
  }	
	return true;
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    char temp_filename[128]; // to help filter by extension	TODO: 128 needs a definition based on something
		
    File root = fs.open(dirname);
    if(!root){
				report_status_message(STATUS_SD_FAILED_OPEN_DIR);
        return;
    }
    if(!root.isDirectory()){
				report_status_message(STATUS_SD_DIR_NOT_FOUND);
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){            
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
					strcpy(temp_filename, file.name()); // make a copy
					
					// convert it to uppercase so it is easy to filter
					for(int i = 0; i <= strlen(file.name()); i++){
						temp_filename[i] = toupper(temp_filename[i]);
					}
					
					// now filter for accetable file types
					for (uint8_t i=0; i < FILE_TYPE_COUNT; i++) // make sure it is a valid file type
					{
						if (strstr(temp_filename, fileTypes[i])) { 				
							grbl_sendf("[FILE:%s,SIZE:%d]\r\n", file.name(), file.size());
							break;
						}
					}            
        }
        file = root.openNextFile();
    }
}

boolean openFile(fs::FS &fs, const char * path){
  myFile = fs.open(path);
  if(!myFile){
      report_status_message(STATUS_SD_FAILED_READ);
      return false;
  }
	
  SD_file_running = true;
	SD_ready_next = false; // this will get set to true when Grbl issues "ok" message 
  return true; 
}

boolean closeFile(){
  if(!myFile){
      return false;
  }
	
	SD_file_running = false;
	SD_ready_next = false;
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
boolean readFileLine(char *line) {
  char c;
  uint8_t index = 0;
  uint8_t line_flags = 0;
	
  if (!myFile) {
		report_status_message(STATUS_SD_FAILED_READ);
    return false;
  }

    
  while(myFile.available()){
        c = myFile.read();
				
				
        if (c == '\r' || c == ' ' ) {  
         // ignore these whitespace items
        }
				else if (c == '(') {
					line_flags |= LINE_FLAG_COMMENT_PARENTHESES;
				}
				else if (c == ')') {
            // End of '()' comment. Resume line allowed.
            if (line_flags & LINE_FLAG_COMMENT_PARENTHESES) { line_flags &= ~(LINE_FLAG_COMMENT_PARENTHESES); }
        }
				else if (c == ';') {
          // NOTE: ';' comment to EOL is a LinuxCNC definition. Not NIST.
					if (!(line_flags & LINE_FLAG_COMMENT_PARENTHESES)) // semi colon inside parentheses do not mean anything						
						line_flags |= LINE_FLAG_COMMENT_SEMICOLON;
				}
        else if (c == '\n') {  // found the newline, so mark the end and return true
          line[index] = '\0';
          return true;
        }
        else { // add characters to the line
					if (!line_flags) {
						c = toupper(c); // make upper case
						line[index] = c;
						index++;
					}
        }

        if (index == 255) // name is too long so return false
        {
          line[index] = '\0';
					report_status_message(STATUS_OVERFLOW);
          return false;
        }
  }
	// some files end without a newline
  if (index !=0) {
    line[index] = '\0';
    return true;
  }
  else // empty line after new line
    return false;
}

// return a percentage complete 50.5 = 50.5%
float sd_report_perc_complete() {
	if (!myFile)
		return 0.0;
	
	return  ((float)myFile.position() /  (float)myFile.size() * 100.0);
}

/*
void readFile(fs::FS &fs, const char * path){ 

    File file = fs.open(path);
    if(!file){
				report_status_message(STATUS_SD_FAILED_READ);				
        return;
    }
    
    while(file.available()){          
        Serial.write(file.read());
    }
    file.close();
}
*/