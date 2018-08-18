#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void bluetooth_init(char *name)
{	
	if (!SerialBT.begin(name))
	{
		Serial.printf("BlueTooth Failed: %s", name);
	}
}