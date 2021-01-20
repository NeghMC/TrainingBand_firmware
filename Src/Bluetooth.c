/*
 * Bluetooth.c
 *
 *  Created on: Dec 24, 2020
 *      Author: NeghM
 */

#include <stm32l031xx.h>
#include <DMA_men.h>
#include <FreeRTOS.h>
#include <semphr.h>

static SemaphoreHandle_t semaphoreHandle;
static StaticSemaphore_t semaphoreBuffer;
static TaskHandle_t taskToResume;

void USART_blokingTransmit(uint8_t data) {
	while(!(USART2->ISR & USART_ISR_TXE));
	USART2->TDR = data;
}

void USART_prints(char * data) {
	while(*data) {
		while(!(USART2->ISR & USART_ISR_TC));
		USART2->TDR = *data;
		data++;
	}
}

uint8_t USART_blokingReceive(void) {
	while(!(USART2->ISR & USART_ISR_RXNE));
	return USART2->RDR;
}

void BT_init() {
	// GPIO configuration
	RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
	GPIOB->MODER &= ~(0b01 << GPIO_MODER_MODE7_Pos | 0b01 << GPIO_MODER_MODE6_Pos);
	//GPIOB->AFR[0] |=

	// DMA 6th chanel configuration


	// periph configuration
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	const uint32_t USARTDIV = 2000000/38400; //baud 38400
	USART2->BRR = (USARTDIV & 0xfff0) | ((USARTDIV & 0xf) >> 1);
	USART2->CR3 |= USART_CR3_DMAR | USART_CR3_DMAT;
	USART2->CR1 |= USART_CR1_OVER8 | /*USART_CR1_RE |*/ USART_CR1_TE | USART_CR1_UE;

	//NVIC_EnableIRQ(USART2_IRQn);

	semaphoreHandle = xSemaphoreCreateBinaryStatic(&semaphoreBuffer);
	xSemaphoreGive(semaphoreHandle);
}

void BT_enable() {
	// clock enable for peripheral
	RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
	// mode
	GPIOB->MODER &= ~(0b10 << GPIO_MODER_MODE3_Pos);
}

void USART2_IRQHandler(void) {
	if(USART2->ISR & USART_ISR_RXNE) {

	}
	// if(...
}

void BT_disable() {
	// Select Analog mode (00- default) on GPIOA pin 3
	GPIOB->MODER &= ~(GPIO_MODER_MODE3);

	/* (3) Select Alternate function mode (10) on GPIOA pin 0 */
	//GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE0)) | (GPIO_MODER_MODE0_1); /* (3) */
}

static void transmit_callback(BaseType_t * pxHigherPriorityTaskWoken) {
	xSemaphoreGiveFromISR(semaphoreHandle, pxHigherPriorityTaskWoken);
}

void BT_Transmitt(uint8_t * buffer, uint16_t length) {
	xSemaphoreTake(semaphoreHandle, portMAX_DELAY);
	DMA_reserve(DMA_4);
	DMA_transferWithCallback(DMA_4, 4, (void*)&USART2->TDR, buffer, length, 1, transmit_callback);
}

