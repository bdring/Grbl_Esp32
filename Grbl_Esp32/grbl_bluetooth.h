#ifndef grbl_bluetooth_h
#define grbl_bluetooth_h

	#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
		#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
	#endif

	#include "grbl.h"
	#include "BluetoothSerial.h"
	
	extern BluetoothSerial SerialBT;	
	void bluetooth_init(char *name);	

#endif