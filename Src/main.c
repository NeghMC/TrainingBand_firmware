#include <FreeRTOS.h>
#include "task.h"
#include "semphr.h"

#include <stm32l031xx.h>

#include <Bluetooth.h>
#include <FileSystem.h>
#include <power_men.h>
#include <I2C.h>

// STM32L031K6T6
// https://www.st.com/en/microcontrollers-microprocessors/stm32l031k6.html#documentation

volatile uint8_t buf[256] = {0};

void testTask(void * p) {
	I2C_init();
	peryph_en();

	for(;;) {
		I2C_ReceiveRegister(buf, 255, 0x57, 0, NULL);
		vTaskDelay(500);
	}
}
StackType_t testTaskStack[120];
StaticTask_t testTaskTask;

int main(void) {
	/* Configure the system clock */
	clock_setup();

	xTaskCreateStatic(testTask,"BL", 120, NULL, tskIDLE_PRIORITY+1, testTaskStack, &testTaskTask);
	vTaskStartScheduler();
}

