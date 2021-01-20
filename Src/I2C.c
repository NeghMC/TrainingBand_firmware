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

static uint8_t inited = 0;

static volatile uint16_t toRead, toWrite;


void I2C_init(void) {

	if(inited)
		return;
	else
		inited = 1;

	// gpio
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD9_0 | GPIO_PUPDR_PUPD10_0;
	GPIOA->AFR[1] |= (1<<GPIO_AFRH_AFSEL10_Pos) | (1<<GPIO_AFRH_AFSEL9_Pos);
	GPIOA->MODER &= ~(GPIO_MODER_MODE9_0 | GPIO_MODER_MODE10_0);

	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	// Timing register value is computed with the AN4235 xls file, @100kHz with I2CCLK = 1MHz, rise time = 100ns, fall time = 10ns
	I2C1->TIMINGR = (uint32_t)0x00000103;
	I2C1->CR1 = I2C_CR1_PE | I2C_CR1_TXDMAEN | I2C_CR1_RXDMAEN | I2C_CR1_TCIE;
	NVIC_EnableIRQ(I2C1_IRQn);
	//NVIC_SetPriority(I2C1_IRQn, 3);

	semaphoreHandle = xSemaphoreCreateBinaryStatic(&semaphoreBuffer);
	xSemaphoreGive(semaphoreHandle);
}

void I2C1_IRQHandler(void) {
	BaseType_t pxHigherPriorityTaskWoken = pdFALSE;

	if(I2C1->ISR & I2C_ISR_TC) {
		if(toRead) {
			I2C1->CR2 = (I2C1->CR2 & ~(0xff << I2C_CR2_NBYTES_Pos)) | I2C_CR2_START |  (toRead << I2C_CR2_NBYTES_Pos) | I2C_CR2_RD_WRN ;
			toRead = 0;
		} else {
			// We should switch context so the ISR returns to a different task.
			// NOTE:  How this is done depends on the port you are using.  Check
			// the documentation and examples for your port.

			vTaskNotifyGiveFromISR(taskToResume, &pxHigherPriorityTaskWoken);

			I2C1->CR2 |= I2C_CR2_STOP;
		}

	} else if(I2C1->ISR & I2C_ISR_TCR) {
		if(toWrite) {
			if(toWrite > 255) {
				I2C1->CR2 |= (255 << I2C_CR2_NBYTES_Pos) | I2C_CR2_RELOAD;
				toWrite -= 255;
			} else {
				I2C1->CR2 = (I2C1->CR2 & ~((0xff << I2C_CR2_NBYTES_Pos) | I2C_CR2_RELOAD)) | (toWrite << I2C_CR2_NBYTES_Pos);
				toWrite = 0;
			}
		}
	}

	portEND_SWITCHING_ISR(pxHigherPriorityTaskWoken);
}

void I2C_reserve() {
	xSemaphoreTake(semaphoreHandle, portMAX_DELAY);
}

void I2C_release() {
	xSemaphoreGive(semaphoreHandle);
}

void I2C_ReadReg(uint8_t devAddress, uint8_t regAddress, uint8_t * data, uint8_t length) {
	taskToResume = xTaskGetCurrentTaskHandle();
	DMA_reserve(DMA_7);
	I2C1->TXDR = regAddress;
	DMA_transfer(DMA_7, 6, (void*)&I2C1->RXDR, data, length, 0); // ! there was problem
	toRead = length;

	I2C1->CR2 = I2C_CR2_START | (devAddress << I2C_CR2_SADD7) | (1 << I2C_CR2_NBYTES_Pos);
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	DMA_release(DMA_7);
}

void I2C_WriteReg(uint8_t devAddress, uint8_t regAddress, const uint8_t * data, uint16_t length) {
	taskToResume = xTaskGetCurrentTaskHandle();
	I2C1->TXDR = regAddress;
	DMA_reserve(DMA_2);
	DMA_transfer(DMA_2, 0b110, (void*)&I2C1->TXDR, data, length, 1);
	length += 1;
	if(length > 255) {
		toWrite = length - 255;
		I2C1->CR2 = I2C_CR2_START | (devAddress << I2C_CR2_SADD7) | (255 << I2C_CR2_NBYTES_Pos) | I2C_CR2_RELOAD;
	} else {
		I2C1->CR2 = I2C_CR2_START | (devAddress << I2C_CR2_SADD7) | (length << I2C_CR2_NBYTES_Pos);
	}
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	DMA_release(DMA_2);
}

