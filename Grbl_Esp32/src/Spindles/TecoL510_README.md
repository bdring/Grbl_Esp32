# VFD setup

Change these values:
1) 00-02, Main Run Command Source Selection,   Default = 0  Change to 2
2) 00-05, Main Freq Command Source Selection,  Default = 0  Change to 5

Check that these are set to the default factory values:
3) 00-07, Main and Alternative Freq Cmnd Source Selection,  Default = 0  
4) 09-00, Assigned Communication Number,      Default = 1  this is defined as  VFD_RS485_ADDR
5) 09-01, RTU/ASCII Code Selection,           Default = 0  Should be 0 (We are using hex "RTU" commands, not ASCII commands) 
6) 09-02, Baud Rate Setting,                  Default = 1  Should be 1 (9600  baud) but can be changed in the TecoL510.cpp
7) 09-03, Stop Bit Selection,                 Default = 0  Should be 0 (1 stop bit) 
8) 09-04, Parity Selection,                   Default = 0  Should be 0 (Without parity) 
9) 09-05, Data Format Selection,              Default = 0  Should be 0 (8 data bits) 

# Implementation details

this was tested with pin 16 RX and 26 TX mapped in the machine config file.  Pin 4 was used for RTS but it is not used on the rs485 module I tested with.

```
#define VFD_RS485_TXD_PIN        GPIO_NUM_26
#define VFD_RS485_RTS_PIN        GPIO_NUM_4
#define VFD_RS485_RXD_PIN        GPIO_NUM_16
// Not sure why this isn't a setting

#define DEFAULT_SPINDLE_RPM_MAX 24000.0 // rpm
#define DEFAULT_SPINDLE_RPM_MIN 6000.0 // rpm
```

not sure how to detect and parse error messages
max frequency is currenly hardcode
