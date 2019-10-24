/* 
 grbl_unipolar.h
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

	Unipolar Class

	This class allows you to control a unipolar motor. Unipolar motors have 5 wires. One
	is typically tied to a voltage, while the other 4 are switched to ground in a 
	sequence

	To take a step simply call the step(direction) function. It will take 

*/
#ifndef grbl_unipolar_h
  #define grbl_unipolar_h

	void unipolar_init();
	void unipolar_step(uint8_t step_mask, uint8_t dir_mask);
	void unipolar_disable(bool enable);

	class Unipolar{
		public:
			Unipolar(uint8_t pin_phase0, uint8_t pin_phase1, uint8_t pin_phase2, uint8_t pin_phase3, bool half_step); // constructor   
			void set_enabled(bool enabled);
			void step(bool step, bool dir_forward);
			void init();
		
		private:
			uint8_t _current_phase = 0;
			bool _enabled = false;
			bool _half_step = true;	// default is half step, full step
			uint8_t _pin_phase0;
			uint8_t _pin_phase1;
			uint8_t _pin_phase2;
			uint8_t _pin_phase3;
			
	};
#endif