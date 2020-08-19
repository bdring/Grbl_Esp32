/*
    DCServo.h
    Part of Grbl_ESP32
    
    Pin assignments for a 4 axis flat cnc
    https://easyeda.com/polaski/esp32-cnc-shield_copy
    
    Now it uses transistor/darlington array to comunicate with stepper driver. 
    
    
    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    Grbl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/

#define MACHINE_NAME "DCServo_CNC" //now based on "ESPDUINO_32"

#define X_STEP_PIN              GPIO_NUM_18
#define X_DIRECTION_PIN         GPIO_NUM_5

#define Y_STEP_PIN              GPIO_NUM_17
#define Y_DIRECTION_PIN         GPIO_NUM_16

#define Z_STEP_PIN              GPIO_NUM_4
#define Z_DIRECTION_PIN         GPIO_NUM_0

//#define A_STEP_PIN              GPIO_NUM_2
//#define A_DIRECTION_PIN         GPIO_NUM_15

#define SPINDLE_TYPE SPINDLE_TYPE_RELAY //SPINDLE_TYPE_PWM 
#define SPINDLE_OUTPUT_PIN         GPIO_NUM_26

#define X_LIMIT_PIN             GPIO_NUM_36  //I will just connect both limit switches to one input :/
//#define X_two_LIMIT_PIN         GPIO_NUM_39
#define Y_LIMIT_PIN             GPIO_NUM_34  
//#define Y_two_LIMIT_PIN         GPIO_NUM_35  
#define Z_LIMIT_PIN             GPIO_NUM_32 
//#define Z_two_LIMIT_PIN         GPIO_NUM_33 
//#define A_LIMIT_PIN             GPIO_NUM_25
//#define A_two_LIMIT_PIN         GPIO_NUM_27 

#define COOLANT_MIST_PIN        GPIO_NUM_8  // labeled Mist
#define COOLANT_FLOOD_PIN       GPIO_NUM_7  // labeled Flood
#define PROBE_PIN               GPIO_NUM_13  // labeled Probe



