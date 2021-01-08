/*
 * I2C.c
 *
 *  Created on: 1 sty 2021
 *      Author: NeghM
 */

#include <stm32l031xx.h>

#define I2C_CR2_SADD7 (I2C_CR2_SADD_Pos+1)

static volatile struct {
	uint8_t address;
	uint8_t * buffer;
	uint16_t length;
} receive;

static volatile uint16_t leftToTransmit;
static void (*completeCallback)(void);

static volatile enum {
	IDLE,
	REG_ADD_SENT
} state;

void I2C_init(void) {
	// gpio
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD9_0 | GPIO_PUPDR_PUPD10_0;
	GPIOA->AFR[1] |= (1<<GPIO_AFRH_AFSEL10_Pos) | (1<<GPIO_AFRH_AFSEL9_Pos);
	GPIOA->MODER &= ~(GPIO_MODER_MODE9_0 | GPIO_MODER_MODE10_0);

	// dma
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	DMA1_CSELR->CSELR |= (0b0110 << DMA_CSELR_C2S_Pos); // channel 2, i2c_tx
	DMA1_Channel3->CPAR = (uint32_t)&I2C1->TXDR;
	//DMA1_Channel3->CMAR = (uint32_t)buffer;
	//DMA1_Channel3->CNDTR = BUFFER_SIZE;
	DMA1_Channel3->CCR |= DMA_CCR_MINC;


	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	// Timing register value is computed with the AN4235 xls file, @100kHz with I2CCLK = 1MHz, rise time = 100ns, fall time = 10ns
	I2C1->TIMINGR = (uint32_t)0x00000103;
	I2C1->CR1 = I2C_CR1_PE | I2C_CR1_TXDMAEN | I2C_CR1_RXIE | I2C_CR1_TCIE;
	NVIC_EnableIRQ(I2C1_IRQn);

	state = IDLE;

}

void I2C1_IRQHandler(void) {
	// byte received
	if(I2C1->ISR & I2C_ISR_RXNE) {
		*(receive.buffer) = I2C1->RXDR;
		receive.buffer++;
		receive.length--;
	}
	// need to reload
	else if(I2C1->ISR & I2C_ISR_TCR) {
		if(leftToTransmit > 255) {
			leftToTransmit -= 255;
			I2C1->CR2 |= (255 << I2C_CR2_NBYTES_Pos);
		} else {
			I2C1->CR2 |= (leftToTransmit << I2C_CR2_NBYTES_Pos);
		}
	}
	// transfer completed
	else if(I2C1->ISR & I2C_ISR_TC) {
		// repeated start after asking for register
		if(state == REG_ADD_SENT) {
			I2C1->CR2 = I2C_CR2_START | (receive.address << I2C_CR2_SADD7) | (receive.length << I2C_CR2_NBYTES_Pos) | I2C_CR2_RD_WRN;
			state = IDLE;
		} else {
			I2C1->CR2 |= I2C_CR2_STOP;
			if(completeCallback != 0)
				completeCallback();
		}
	}
}

void I2C_Transmitt(uint8_t * buffer, uint16_t length, uint8_t address, void (*callback)(void)) {
	DMA1_Channel3->CCR &= ~DMA_CCR_EN;
	DMA1_Channel3->CMAR = (uint32_t)buffer;
	DMA1_Channel3->CNDTR = length;
	DMA1_Channel3->CCR |= DMA_CCR_EN;

	completeCallback = callback;

	if(length > 255) {
		leftToTransmit = length - 255;
		I2C1->CR2 = I2C_CR2_START | (address << I2C_CR2_SADD7) | (255 << I2C_CR2_NBYTES_Pos) | I2C_CR2_RELOAD;
	} else {
		I2C1->CR2 = I2C_CR2_START | (address << I2C_CR2_SADD7) | (length << I2C_CR2_NBYTES_Pos);
	}
}
/*
void I2C_Receive(uint8_t * buffer, uint8_t length, uint8_t address, void (*callback)(void)) {
	receive.buffer = buffer;
	receive.length = length;
	completeCallback = callback;
	I2C1->CR2 = I2C_CR2_START | (address << I2C_CR2_SADD7) | (length << I2C_CR2_NBYTES_Pos) | I2C_CR2_RD_WRN;
}*/

int I2C_ReceiveRegister(volatile uint8_t * buffer, uint8_t length, uint8_t devAddress, uint8_t regAddress, void (*callback)(void)) {
	if(I2C1->ISR & I2C_ISR_BUSY)
		return -1;

	receive.buffer = buffer;
	receive.length = length;
	receive.address = devAddress;
	completeCallback = callback;

	I2C1->TXDR = regAddress;
	I2C1->CR2 = I2C_CR2_START | (devAddress << I2C_CR2_SADD7) | (1 << I2C_CR2_NBYTES_Pos);
	state = REG_ADD_SENT;

	return 0;
}

