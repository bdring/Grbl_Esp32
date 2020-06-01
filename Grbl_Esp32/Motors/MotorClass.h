

#ifndef MOTORCLASS_H
#define MOTORCLASS_H

#include "../grbl.h"

void init_motors();
void motors_set_disable(bool disable);
void motors_set_direction_pins(uint8_t onMask);


#endif


