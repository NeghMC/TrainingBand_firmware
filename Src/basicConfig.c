/*
 * basicConfig.c
 *
 *  Created on: 11 paź 2020
 *      Author: NeghM
 */
#include <FreeRTOS.h>
#include <task.h>

#define IDLE_TASK_STACK_SIZE 128
StaticTask_t idleTask;
StackType_t idleTaskStack[IDLE_TASK_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **pxIdleTaskTCBBuffer, StackType_t **pxIdleTaskStackBuffer, uint32_t * ulIdleTaskStackSize) {
	*pxIdleTaskTCBBuffer = &idleTask;
	*pxIdleTaskStackBuffer = idleTaskStack;
	*ulIdleTaskStackSize = (uint32_t){IDLE_TASK_STACK_SIZE};
}

TaskStatus_t stat[6];

void vApplicationIdleHook() {
	int ret = uxTaskGetSystemState(stat, 6, NULL);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName) {
	;
}
