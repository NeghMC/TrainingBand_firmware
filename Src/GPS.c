/*
 * GPS.c
 *
 *  Created on: 3 sty 2021
 *      Author: NeghM
 */

#include <stm32l031xx.h>

void GPS_enable() {
	// clock enable for peripheral
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	// mode
	GPIOA->MODER &= ~(0b10 << GPIO_MODER_MODE1_Pos);
}

void GPS_disable() {
	GPIOA->MODER |= (0b11 << GPIO_MODER_MODE1_Pos);
}
