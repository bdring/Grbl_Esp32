#include "grbl.h"

BluetoothSerial SerialBT;

void bluetooth_init(char *name)
{	
	if (!SerialBT.begin(name))
	{		
		report_status_message(STATUS_BT_FAIL_BEGIN);		
	}	
}