#pragma once

#include "Task.h"
#include "Queue.h"
#include <mutex>

/* "mux" data structure (spinlock) */
using portMUX_TYPE = std::mutex;

#define portMAX_DELAY (TickType_t)0xffffffffUL

void vTaskExitCritical(portMUX_TYPE* mux);
void vTaskEnterCritical(portMUX_TYPE* mux);

int32_t xPortGetFreeHeapSize();
