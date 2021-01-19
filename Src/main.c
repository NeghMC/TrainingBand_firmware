#include <FreeRTOS.h>
#include <task.h>

#include <stm32l031xx.h>

#include <Bluetooth.h>
#include <GPS.h>
#include <FileSystem.h>
#include <power_men.h>
#include <HR_sensor.h>
#include <OLED.h>

#include <stdlib.h>

// STM32L031K6T6
// https://www.st.com/en/microcontrollers-microprocessors/stm32l031k6.html#documentation

int main(void) {
	/* Configure the system clock */
 	clock_setup();

	HR_createTask();
	OLED_createTask();

	vTaskStartScheduler();
}
