#pragma once

#include "task.h"

typedef void* QueueHandle_t;

using xQueueHandle = QueueHandle_t;

#define queueSEND_TO_BACK ((BaseType_t)0)
#define queueSEND_TO_FRONT ((BaseType_t)1)
#define queueOVERWRITE ((BaseType_t)2)

BaseType_t xQueueGenericReceive(QueueHandle_t xQueue, void* const pvBuffer, TickType_t xTicksToWait, const BaseType_t xJustPeek);

BaseType_t xQueueGenericSendFromISR(QueueHandle_t     xQueue,
                                    const void* const pvItemToQueue,
                                    BaseType_t* const pxHigherPriorityTaskWoken,
                                    const BaseType_t  xCopyPosition);

QueueHandle_t xQueueGenericCreate(const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, const uint8_t ucQueueType);

BaseType_t xQueueGenericReset(QueueHandle_t xQueue, BaseType_t xNewQueue);

BaseType_t xQueueGenericSend(QueueHandle_t xQueue, const void* const pvItemToQueue, TickType_t xTicksToWait, BaseType_t xCopyPosition);

#define xQueueSendFromISR(xQueue, pvItemToQueue, pxHigherPriorityTaskWoken)                                                                \
    xQueueGenericSendFromISR((xQueue), (pvItemToQueue), (pxHigherPriorityTaskWoken), queueSEND_TO_BACK)

#define pdFALSE ((BaseType_t)0)
#define pdTRUE ((BaseType_t)1)
#define xQueueReceive(xQueue, pvBuffer, xTicksToWait) xQueueGenericReceive((xQueue), (pvBuffer), (xTicksToWait), pdFALSE)

#define queueQUEUE_TYPE_BASE ((uint8_t)0U)

#define xQueueCreate(uxQueueLength, uxItemSize) xQueueGenericCreate((uxQueueLength), (uxItemSize), (queueQUEUE_TYPE_BASE))

#define xQueueReset(xQueue) xQueueGenericReset(xQueue, pdFALSE)

#define xQueueSend(xQueue, pvItemToQueue, xTicksToWait) xQueueGenericSend((xQueue), (pvItemToQueue), (xTicksToWait), queueSEND_TO_BACK)
