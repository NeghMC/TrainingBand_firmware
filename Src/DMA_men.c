/*
 * DMA_men.c
 *
 *  Created on: 10 sty 2021
 *      Author: NeghM
 */

#include <stm32l031xx.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "DMA_men.h"

#define DMA_NUMBER 4

static struct {
	uint8_t offset;
	SemaphoreHandle_t semaphoreHandle;
	StaticSemaphore_t semaphoreBuffer;
	TaskHandle_t taskToResume;
} DMAs[DMA_NUMBER] = {
	{.offset = 0},
	{.offset = 1},
	{.offset = 3},
	{.offset = 4}
};

static uint8_t DMA_inited = 0;

void DMA_init(void) {
	if(DMA_inited == 0)
		return;

	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	for(int i = 0; i < DMA_NUMBER; ++i)
		DMAs[i].semaphoreHandle = xSemaphoreCreateBinaryStatic( &DMAs[i].semaphoreBuffer );

	NVIC_EnableIRQ(DMA1_Channel1_IRQn);
	NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
	NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);

	DMA_inited = 1;
}

void DMA_transfer(enum dma_number nr, uint8_t peryph, void * peryphAddr, void * memAddr, uint16_t size, uint8_t bool_memToPeryph) {
	if(DMA_inited == 0)
			DMA_init();

	xSemaphoreTake(DMAs[nr].semaphoreHandle, portMAX_DELAY);
	DMAs[nr].taskToResume = xTaskGetCurrentTaskHandle();

	DMA1_CSELR->CSELR |= (peryph << (DMAs[nr].offset * 4));
	DMA_Channel_TypeDef * channel = (DMA_Channel_TypeDef *)DMA1_Channel1_BASE + (DMAs[nr].offset * 0x14);
	channel->CPAR = (uint32_t)peryphAddr;
	channel->CMAR = (uint32_t)memAddr;
	channel->CNDTR = size;
	channel->CCR |= DMA_CCR_MINC | DMA_CCR_TCIE | DMA_CCR_EN | (bool_memToPeryph ? DMA_CCR_DIR : 0);

	vTaskSuspend(NULL);
}

static void giveSemaphorAndResumeTask(uint8_t nr) {
	xSemaphoreGiveFromISR(DMAs[nr].semaphoreHandle, NULL);
	vTaskResume(DMAs[nr].taskToResume);
}

void DMA1_Channel1_IRQHandler(void) {
	if(DMA1->ISR & DMA_ISR_TCIF1) {
		giveSemaphorAndResumeTask(DMA_1);
	}
}

void DMA1_Channel2_3_IRQHandler(void) {
	if(DMA1->ISR & DMA_ISR_TCIF2) {
		giveSemaphorAndResumeTask(DMA_2);
	}
}

void DMA1_Channel4_7_IRQHandler(void) {
	if(DMA1->ISR & DMA_ISR_TCIF4) {
		giveSemaphorAndResumeTask(DMA_4);
	}
	else if(DMA1->ISR & DMA_ISR_TCIF5) {
		giveSemaphorAndResumeTask(DMA_5);
	}
}

