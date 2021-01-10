#include <FreeRTOS.h>
#include "task.h"
#include "semphr.h"

#include <stm32l031xx.h>

#include <Bluetooth.h>
#include <GPS.h>
#include <FileSystem.h>
#include <power_men.h>
#include <I2C.h>

// STM32L031K6T6
// https://www.st.com/en/microcontrollers-microprocessors/stm32l031k6.html#documentation

uint8_t buf[] = {0x02, 0xE0, 0};

void testTask(void * p) {
	I2C_init();
	peryph_en();

	I2C_Transmit(buf, 2, 0x57);
	vTaskDelay(500);
	I2C_ReadRegister(buf, 2, 0x57, 0x02);

	for(;;) {

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

