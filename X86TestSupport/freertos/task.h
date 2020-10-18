#pragma once

#include <cstdint>
#include <climits>

class TaskHandle {};

typedef TaskHandle* TaskHandle_t;

typedef uint32_t TickType_t;
#define portMAX_DELAY (TickType_t)0xffffffffUL

TickType_t xTaskGetTickCount();
void       vTaskDelay(int delay);
void       vTaskDelayUntil(TickType_t* lastWake, TickType_t tickType);

#define portBASE_TYPE int
typedef portBASE_TYPE BaseType_t;
typedef unsigned portBASE_TYPE UBaseType_t;

typedef void (*TaskFunction_t)(void*);
typedef BaseType_t(*TaskHookFunction_t)(void*);

#define tskNO_AFFINITY INT_MAX

BaseType_t xTaskCreatePinnedToCore(TaskFunction_t      pvTaskCode,
                                   const char* const   pcName,
                                   const uint32_t      usStackDepth,
                                   void* const         pvParameters,
                                   UBaseType_t         uxPriority,
                                   TaskHandle_t* const pvCreatedTask,
                                   const BaseType_t    xCoreID);

static inline BaseType_t xTaskCreate(TaskFunction_t      pvTaskCode,
                                     const char* const   pcName,
                                     const uint32_t      usStackDepth,
                                     void* const         pvParameters,
                                     UBaseType_t         uxPriority,
                                     TaskHandle_t* const pvCreatedTask) {
    return xTaskCreatePinnedToCore(pvTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pvCreatedTask, tskNO_AFFINITY);
}
