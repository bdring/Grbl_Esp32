/*
  kinematics.h - Implements simple kinematics for Grbl_ESP32
  Part of Grbl_ESP32

  Copyright (c) 2019 Barton Dring @buildlog
      
	 
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

#ifndef kinematics_h
  #define kinematics_h    
		
void inverse_kinematics(float *target, plan_line_data_t *pl_data, float *position);

#endif