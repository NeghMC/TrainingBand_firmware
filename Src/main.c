#include <FreeRTOS.h>
#include <task.h>

#include <stm32l031xx.h>

#include <Bluetooth.h>
#include <GPS.h>
#include <FileSystem.h>
#include <power_men.h>
#include <HR_sensor.h>
#include <OLED.h>
#include <Bluetooth.h>
#include <stdlib.h>
#include <DMA_men.h>
#include <I2C.h>

// STM32L031K6T6
// https://www.st.com/en/microcontrollers-microprocessors/stm32l031k6.html#documentation

void testTaskFun(void * p) {
	GPS_init();
	GPS_enable();

	for(;;) {

		vTaskDelay(500);
	}
}

#define TEST_STACK_SIZE 128
static StackType_t testStack[TEST_STACK_SIZE];
static StaticTask_t testTask;

int main(void) {
	/* Configure the system clock */
 	clock_setup();

 	I2C_init();
 	DMA_init();

 	//xTaskCreateStatic(testTaskFun, "TS", TEST_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, testStack, &testTask);

	//HR_createTask();
	OLED_createTask();

	vTaskStartScheduler();
}
