## User I/O Commands

Users will always have special requirements that need I/O control. These could be for cooling fans, clamping, valves, etc. These could be digital (on/off) or analog (PWM). Many of these can be done via gcode rather than special coding.

### Types

- Digital I/O
- Analog (PWM)

Commands

#### M62 - M65 Digital Output Control 

M62 Synchronized Digital Output On

M63 Synchronized Digital Output Off

M64 Immediate Digital Output On

M65 Immediate Digital Output Off

Use P word to indicate digital output number (0-3)

#### M67 - M68 Digital Output Control 

M67 Synchronized Set Analog Value

M68 Immediate Set Analog Value

Use E word for analog output number (0-3)

Use Q word for analog value in percent duty.

Examples 

M67 E1 Q23.87 (set set output#1 to 23.87% duty)

M67 E1 Q0 (to turn off the pin. 0% duty)

### Pin Numbering

Each pin will be given a number starting at 0. The numbers for the digital and analog are independent. If you use one of each, they should both be 0.

### Synchronized

Initially all commands will be synchronized. In Grbl_ESP32 means all buffered motion must complete before the command is executed. See `protocol_buffer_synchronize()`



## Special Behaviors

- Power On - All pins will be set to the off state
- Reset - All pins will be set to the off state
- Alarm Mode - No change in pin state
- Error - No change in pins state
- End of File - TBD

### Setup

M62 - M65

```
#define USER_DIGITAL_PIN_0   GPIO_NUM_xx
#define USER_DIGITAL_PIN_1   GPIO_NUM_xx
#define USER_DIGITAL_PIN_2   GPIO_NUM_xx
#define USER_DIGITAL_PIN_3   GPIO_NUM_xx
```