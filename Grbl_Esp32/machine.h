// Configure for a specific machine by including one or more
// machine definition files as described below.  Machine
// definition files assign CPU pins to machine functions
// according to the wiring of interface boards.

#ifndef _machine_h
#define _machine_h

// For initial testing, start with this one which disables all I/O pins
// #include "Machines/test_drive.h"

// For actual use, select the appropriate board from Machines/,
// or create your own, for example:
// #include "Machines/3x_v4.h"
#include "Machines/spi_daisy_4x.h"

// Some configurations use two files, the first assigning pins
// and the second providing additional customization, for example:
// #include "Machines/3x_v4.h"
// #include "Machines/3x_esc_spindle.h"


/* ---------------------------------------------------------------------------------------
   OEM Single File Configuration Option

   Instructions: Comment out all #include's above and paste the machine definition and
   any default settings that you wish to override below.
*/

// Paste machine definitions here.

// Paste default settings override definitions here.

#endif _machine_h
