#include <FreeRTOS.h>
#include "task.h"
#include "semphr.h"

#include <stm32l031xx.h>

#include <Bluetooth.h>
#include <GPS.h>
#include <FileSystem.h>

// STM32L031K6T6
// https://www.st.com/en/microcontrollers-microprocessors/stm32l031k6.html#documentation

void clock_setup();
void Error_Handler(void);

// example task
void exampleBlink(void * p) {

	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~(GPIO_MODER_MODE15_1); // output

	for(;;) {
		vTaskDelay(500);
		GPIOA->ODR ^= GPIO_ODR_OD15;
	}
}
StackType_t exampleBlinkStack[80];
StaticTask_t exampleTask;

int main(void) {
	/* Configure the system clock */
	clock_setup();

	xTaskCreateStatic(exampleBlink,"BL", 80, NULL, tskIDLE_PRIORITY+1, exampleBlinkStack, &exampleTask);
	vTaskStartScheduler();
}

inline void clock_setup() {
	/* Because of balance between power consumption and performance
	 * I decided to setup 1MHz internal clock
	 */

	//turn on internal crystal, divide by 4
	 RCC->CR |= RCC_CR_HSION | RCC_CR_HSIDIVF | RCC_CR_HSIDIVEN;

	 //wait for HSI crystal be stable
	 while(!(RCC->CR & RCC_CR_HSIRDY))
	 ;

	 //activate prefetch buffer but it should already be on
	 FLASH->ACR |= FLASH_ACR_PRFTEN;

	 // Flash 2 wait state
	 FLASH->ACR |= (uint32_t)0x1;


	 //configure RCC and PLL settings while PLL is off
	 RCC->CFGR |= RCC_CFGR_HPRE_DIV4;  //AHB prescaler

	 //set clock source
	 RCC->CFGR |= RCC_CFGR_SW_HSI;
	 while (!(RCC->CFGR & RCC_CFGR_SWS_PLL)) ;
}

