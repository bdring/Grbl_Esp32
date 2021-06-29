#pragma once

#include "Task.h"
#include "Queue.h"

/* "mux" data structure (spinlock) */
typedef struct {
    /* owner field values:
	 * 0                - Uninitialized (invalid)
	 * portMUX_FREE_VAL - Mux is free, can be locked by either CPU
	 * CORE_ID_PRO / CORE_ID_APP - Mux is locked to the particular core
	 *
	 * Any value other than portMUX_FREE_VAL, CORE_ID_PRO, CORE_ID_APP indicates corruption
	 */
    uint32_t owner;
    /* count field:
	 * If mux is unlocked, count should be zero.
	 * If mux is locked, count is non-zero & represents the number of recursive locks on the mux.
	 */
    uint32_t count;
#ifdef CONFIG_FREERTOS_PORTMUX_DEBUG
    const char* lastLockedFn;
    int         lastLockedLine;
#endif
} portMUX_TYPE;

#define portMAX_DELAY (TickType_t)0xffffffffUL

void vTaskExitCritical(portMUX_TYPE* mux);
void vTaskEnterCritical(portMUX_TYPE* mux);

int32_t xPortGetFreeHeapSize();
