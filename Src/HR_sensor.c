/*
 * HR_sensor.c
 *
 *  Created on: 9 sty 2021
 *      Author: NeghM
 */

#include <stm32l031xx.h>
#include <I2C.h>
#include <FreeRTOS.h>
#include <task.h>
#include <heartRate.h>
#include <power_men.h>

#define HR_DEV_ADDRESS 0x57
#define HR_FIFO_DATA_ADD 0x07
#define HR_FIFO_WR_PTR_ADD 0x04
#define HR_FIFO_RD_PTR_ADD 0x06

volatile float beatRate = 0;

static uint8_t fifo_rd_ptr, fifo_wr_ptr;

static const uint8_t configuration[] = {
		0x02 /*Mode Configuration*/,
		0x60 /*SpO2 Configuration*/,
		0x00 /*RESERVED*/,
		0x0F /*LED1 Pulse Amplitude*/
};

//#define SAMPLE_NUMBER 96
#define HR_SAMPLE_SIZE 3
#define HR_BUFFER_SIZE 32
#define FULL_BUF_SIZE (HR_BUFFER_SIZE * HR_SAMPLE_SIZE)

#define DT 4

uint8_t pulseData[FULL_BUF_SIZE];
//uint32_t redData[SAMPLE_NUMBER];
//uint32_t IRData[SAMPLE_NUMBER];

void HR_init(void) {
	// setting
	I2C_init();
	peryph_en();
	I2C_reserve();
	I2C_WriteReg(HR_DEV_ADDRESS, 0x09, configuration, sizeof(configuration));
	I2C_release();
}

void HR_task(void * p) {
	HR_init();

	TickType_t lastBeat;
	TickType_t now = xTaskGetTickCount();

	float average = 0;

	for(;;) {
		vTaskDelayUntil(&now, pdMS_TO_TICKS(500));
		now = xTaskGetTickCount();

		I2C_reserve();
		I2C_ReadReg(HR_DEV_ADDRESS, HR_FIFO_WR_PTR_ADD, &fifo_wr_ptr, 1);
		uint8_t dataToRead;
		if(fifo_wr_ptr > fifo_rd_ptr)
			dataToRead = fifo_wr_ptr - fifo_rd_ptr;
		else
			dataToRead = HR_BUFFER_SIZE - (fifo_rd_ptr - fifo_wr_ptr);
		if(dataToRead > 0) {
			dataToRead *= HR_SAMPLE_SIZE;
			I2C_ReadReg(HR_DEV_ADDRESS, HR_FIFO_DATA_ADD, pulseData, dataToRead);
			I2C_WriteReg(HR_DEV_ADDRESS, HR_FIFO_RD_PTR_ADD, &fifo_wr_ptr, 1);
			fifo_rd_ptr = fifo_wr_ptr;
		}
		I2C_release();

		for(int i = 0; i < dataToRead; ++i) {
			int index = i * 3;
			uint32_t sample = (((uint32_t)pulseData[index]) << 16) | (((uint32_t)pulseData[index+1]) << 8) | ((uint32_t)pulseData[index+2]);
			if(checkForBeat(sample)) {
				TickType_t delta = xTaskGetTickCount() - lastBeat;
				lastBeat =  xTaskGetTickCount();

				beatRate =  60 / (delta / 1000.0);
				average = (DT * average + beatRate) / (DT + 1);
			}
		}
	}


}

#define HR_STACK_SIZE 128
static StackType_t stack[HR_STACK_SIZE];
static StaticTask_t task;

void HR_createTask(void) {
	xTaskCreateStatic(HR_task, "HR", HR_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, stack, &task);
}
