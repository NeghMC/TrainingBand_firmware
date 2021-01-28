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
#include <fixmath.h>

#define HR_DEV_ADDRESS 0x57
#define HR_FIFO_DATA_ADD 0x07
#define HR_FIFO_WR_PTR_ADD 0x04
#define HR_FIFO_RD_PTR_ADD 0x06

volatile fix16_t beatAvg;

// averaging
#define HR_AV_ROW 5
static fix16_t rates[HR_AV_ROW];
static uint8_t rateSpot = 0;

static uint8_t fifo_rd_ptr, fifo_wr_ptr;

static const uint8_t configuration[] = {
		0xe0, /* FIFO configuration */
		0x03, /*Mode Configuration*/ // spo
		0x59, /*SpO2 Configuration*/ //4096, 100, 15
		0x00, /*RESERVED*/
		0x00, /*LED1 Pulse Amplitude*/
		0x3F /* IR Pulsa amplitude */
};

#define HR_SAMPLE_SIZE 6
#define HR_BUFFER_SIZE 32
#define HR_FULL_BUF_SIZE (HR_BUFFER_SIZE * HR_SAMPLE_SIZE)

uint8_t pulseData[HR_FULL_BUF_SIZE];
uint16_t * sampleBuffer = (uint16_t*)pulseData;

static uint32_t lastBeatCounter;

void HR_init(void) {
	// setting
	I2C_init();
	peryph_en();
	I2C_reserve();
	I2C_WriteReg(HR_DEV_ADDRESS, 0x08, configuration, sizeof(configuration));
	I2C_release();
}

static void add_to_buffer(uint16_t sample) {
	rates[rateSpot++] = sample; //Store this reading in the array
	if(rateSpot >= HR_AV_ROW) rateSpot = 0; //Wrap variable
}

static fix16_t get_average_from_buffer() {
	//Take average of readings
	fix16_t _beatAvg = F16(0);
	for (int x = 0; x < HR_AV_ROW; x++)
		_beatAvg = fix16_add(_beatAvg, rates[x]);
	return fix16_div(_beatAvg, HR_AV_ROW);
}

void HR_task(void * p) {

	TickType_t now = xTaskGetTickCount();

	HR_init();

	for(;;) {
		vTaskDelayUntil(&now, pdMS_TO_TICKS(500));
		now = xTaskGetTickCount();

		I2C_reserve();

		// read head pointer
		I2C_ReadReg(HR_DEV_ADDRESS, HR_FIFO_WR_PTR_ADD, &fifo_wr_ptr, 1);
		uint8_t availableSamples;

		// count number of samples
		if(fifo_wr_ptr > fifo_rd_ptr) {
			availableSamples = fifo_wr_ptr - fifo_rd_ptr;
		} else {
			availableSamples = HR_BUFFER_SIZE - (fifo_rd_ptr - fifo_wr_ptr);
		}

		if(availableSamples == 0)
			continue;

		// read buffer
		I2C_ReadReg(HR_DEV_ADDRESS, HR_FIFO_DATA_ADD, pulseData, availableSamples * HR_SAMPLE_SIZE);

		// update tail pointer
		fifo_rd_ptr = fifo_wr_ptr;

		I2C_release();

		// Assemble samples
		for(int i = 0, j = 0; i < availableSamples; ++i, j += HR_SAMPLE_SIZE) {
			sampleBuffer[i] = (((uint16_t)pulseData[j+4]) << 8) | ((uint16_t)pulseData[j+5]); // forth and fifth byte contains IR sample
		}

		// 0.5 <- part of second we waited divided by available data equals time between samples
		fix16_t delta = fix16_div(F16(0.5), fix16_from_int(availableSamples));

		// samples analytics
		for(int i = 0; i < availableSamples; ++i) {
			lastBeatCounter++;

			if(checkForBeat(sampleBuffer[i])) {

				fix16_t beatsPerMinute =  fix16_div(fix16_div(F16(60), delta), fix16_from_int(lastBeatCounter)); //   == ((60 / delta) / lastBeatCounter)

				if (beatsPerMinute < F16(150) && beatsPerMinute > F16(30)) {
					add_to_buffer(beatsPerMinute);
					beatAvg = get_average_from_buffer();
				}

				lastBeatCounter = 0;
			}
		}
	}
}

#define HR_STACK_SIZE 200
static StackType_t stack[HR_STACK_SIZE];
static StaticTask_t task;

void HR_createTask(void) {
	xTaskCreateStatic(HR_task, "HR", HR_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, stack, &task);
}
