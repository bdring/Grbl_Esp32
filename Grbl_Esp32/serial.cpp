/*
  serial.cpp - Header for system level commands and real-time processes
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

#include "grbl.h"

#define RX_RING_BUFFER (RX_BUFFER_SIZE+1)
#define TX_RING_BUFFER (TX_BUFFER_SIZE+1)

portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;

uint8_t serial_rx_buffer[RX_RING_BUFFER];
uint8_t serial_rx_buffer_head = 0;
volatile uint8_t serial_rx_buffer_tail = 0;


// Returns the number of bytes available in the RX serial buffer.
uint8_t serial_get_rx_buffer_available()
{
  uint8_t rtail = serial_rx_buffer_tail; // Copy to limit multiple calls to volatile
  if (serial_rx_buffer_head >= rtail) { return(RX_BUFFER_SIZE - (serial_rx_buffer_head-rtail)); }
  return((rtail-serial_rx_buffer_head-1));
}

void serial_init()
{
	Serial.begin(BAUD_RATE);
	
	// create a task to check for incoming data
	xTaskCreatePinnedToCore(	serialCheckTask,    // task
													"servoSyncTask", // name for task
													2048,   // size of task stack
													NULL,   // parameters
													1, // priority
													&serialCheckTaskHandle, 
													0 // core
													); 
}


// this task runs and checks for data on all interfaces, currently
// only hardware serial port is checked
// This was normally done in an interrupt on 8-bit Grbl
void serialCheckTask(void *pvParameters)
{
	uint8_t data;
  uint8_t next_head;
	
	while(true) // run continuously
	{
		#ifdef ENABLE_BLUETOOTH 
			while (Serial.available() || (SerialBT.hasClient() && SerialBT.available()))
		#else
			while (Serial.available())
		#endif
		{			
		  if (Serial.available())				
				data = Serial.read();
			else
			{
				#ifdef ENABLE_BLUETOOTH
					data = SerialBT.read();					
					Serial.write(data);  // echo all data to serial					
				#endif
			}	  	
			
			
			// Pick off realtime command characters directly from the serial stream. These characters are
			// not passed into the main buffer, but these set system state flag bits for realtime execution.
			switch (data) {
				case CMD_RESET:         mc_reset(); break; // Call motion control reset routine.
				case CMD_STATUS_REPORT: report_realtime_status(); break; // direct call instead of setting flag
				case CMD_CYCLE_START:   system_set_exec_state_flag(EXEC_CYCLE_START); break; // Set as true
				case CMD_FEED_HOLD:     system_set_exec_state_flag(EXEC_FEED_HOLD); break; // Set as true
				default :
					if (data > 0x7F) { // Real-time control characters are extended ACSII only.
						switch(data) {
							case CMD_SAFETY_DOOR:   system_set_exec_state_flag(EXEC_SAFETY_DOOR); break; // Set as true
							case CMD_JOG_CANCEL:   
								if (sys.state & STATE_JOG) { // Block all other states from invoking motion cancel.
									system_set_exec_state_flag(EXEC_MOTION_CANCEL); 
								}
								break; 
							#ifdef DEBUG
								case CMD_DEBUG_REPORT: {uint8_t sreg = SREG; cli(); bit_true(sys_rt_exec_debug,EXEC_DEBUG_REPORT); SREG = sreg;} break;
							#endif
							case CMD_FEED_OVR_RESET: system_set_exec_motion_override_flag(EXEC_FEED_OVR_RESET); break;
							case CMD_FEED_OVR_COARSE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_PLUS); break;
							case CMD_FEED_OVR_COARSE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_MINUS); break;
							case CMD_FEED_OVR_FINE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_PLUS); break;
							case CMD_FEED_OVR_FINE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_MINUS); break;
							case CMD_RAPID_OVR_RESET: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_RESET); break;
							case CMD_RAPID_OVR_MEDIUM: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_MEDIUM); break;
							case CMD_RAPID_OVR_LOW: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_LOW); break;
							case CMD_SPINDLE_OVR_RESET: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_RESET); break;
							case CMD_SPINDLE_OVR_COARSE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_PLUS); break;
							case CMD_SPINDLE_OVR_COARSE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_MINUS); break;
							case CMD_SPINDLE_OVR_FINE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_PLUS); break;
							case CMD_SPINDLE_OVR_FINE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_MINUS); break;
							case CMD_SPINDLE_OVR_STOP: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_STOP); break;
							case CMD_COOLANT_FLOOD_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_FLOOD_OVR_TOGGLE); break;
							#ifdef ENABLE_M7
								case CMD_COOLANT_MIST_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_MIST_OVR_TOGGLE); break;
							#endif
						}
						// Throw away any unfound extended-ASCII character by not passing it to the serial buffer.
					} else { // Write character to buffer
						vTaskEnterCritical(&myMutex);
						next_head = serial_rx_buffer_head + 1;
						if (next_head == RX_RING_BUFFER) { next_head = 0; }

						// Write data to buffer unless it is full.
						if (next_head != serial_rx_buffer_tail) {
							serial_rx_buffer[serial_rx_buffer_head] = data;
							serial_rx_buffer_head = next_head;
						}
						vTaskExitCritical(&myMutex);
					}
			}  // switch data			
			
		}  // if something available	
		vTaskDelay(1 / portTICK_RATE_MS);  // Yield to other tasks		
	}  // while(true)
}

void serial_reset_read_buffer()
{
  serial_rx_buffer_tail = serial_rx_buffer_head;
}

// Writes one byte to the TX serial buffer. Called by main program.
void serial_write(uint8_t data) {
  Serial.write((char)data);
}
// Fetches the first byte in the serial read buffer. Called by main program.
uint8_t serial_read()
{
  uint8_t tail = serial_rx_buffer_tail; // Temporary serial_rx_buffer_tail (to optimize for volatile)
  if (serial_rx_buffer_head == tail) {
    return SERIAL_NO_DATA;
  } else {
		vTaskEnterCritical(&myMutex); // make sure buffer is not modified while reading by newly read chars from the serial when we are here
    uint8_t data = serial_rx_buffer[tail];

    tail++;
    if (tail == RX_RING_BUFFER) { tail = 0; }
    serial_rx_buffer_tail = tail;
		vTaskExitCritical(&myMutex);
    return data;
  }
}

