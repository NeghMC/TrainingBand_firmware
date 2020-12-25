/*
 * UART.c
 *
 *  Created on: 10 paź 2020
 *      Author: NeghM
 */

#include <stm32l031xx.h>
#include <stdint.h>

/* mantis
 *
 *
 *
 */

void UART_init() {
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	// pin configuration
	GPIOB->MODER |= (0b10 << GPIO_MODER_MODE7_Pos) | (0b10 << GPIO_MODER_MODE6_Pos);

	USART2->BRR = 0x0117; //baud 115200

	//NVIC_EnableIRQ(USART2_IRQn);

	USART2->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
}

void USART2_IRQHandler(void) {
	if(USART2->ISR & USART_ISR_RXNE) {

	}
	// if(...
}

void USART_blokingTransmit(uint8_t data) {
	while(!(USART2->ISR & USART_ISR_TXE));
	USART2->TDR = data;
}

void USART_prints(char * data) {
	while(*data) {
		while(!(USART2->ISR & USART_ISR_TXE));
		USART2->TDR = *data;
		data++;
	}
}

uint8_t USART_blokingReceive(void) {
	while(!(USART2->ISR & USART_ISR_RXNE));
	return USART2->RDR;
}
