/*
 * I2C.c
 *
 *  Created on: 1 sty 2021
 *      Author: NeghM
 */

#include <stm32l031xx.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <DMA_men.h>

#define I2C_CR2_SADD7 (I2C_CR2_SADD_Pos+1)

static SemaphoreHandle_t semaphoreHandle;
static StaticSemaphore_t semaphoreBuffer;
static TaskHandle_t taskToResume;

void I2C_init(void) {
	// gpio
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD9_0 | GPIO_PUPDR_PUPD10_0;
	GPIOA->AFR[1] |= (1<<GPIO_AFRH_AFSEL10_Pos) | (1<<GPIO_AFRH_AFSEL9_Pos);
	GPIOA->MODER &= ~(GPIO_MODER_MODE9_0 | GPIO_MODER_MODE10_0);

	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	// Timing register value is computed with the AN4235 xls file, @100kHz with I2CCLK = 1MHz, rise time = 100ns, fall time = 10ns
	I2C1->TIMINGR = (uint32_t)0x00000103;
	I2C1->CR1 = I2C_CR1_PE | I2C_CR1_TXDMAEN | I2C_CR1_RXIE | I2C_CR1_TCIE;
	NVIC_EnableIRQ(I2C1_IRQn);

	semaphoreHandle = xSemaphoreCreateBinaryStatic(&semaphoreBuffer);
}

void I2C1_IRQHandler(void) {
	if(I2C1->ISR & (I2C_ISR_TCR | I2C_ISR_TC | I2C_ISR_RXNE)) {
		vTaskResume(taskToResume);
	}
}

void I2C_Transmit(const uint8_t * buffer, uint16_t length, uint8_t address) {
	xSemaphoreTake(&semaphoreHandle, portMAX_DELAY);
	DMA_transfer(DMA_2, 0b110, &I2C1->TXDR, buffer, length, 1);
	taskToResume = xTaskGetCurrentTaskHandle();

	int leftToTransmit;
	while(length > 255) {
		leftToTransmit = length - 255;
		I2C1->CR2 = I2C_CR2_START | (address << I2C_CR2_SADD7) | (255 << I2C_CR2_NBYTES_Pos) | I2C_CR2_RELOAD;
		vTaskSuspend(NULL);
	}

	I2C1->CR2 = I2C_CR2_START | (address << I2C_CR2_SADD7) | (length << I2C_CR2_NBYTES_Pos);
	vTaskSuspend(NULL);

	xSemaphoreGive(&semaphoreHandle);
}
/*
void I2C_Receive(uint8_t * buffer, uint8_t length, uint8_t address, void (*callback)(void)) {
	receive.buffer = buffer;
	receive.length = length;
	completeCallback = callback;
	I2C1->CR2 = I2C_CR2_START | (address << I2C_CR2_SADD7) | (length << I2C_CR2_NBYTES_Pos) | I2C_CR2_RD_WRN;
}*/

void I2C_ReadRegister(uint8_t * buffer, uint8_t length, uint8_t devAddress, uint8_t regAddress) {
	xSemaphoreTake(&semaphoreHandle, portMAX_DELAY);

	taskToResume = xTaskGetCurrentTaskHandle();

	I2C1->TXDR = regAddress;
	I2C1->CR2 = I2C_CR2_START | (devAddress << I2C_CR2_SADD7) | (1 << I2C_CR2_NBYTES_Pos);
	vTaskSuspend(NULL);
	DMA_transfer(DMA_5, 6, I2C1->RXDR, buffer, length, 0);
	I2C1->CR2 = I2C_CR2_START | (devAddress << I2C_CR2_SADD7) | (length << I2C_CR2_NBYTES_Pos) | I2C_CR2_RD_WRN;
	vTaskSuspend(NULL);

	xSemaphoreGive(&semaphoreHandle);
}

