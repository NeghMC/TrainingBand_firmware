/*
 * GPS.c
 *
 *  Created on: 3 sty 2021
 *      Author: NeghM
 */

#include <stm32l031xx.h>
#include "NEO_M6_cmd_parser.h"
#include "DMA_men.h"

#define GPIO_MODER_MODE2_AF (2 << GPIO_MODER_MODE2_Pos)
#define GPIO_MODER_MODE3_AF (2 << GPIO_MODER_MODE3_Pos)

#define BUFFER_SIZE 79+1 // NMEA max length
char buffer[BUFFER_SIZE];

void GPS_init() {
	// GPIO setup
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3)) |
			(GPIO_MODER_MODE2_AF | GPIO_MODER_MODE3_AF);
	GPIOA->AFR[0] |= (6 << GPIO_AFRL_AFSEL2_Pos) | (6 << GPIO_AFRL_AFSEL3_Pos);

	// DMA setup
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	DMA1_CSELR->CSELR |= (0b0101 << DMA_CSELR_C3S_Pos); // channel 6, lpuart_rx
	DMA1_Channel3->CPAR = (uint32_t)&LPUART1->RDR;
	DMA1_Channel3->CMAR = (uint32_t)buffer;
	DMA1_Channel3->CNDTR = BUFFER_SIZE;
	DMA1_Channel3->CCR |= DMA_CCR_MINC | DMA_CCR_EN;

	// Peripheral setup
	RCC->APB1ENR |= RCC_APB1ENR_LPUART1EN;
	LPUART1->BRR = 26667;
	LPUART1->CR3 |= USART_CR3_DMAR;
	LPUART1->CR2 |= ('\n' << USART_CR2_ADD_Pos);
	LPUART1->CR1 |= USART_CR1_CMIE | USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;

	// NVIC
	NVIC_EnableIRQ(LPUART1_IRQn);
}

void LPUART1_IRQHandler(void) {
	if(LPUART1->ISR & USART_ISR_CMF) {
		LPUART1->ICR |= USART_ICR_CMCF; // clear flag

		// reset
		DMA1_Channel3->CCR &= ~DMA_CCR_EN;
		DMA1_Channel3->CMAR = (uint32_t)buffer;
		DMA1_Channel3->CNDTR = BUFFER_SIZE;
		DMA1_Channel3->CCR |= DMA_CCR_EN;

		(void) buffer;

	}
	else if(LPUART1->ISR & USART_ISR_RXNE) {
		LPUART1->RQR |= USART_RQR_RXFRQ;
	}
}

void LPUART_prints(char * data) {
	while(*data) {
		while(!(LPUART1->ISR & USART_ISR_TC));
		LPUART1->TDR = *data;
		data++;
	}
}


void GPS_enable() {
	// clock enable for peripheral
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	// mode
	GPIOA->MODER &= ~(0b10 << GPIO_MODER_MODE1_Pos);
}

void GPS_disable() {
	GPIOA->MODER |= (0b11 << GPIO_MODER_MODE1_Pos);
}

void GPS_onlyNMEA_GGA() {
	nmea_conf_package conf;
	NEO_M6_setSentence(&conf);
	DMA_transfer(DMA_2, 0b101, &LPUART1->TDR, &conf, sizeof(conf), 1);
}

