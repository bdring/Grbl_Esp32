#pragma once

#include "Task.h"
#include "Queue.h"
#include "FreeRTOSTypes.h"
#include <mutex>

/* "mux" data structure (spinlock) */

void vTaskExitCritical(portMUX_TYPE* mux);
void vTaskEnterCritical(portMUX_TYPE* mux);

int32_t xPortGetFreeHeapSize();
