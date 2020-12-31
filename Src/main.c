#include <FreeRTOS.h>
#include "task.h"
#include "semphr.h"

#include <stm32l031xx.h>

#include <Bluetooth.h>
#include <FileSystem.h>

// STM32L031K6T6
// https://www.st.com/en/microcontrollers-microprocessors/stm32l031k6.html#documentation

void clock_setup();
void Error_Handler(void);
void _delay_ms(unsigned int t);

// Milliseconds elapsed from uC start
volatile unsigned int millis;

int main(void) {
	/* Configure the system clock */
	clock_setup();
	//BT_enable();

	RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
	GPIOB->MODER &= ~(0b10 << GPIO_MODER_MODE7_Pos | 0b10 << GPIO_MODER_MODE6_Pos); // gpio


	while(1) {
		_delay_ms(100);
		GPIOB->ODR |= (GPIO_ODR_OD7 | GPIO_ODR_OD6);
		_delay_ms(100);
		GPIOB->ODR &= ~(GPIO_ODR_OD7 | GPIO_ODR_OD6);
		//BT_prints("Hello there!");
	}

	//spiffs * fileSystem = fs_init();
	//(void)fileSystem;


	//xTaskCreateStatic(usartFunction,"USART", USART_STACK_SIZE, NULL, tskIDLE_PRIORITY, usartStack, &usartTask);
	//xSemaphoreCreateBinaryStatic(&semaphore);
	//vTaskStartScheduler();
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
	 //RCC->CFGR |= RCC_CFGR_PLLSRC;   //pll source external
	 //RCC->CFGR |= RCC_CFGR_PLLMUL4;  //pll miultiplier
	 //RCC->CFGR |= RCC_CFGR_PLLDIV2;
	 RCC->CFGR |= RCC_CFGR_HPRE_DIV4;  //AHB prescaler
	 //RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;  //APB1 presacaler
	 //RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;  //APB2 prescaler


	 //turn on PLL
	 //RCC->CR |= RCC_CR_PLLON;
	 //while (!(RCC->CR & RCC_CR_PLLRDY)) ;

	 //set clock source
	 RCC->CFGR |= RCC_CFGR_SW_HSI;
	 while (!(RCC->CFGR & RCC_CFGR_SWS_PLL)) ;

	 //SystemCoreClockUpdate();

	 SysTick_Config(1000);
	 //NVIC_EnableIRQ(SysTick_IRQn);
}

void SysTick_Handler(void) {
	millis++;
}

void _delay_ms(unsigned int t) {
	unsigned int temp = millis;
	while(millis - temp <= t);
}

