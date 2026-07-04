/*
 * FreeRTOSConfig.h - FreeRTOS 内核配置
 * 目标芯片: STM32F103C8T6 (72MHz)
 * 适配 CMSIS_V2 接口
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f1xx_hal.h"

/*-----------------------------------------------------------
 * CMSIS 设备头文件 (CMSIS_V2 必需)
 *-----------------------------------------------------------*/
#define CMSIS_device_header                "stm32f1xx.h"

/*-----------------------------------------------------------
 * 应用程序相关定义
 *-----------------------------------------------------------*/
#define configUSE_PREEMPTION               1
#define configUSE_IDLE_HOOK                0
#define configUSE_TICK_HOOK                0
#define configCPU_CLOCK_HZ                 ((uint32_t)72000000)
#define configTICK_RATE_HZ                 ((TickType_t)1000)  // 1ms 时钟节拍
#define configMAX_PRIORITIES               (56)                // CMSIS_V2 要求56级
#define configMINIMAL_STACK_SIZE           ((uint16_t)128)
#define configTOTAL_HEAP_SIZE              ((size_t)(14 * 1024))  // 14KB 堆
#define configMAX_TASK_NAME_LEN            (16)
#define configUSE_TRACE_FACILITY           1
#define configUSE_16_BIT_TICKS             0
#define configIDLE_SHOULD_YIELD            1
#define configUSE_MUTEXES                  1
#define configUSE_COUNTING_SEMAPHORES      1
#define configUSE_QUEUE_SETS               0
#define configQUEUE_REGISTRY_SIZE          8
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0  // CMSIS_V2 要求为0
#define configUSE_TIMERS                   1
#define configTIMER_TASK_PRIORITY          (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH           10
#define configTIMER_TASK_STACK_DEPTH       configMINIMAL_STACK_SIZE

/*-----------------------------------------------------------
 * CMSIS_V2 要求的 INCLUDE_* 定义
 *-----------------------------------------------------------*/
#define INCLUDE_xSemaphoreGetMutexHolder    1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_xTaskGetCurrentTaskHandle   1
#define INCLUDE_xTaskGetSchedulerState      1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_eTaskGetState               1
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_xTimerPendFunctionCall      1
#define INCLUDE_xTaskGetIdleTaskHandle      1
#define INCLUDE_xTaskGetHandle              1
#define INCLUDE_xQueueGetMutexHolder        1
#define INCLUDE_xSemaphoreGetCount          1
#define INCLUDE_pcTaskGetTaskName           1
#define INCLUDE_uxTaskGetNumberOfTasks      1

/*-----------------------------------------------------------
 * 可选的函数钩子
 *-----------------------------------------------------------*/
#define configCHECK_FOR_STACK_OVERFLOW      0
#define configUSE_RECURSIVE_MUTEXES         1
#define configUSE_MALLOC_FAILED_HOOK        0
#define configUSE_APPLICATION_TASK_TAG      0

/* 任务的栈深度 (单位: Word = 4字节) */
#define configTASK_STACK_DEPTH_OLED         256
#define configTASK_STACK_DEPTH_SENSOR       256
#define configTASK_STACK_DEPTH_BUTTON       256

/* 优先级定义 */
#define OLED_TASK_PRIORITY                  2
#define SENSOR_TASK_PRIORITY                3
#define BUTTON_TASK_PRIORITY                4

/* 协程相关 */
#define configUSE_CO_ROUTINES               0
#define configMAX_CO_ROUTINE_PRIORITIES     1

/* 中断优先级 */
#define configKERNEL_INTERRUPT_PRIORITY      255
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 191
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/*-----------------------------------------------------------
 * 中断服务函数映射
 * note: 使用 CMSIS_V2 时，SysTick_Handler 由 cmsis_os2.c 管理
 *-----------------------------------------------------------*/
#define vPortSVCHandler                     SVC_Handler
#define xPortPendSVHandler                  PendSV_Handler

/* 断言 */
#define configASSERT( x )                   if ((x) == 0) { taskDISABLE_INTERRUPTS(); for(;;); }

#endif /* FREERTOS_CONFIG_H */
