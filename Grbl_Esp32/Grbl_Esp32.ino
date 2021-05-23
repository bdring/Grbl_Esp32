/*
  Grbl_ESP32.ino - Header for system level commands and real-time processes
  Part of Grbl
  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC

	2018 -	Bart Dring This file was modified for use on the ESP32
					CPU. Do not use this with Grbl for atMega328P

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

#include "src/Grbl.h"

class CustomGCmdM102:public CustomGCmd
{
  public:
  CustomGCmdM102():CustomGCmd('M',102)
  {
  }
  int parse(char* line, uint8_t *char_offset)
  {
    return 0;
  }

  int act()
  {
    Serial.printf(">>CustomGCmdM102 run>>\n");
    return 0;
  }
};
CustomGCmdM102 cmd1;
CustomGCmdSets_Static <5>cgc_set;
void setup() {
  grbl_init();
  cgc_set.add(&cmd1);
  gc_set_custom_gcode_sets(&cgc_set);
}

void loop() {
    run_once();
}
