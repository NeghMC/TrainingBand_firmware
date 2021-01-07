/*
 * power_men.c
 *
 *  Created on: 7 sty 2021
 *      Author: NeghM
 */

#include <stm32l031xx.h>

void clock_setup() {
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

void peryph_en(void) {
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~(GPIO_MODER_MODE12_1);
}
