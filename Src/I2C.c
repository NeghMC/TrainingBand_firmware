/*
 * I2C.c
 *
 *  Created on: 1 sty 2021
 *      Author: NeghM
 */

#include <stm32l031xx.h>

#define I2C_CR2_SADD7 (I2C_CR2_SADD_Pos+1)

void I2C_init(void) {
	// gpio
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~(GPIO_MODER_MODE9_0 | GPIO_MODER_MODE10_0);
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD9_0 | GPIO_PUPDR_PUPD10_0;
	GPIOA->AFR[1] |= (1<<GPIO_AFRH_AFSEL10_Pos) | (1<<GPIO_AFRH_AFSEL9_Pos);


	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	// Timing register value is computed with the AN4235 xls file, @100kHz with I2CCLK = 1MHz, rise time = 100ns, fall time = 10ns
	I2C1->TIMINGR = (uint32_t)0x00000103;
	I2C1->CR1 = I2C_CR1_PE;
}

void I2C_testWrite(void) {
	I2C1->CR2 = I2C_CR2_START | (0x1C << I2C_CR2_SADD7) | (1 << I2C_CR2_NBYTES_Pos) | I2C_CR2_AUTOEND;
}
