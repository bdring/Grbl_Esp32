#include "grbl.h"

#ifdef ENABLE_BLUETOOTH

BluetoothSerial SerialBT;

void bluetooth_init(char *name)
{	
	if (!SerialBT.begin(name))
	{		
		report_status_message(STATUS_BT_FAIL_BEGIN, CLIENT_SERIAL);		
	}	
}

#endif
