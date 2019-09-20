/*
  kinematics_polar_coaster.h - Implements simple kinematics for Grbl_ESP32
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

#define RADIUS_AXIS 0
#define POLAR_AXIS 1

#define SEGMENT_LENGTH 0.5 // segment length in mm

#ifndef kinematics_h
  #define kinematics_h
    
	#include "grbl.h"
		
bool kinematics_homing(uint8_t cycle_mask);
void inverse_kinematics(float *target, plan_line_data_t *pl_data, float *position);
void calc_polar(float *target_xyz, float *polar, float last_angle);
void user_defined_macro(uint8_t index);

void forward_kinematics(float *position);

#endif