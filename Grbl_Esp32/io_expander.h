/*
 io_expander.h
  Part of Grbl_ESP32

	copyright (c) 2019 -	Bart Dring. This file was intended for use on the ESP32
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

  IO Expander definitions

  These definitions let you use an I/O expander, for example
  one connected via I2C, for extra low-speed digital outputs.
*/
#ifndef io_expander_h
#define io_expander_h

void expanderPinMode(uint8_t pin, uint8_t mode);
void expanderDigitalWrite(uint8_t pin, uint8_t value);

#endif
