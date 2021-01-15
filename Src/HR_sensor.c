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

#define HR_DEV_ADDRESS 0x57
#define HR_FIFO_DATA_ADDRESS 0x07

float beatRate = 0;

static const uint8_t configuration[] = {
		0x09 /*configuration address*/,
		0x03 /*Mode Configuration*/,
		0x27 /*SpO2 Configuration*/,
		0x00 /*RESERVED*/,
		0x0F /*LED1 Pulse Amplitude*/
};

//#define SAMPLE_NUMBER 96
#define SAMPLE_SIZE 3
#define HR_BUFFER_SIZE 32
#define FULL_BUF_SIZE (HR_BUFFER_SIZE * SAMPLE_SIZE)

#define DT 4

uint8_t pulseData[FULL_BUF_SIZE];
//uint32_t redData[SAMPLE_NUMBER];
//uint32_t IRData[SAMPLE_NUMBER];

void HR_init(void) {
	// setting

	I2C_Transmit(configuration, sizeof(configuration), HR_DEV_ADDRESS);
}

void HR_task(void * p) {
	HR_init();

	TickType_t lastBeat;
	TickType_t now = xTaskGetTickCount();

	for(;;) {
		vTaskDelayUntil(&now, pdMS_TO_TICKS(320));
		I2C_ReadRegister(pulseData, FULL_BUF_SIZE, HR_DEV_ADDRESS, HR_FIFO_DATA_ADDRESS);
		for(int i = 0; i < HR_BUFFER_SIZE; ++i) {
			int index = i * 3;
			uint32_t sample = (((uint32_t)pulseData[index]) << 16) | (((uint32_t)pulseData[index+1]) << 8) | ((uint32_t)pulseData[index+2]);
			if(checkForBeat(sample)) {
				TickType_t delta = xTaskGetTickCount() - lastBeat;
				lastBeat =  xTaskGetTickCount();

				beatRate = (beatRate * DT) + ( 60 / (delta / 1000.0) );
				beatRate /= (DT + 1);
			}
		}
	}
}

#define HR_STACK_SIZE 200
static StackType_t stack[HR_STACK_SIZE];
static StaticTask_t task;

void HR_createTask(void) {
	xTaskCreateStatic(HR_task, "HR", HR_STACK_SIZE, NULL, 1, stack, &task);
}
