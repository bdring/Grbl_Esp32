#pragma once

// Stub versions of FreeRTOS functions
typedef int TickType_t;

const int            portMUX_INITIALIZER_UNLOCKED = 0;
const int            portMAX_DELAY                = 0;
const int            portTICK_PERIOD_MS           = 1;
const int            portTICK_RATE_MS             = 1;
typedef int          portMUX_TYPE;
typedef int          BaseType_t;
typedef unsigned int UBaseType_t;
typedef void*        xQueueHandle;
typedef void*        QueueHandle_t;
typedef void*        TaskHandle_t;
typedef int          TickType_t;
const BaseType_t     pdTRUE  = 1;
const BaseType_t     pdFALSE = 0;
const BaseType_t     pdPASS  = 0;
inline void          vTaskDelay(TickType_t ticks) {}
inline BaseType_t    xQueueReceive(QueueHandle_t xQueue, void* pvBuffer, TickType_t xTicksToWait) {}
inline xQueueHandle  xQueueCreate(int n, int len) {}
inline BaseType_t    xQueueReset(QueueHandle_t queue) {
    return pdPASS;
}
inline BaseType_t xQueueSend(QueueHandle_t queue, void* item, TickType_t ticks) {
    return pdTRUE;
}
inline BaseType_t xQueueSendFromISR(QueueHandle_t queue, void* item, void* p) {
    return pdTRUE;
}
inline TaskHandle_t xTaskCreate(void (*task)(void*), const char* name, int stacksize, void* arg0, int pri, TaskHandle_t* th) {}
inline TaskHandle_t xTaskCreatePinnedToCore(
    void (*task)(void*), const char* name, int stacksize, void* arg0, int pri, TaskHandle_t* th, int core) {}
inline int xTaskGetTickCount() {
    return 0;
}
inline void vTaskDelayUntil(TickType_t* pxPreviousWakeTime, const TickType_t xTimeIncrement) {}
inline int  xPortGetFreeHeapSize() {
    return 0;
}
inline void vTaskEnterCritical(int* mutex) {}
inline void vTaskExitCritical(int* mutex) {}
