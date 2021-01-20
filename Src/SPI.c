/*
 * SPI.c
 *
 *  Created on: 11 paź 2020
 *      Author: NeghM
 */

#include <stm32l031xx.h>

void SPI_init(void) {
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	GPIOB->MODER |= (0b01 << GPIO_MODER_MODE4_Pos) // cs - gpo
					| (0b10 << GPIO_MODER_MODE5_Pos)
					| (0b10 << GPIO_MODER_MODE6_Pos)
					| (0b10 << GPIO_MODER_MODE7_Pos);

	SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM;
	//SPI1->CR2 = SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN | SPI_CR2_SSOE; // TX and RX with DMA, slave select output enabled, RXNE IT, 8-bit Rx
	SPI1->CR1 |= SPI_CR1_SPE;


	// dma channel 4 and 5
}

void SPI_blokingRead(uint8_t * buffer, uint8_t size) {
	while(size > 0) {
		size--;
		while(!(SPI1->SR & SPI_SR_RXNE));
		SPI1->DR = *buffer;
		buffer++;
	}
}

void SPI_blokingWrite(uint8_t * buffer, uint8_t size) {
	while(size > 0) {
		size--;
		while(!(SPI1->SR & SPI_SR_TXE));
		*buffer = SPI1->DR;
		buffer--;
	}
}

void SPI_csSet(void) {
	GPIOB->BSRR = GPIO_BSRR_BS_4;
}

void SPI_csClear(void) {
	GPIOB->BSRR = GPIO_BSRR_BR_4;
}
