/*
  unipolar.cpp
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

	To take a step simply call the step(step, direction) function.

*/
#include "grbl.h"

#ifdef USE_UNIPOLAR	

	// assign the I/O pins used for each coil of the motors
	#ifdef X_UNIPOLAR
		Unipolar X_Unipolar(X_PIN_PHASE_0, X_PIN_PHASE_1, X_PIN_PHASE_2, X_PIN_PHASE_3, true);		
	#endif

	#ifdef Y_UNIPOLAR
		Unipolar Y_Unipolar(Y_PIN_PHASE_0, Y_PIN_PHASE_1, Y_PIN_PHASE_2, Y_PIN_PHASE_3, true);
	#endif

	#ifdef Z_UNIPOLAR
		Unipolar Z_Unipolar(Z_PIN_PHASE_0, Z_PIN_PHASE_1, Z_PIN_PHASE_2, Z_PIN_PHASE_3, true);
	#endif

	void unipolar_init(){
		#ifdef X_UNIPOLAR
			X_Unipolar.init();
			grbl_send(CLIENT_SERIAL, "[MSG:X Unipolar]\r\n");
		#endif
		#ifdef Y_UNIPOLAR
			Y_Unipolar.init();
			grbl_send(CLIENT_SERIAL, "[MSG:Y Unipolar]\r\n");
		#endif
		#ifdef Z_UNIPOLAR
			Z_Unipolar.init();
			grbl_send(CLIENT_SERIAL, "[MSG:Z Unipolar]\r\n");
		#endif
	}

	void unipolar_step(uint8_t step_mask, uint8_t dir_mask)
	{		
		#ifdef X_UNIPOLAR
			X_Unipolar.step(step_mask & (1<<X_AXIS), dir_mask & (1<<X_AXIS));
		#endif
		#ifdef Y_UNIPOLAR
			Y_Unipolar.step(step_mask & (1<<Y_AXIS), dir_mask & (1<<Y_AXIS));
		#endif
		#ifdef Z_UNIPOLAR
			Z_Unipolar.step(step_mask & (1<<Z_AXIS), dir_mask & (1<<ZX_AXIS));
		#endif
	}
	
	void unipolar_disable(bool disable)
	{
		#ifdef X_UNIPOLAR
			X_Unipolar.set_enabled(!disable);			
		#endif
		#ifdef Y_UNIPOLAR
			Y_Unipolar.set_enabled(!disable);
		#endif
		#ifdef Z_UNIPOLAR
			Z_Unipolar.set_enabled(!disable);
		#endif
	}
	
	
	Unipolar::Unipolar(uint8_t pin_phase0, uint8_t pin_phase1, uint8_t pin_phase2, uint8_t pin_phase3, bool half_step) // constructor
	{
		_pin_phase0 = pin_phase0;
		_pin_phase1 = pin_phase1;
		_pin_phase2 = pin_phase2;
		_pin_phase3 = pin_phase3;
		_half_step = half_step;
	}
	
	void Unipolar::init() {
		pinMode(_pin_phase0, OUTPUT);
		pinMode(_pin_phase1, OUTPUT);
		pinMode(_pin_phase2, OUTPUT);
		pinMode(_pin_phase3, OUTPUT);
		
		_current_phase = 0;
		set_enabled(false);	
	}

	void Unipolar::set_enabled(bool enabled)
	{
		if (enabled == _enabled)
			return; // no change
		
		//grbl_sendf(CLIENT_SERIAL, "[MSG:Enabled...%d]\r\n", enabled);
		
		_enabled = enabled;
		
		if (!enabled) {
			digitalWrite(_pin_phase0, 0);
			digitalWrite(_pin_phase1, 0);
			digitalWrite(_pin_phase2, 0);
			digitalWrite(_pin_phase3, 0);
		}
	}
	
	/*
	To take a step set step to true and set the driection
	
	step is included so that st.step_outbits can be used to determine if a 
	step is required on this axis		
	*/
	void Unipolar::step(bool step, bool dir_forward)
	{
		uint8_t _phase[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // temporary phase values...all start as off		
		uint8_t phase_max;
		
		if (_half_step)
			phase_max = 7;
		else
			phase_max = 3;
		
		if (!step) 
			return;	// a step is not required on this interrupt
		
		if (!_enabled)
			return;	// don't do anything, phase is not changed or lost		
		
		if (dir_forward) { // count up
			if (_current_phase == phase_max) {
				_current_phase = 0;
			}
			else {
				_current_phase++;
			}
		}
		else { // count down
			if (_current_phase == 0) {
				_current_phase = phase_max;
			}
			else {
				_current_phase--;
			}
		}
		
		/*
			8 Step : A – AB – B – BC – C – CD – D – DA
			4 Step : AB – BC – CD – DA (Usual application)
			
			Step		IN4	IN3	IN2	IN1
			A 		0 	0 	0 	1
			AB		0	0	1	1
			B		0	0	1	0
			BC		0	1	1	0
			C		0	1	0	0
			CD		1	1	0	0
			D		1	0	0	0
			DA		1	0	0	1
		*/      
		if (_half_step) {
			switch (_current_phase) {
				case 0:
					_phase[0] = 1;				
				break;				
				case 1:
					_phase[0] = 1;
					_phase[1] = 1;
				break;
				case 2:
					_phase[1] = 1;
				break;
				case 3:
					_phase[1] = 1;
					_phase[2] = 1;
				break;
				case 4:
					_phase[2] = 1;
				break;				
				case 5:
					_phase[2] = 1;
					_phase[3] = 1;
				break;
				case 6:
					_phase[3] = 1;
				break;
				case 7:
					_phase[3] = 1;
					_phase[0] = 1;
				break;			
			}
		}
		else {
			switch (_current_phase) {
				case 0:
					_phase[0] = 1;
					_phase[1] = 1;				
				break;				
				case 1:
					_phase[1] = 1;
					_phase[2] = 1;
				break;
				case 2:
					_phase[2] = 1;
					_phase[3] = 1;
				break;
				case 3:
					_phase[3] = 1;
					_phase[0] = 1;
				break;					
			}
		}
		
		digitalWrite(_pin_phase0, _phase[0]);
		digitalWrite(_pin_phase1, _phase[1]);
		digitalWrite(_pin_phase2, _phase[2]);
		digitalWrite(_pin_phase3, _phase[3]);
	}
#endif