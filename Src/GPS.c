/*
 * GPS.c
 *
 *  Created on: 3 sty 2021
 *      Author: NeghM
 */

#include <stm32l031xx.h>
#include "NEO_M6_cmd_parser.h"
#include "DMA_men.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <GPS.h>

#define GPIO_MODER_MODE2_AF (2 << GPIO_MODER_MODE2_Pos)
#define GPIO_MODER_MODE3_AF (2 << GPIO_MODER_MODE3_Pos)

#define BUFFER_SIZE 79+1 // NMEA max length
char buffer[BUFFER_SIZE];

location_t current;


//const char GPGGA_header[] = { '$', 'G', 'P', 'G', 'G', 'A' };

void GPS_init() {
	// GPIO setup
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3)) |
			(GPIO_MODER_MODE2_AF | GPIO_MODER_MODE3_AF);
	GPIOA->AFR[0] |= (6 << GPIO_AFRL_AFSEL2_Pos) | (6 << GPIO_AFRL_AFSEL3_Pos);

	// DMA setup
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	DMA1_CSELR->CSELR |= (5 << DMA_CSELR_C3S_Pos); // channel 3, lpuart_rx
	DMA1_Channel3->CPAR = (uint32_t)&LPUART1->RDR;
	DMA1_Channel3->CMAR = (uint32_t)buffer;
	DMA1_Channel3->CNDTR = BUFFER_SIZE;
	DMA1_Channel3->CCR |= DMA_CCR_MINC | DMA_CCR_EN;

	// Peripheral setup
	RCC->APB1ENR |= RCC_APB1ENR_LPUART1EN;
	LPUART1->BRR = 26667;
	LPUART1->CR3 |= USART_CR3_DMAR | USART_CR3_DMAT;
	LPUART1->CR2 |= ('\n' << USART_CR2_ADD_Pos);
	LPUART1->CR1 |= USART_CR1_CMIE | USART_CR1_RE | USART_CR1_TE | USART_CR1_UE /*| USART_CR1_TCIE*/;

	// NVIC
	NVIC_EnableIRQ(LPUART1_IRQn);
}

// (d)ddmm.mmmm, D
static float dm_dd(float loc, char dir)
{
	float result = (int)(loc / 100);

	result += (loc - (result * 100)) / 60.0;

	if (dir == 'S' || dir == 'W')
		result = -result;

	return result;
}

void processGPGGA() {
	char * c = buffer;
	float temp;

	current.isValid = 1;

	for(int i = 0; i < 2; ++i)
		while(*c++ != ','); // skip to ','

	// read latitude
	if(*c != ',') {
		temp = atof(c);
		while(*c++ != ','); // skip to ','
		current.latitude = dm_dd(temp, *c++);
		c++;
	} else {
		current.isValid = 0;
		return;
	}

	// read longitude
	if(*c != ',') {
		temp = atof(c);
		while(*c++ != ','); // skip to ','
		current.longitude = dm_dd(temp, *c++);
		//c++;
	} else {
		current.isValid = 0;
		return;
	}
}

void LPUART1_IRQHandler(void) {
	if(LPUART1->ISR & USART_ISR_CMF) {
		LPUART1->ICR = USART_ICR_CMCF; // clear flag
		DMA1_Channel3->CCR &= ~DMA_CCR_EN;
		DMA1->IFCR = DMA_IFCR_CTCIF3;

		if(strncmp(buffer, GPGGA_header, 6) == 0) {
			processGPGGA();
		}

		DMA1_Channel3->CMAR = (uint32_t)buffer;
		DMA1_Channel3->CNDTR = BUFFER_SIZE;
		DMA1_Channel3->CCR |= DMA_CCR_EN;

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

location_t GPS_getCurrentLocation() {
	return current;
}

