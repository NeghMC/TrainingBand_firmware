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
//#include <utils.h>

#include <DMA_men.h>

#define DMA_NUMBER (sizeof(DMAs)/sizeof(DMAs[0]))

static struct {
	uint8_t offset;
	DMA_Channel_TypeDef * channel;
	SemaphoreHandle_t semaphoreHandle;
	StaticSemaphore_t semaphoreBuffer;
	TaskHandle_t taskToResume;
	void (*callback)(BaseType_t * pxHigherPriorityTaskWoken);
} DMAs[] = {
	{.offset = 0, .channel = (DMA_Channel_TypeDef *)DMA1_Channel1_BASE},
	{.offset = 1, .channel = (DMA_Channel_TypeDef *)DMA1_Channel2_BASE},
	{.offset = 3, .channel = (DMA_Channel_TypeDef *)DMA1_Channel4_BASE},
	{.offset = 4, .channel = (DMA_Channel_TypeDef *)DMA1_Channel5_BASE},
	{.offset = 5, .channel = (DMA_Channel_TypeDef *)DMA1_Channel6_BASE},
	{.offset = 6, .channel = (DMA_Channel_TypeDef *)DMA1_Channel7_BASE}
};

static uint8_t DMA_inited = 0;

void trap() {
	while(1);
}

void static interruptDetectTrap() {
	if(SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) {
		trap();
	}
}

void DMA_init(void) {
	interruptDetectTrap();

	if(DMA_inited)
		return;

	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	for(int i = 0; i < DMA_NUMBER; ++i) {
		DMAs[i].semaphoreHandle = xSemaphoreCreateBinaryStatic( &DMAs[i].semaphoreBuffer );
		xSemaphoreGive(DMAs[i].semaphoreHandle);
	}

	NVIC_EnableIRQ(DMA1_Channel1_IRQn);
	NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
	NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);

	DMA_inited = 1;
}

void DMA_reserve(enum dma_number nr) {
	interruptDetectTrap();
	if(DMA_inited == 0)
		DMA_init();
	xSemaphoreTake(DMAs[nr].semaphoreHandle, portMAX_DELAY);
}

void DMA_release(enum dma_number nr) {
	interruptDetectTrap();
	xSemaphoreGive(DMAs[nr].semaphoreHandle);
}

void DMA_transfer(enum dma_number nr, uint8_t peryph, void * peryphAddr, void * memAddr, uint16_t size, uint8_t bool_memToPeryph) {
	if(DMA_inited == 0)
			trap();

	DMA1_CSELR->CSELR |= (peryph << (DMAs[nr].offset * 4));
	DMAs[nr].channel->CPAR = (uint32_t)peryphAddr;
	DMAs[nr].channel->CMAR = (uint32_t)memAddr;
	DMAs[nr].channel->CNDTR = size;
	DMAs[nr].channel->CCR = DMA_CCR_MINC | DMA_CCR_TCIE | DMA_CCR_EN | (bool_memToPeryph ? DMA_CCR_DIR : 0);

}

// !! callback will be executed in interrupt !!
void DMA_transferWithCallback(enum dma_number nr, uint8_t peryph, void * peryphAddr, void * memAddr, uint16_t size, uint8_t bool_memToPeryph, void (*callback)(BaseType_t * pxHigherPriorityTaskWoken)) {
	if(DMA_inited == 0)
			trap();

	DMAs[nr].callback = callback;
	DMA_transfer(nr, peryph, peryphAddr, memAddr, size, bool_memToPeryph);
}


void DMA_waitForTransferEnd(enum dma_number nr) {
	interruptDetectTrap();
	DMAs[nr].taskToResume = xTaskGetCurrentTaskHandle();
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

BaseType_t pxHigherPriorityTaskWoken = pdFALSE;

static void endTransferAndResumeTask(enum dma_number nr) {
	DMAs[nr].channel->CCR &= ~DMA_CCR_EN;
	if(DMAs[nr].taskToResume != NULL) {
		vTaskNotifyGiveFromISR(DMAs[nr].taskToResume, &pxHigherPriorityTaskWoken);
		DMAs[nr].taskToResume = NULL;
	}
	if(DMAs[nr].callback != NULL) {
		DMAs[nr].callback(&pxHigherPriorityTaskWoken);
		DMAs[nr].callback = NULL;
		xSemaphoreGiveFromISR(DMAs[nr].semaphoreHandle, &pxHigherPriorityTaskWoken);
	}
}

void DMA1_Channel1_IRQHandler(void) {
	if(DMA1->ISR & DMA_ISR_TCIF1) {
		endTransferAndResumeTask(DMA_1);
		DMA1->IFCR = DMA_IFCR_CTCIF1;
	}
	portEND_SWITCHING_ISR(pxHigherPriorityTaskWoken);
}

void DMA1_Channel2_3_IRQHandler(void) {
	if(DMA1->ISR & DMA_ISR_TCIF2) {
		endTransferAndResumeTask(DMA_2);
		DMA1->IFCR = DMA_IFCR_CTCIF2;
	}
	portEND_SWITCHING_ISR(pxHigherPriorityTaskWoken);
}

void DMA1_Channel4_7_IRQHandler(void) {
	if(DMA1->ISR & DMA_ISR_TCIF4) {
		endTransferAndResumeTask(DMA_4);
		DMA1->IFCR = DMA_IFCR_CTCIF4;
	}
	else if(DMA1->ISR & DMA_ISR_TCIF5) {
		endTransferAndResumeTask(DMA_5);
		DMA1->IFCR = DMA_IFCR_CTCIF5;
	}
	else if(DMA1->ISR & DMA_ISR_TCIF6) {
		endTransferAndResumeTask(DMA_6);
		DMA1->IFCR = DMA_IFCR_CTCIF6;
	}
	else if(DMA1->ISR & DMA_ISR_TCIF7) {
		endTransferAndResumeTask(DMA_7);
		DMA1->IFCR = DMA_IFCR_CTCIF7;
	}
	portEND_SWITCHING_ISR(pxHigherPriorityTaskWoken);
}

