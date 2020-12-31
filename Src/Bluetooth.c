/*
 * Bluetooth.c
 *
 *  Created on: Dec 24, 2020
 *      Author: NeghM
 */

#include <stm32l031xx.h>
#include <USART.h>

void BT_enable() {
	UART_init();
	// clock enable for peripheral
	RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
	// mode
	GPIOB->MODER &= ~(0b10 << GPIO_MODER_MODE3_Pos);
}

void BT_disable() {
	// Select Analog mode (00- default) on GPIOA pin 3
	GPIOB->MODER &= ~(GPIO_MODER_MODE3);

	/* (3) Select Alternate function mode (10) on GPIOA pin 0 */
	//GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE0)) | (GPIO_MODER_MODE0_1); /* (3) */
}

void BT_prints(char * s) {
	USART_prints(s);
}
