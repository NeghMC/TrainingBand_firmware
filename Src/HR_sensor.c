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
#include <Bluetooth.h>

#define HR_DEV_ADDRESS 0x57
#define HR_FIFO_DATA_ADD 0x07
#define HR_FIFO_WR_PTR_ADD 0x04
#define HR_FIFO_RD_PTR_ADD 0x06

volatile float beatAvg;

// averaging
#define HR_AV_ROW 5
float rates[HR_AV_ROW];
uint8_t rateSpot = 0;

static uint8_t fifo_rd_ptr, fifo_wr_ptr;

static const uint8_t configuration[] = {
		0xe0, /* FIFO configuration */
		0x03, /*Mode Configuration*/ // spo
		0x59, /*SpO2 Configuration*/ //4096, 100, 15
		0x00, /*RESERVED*/
		0x00, /*LED1 Pulse Amplitude*/
		0x3F /* IR Pulsa amplitude */
};

//#define SAMPLE_NUMBER 96
#define HR_SAMPLE_SIZE 6
#define HR_BUFFER_SIZE 32
#define FULL_BUF_SIZE (HR_BUFFER_SIZE * HR_SAMPLE_SIZE)

uint8_t pulseData[FULL_BUF_SIZE];
uint16_t * sampleBuffer = (uint16_t*)pulseData;
//uint32_t redData[SAMPLE_NUMBER];
//uint32_t IRData[SAMPLE_NUMBER];

static int lastBeatCounter;

// plotttttt
#define NUMBER_LENGTH 6
char cbuffer[NUMBER_LENGTH * HR_BUFFER_SIZE];
int cbufferp;

void HR_init(void) {
	// setting
	I2C_init();
	peryph_en();
	I2C_reserve();
	I2C_WriteReg(HR_DEV_ADDRESS, 0x08, configuration, sizeof(configuration));
	I2C_release();
}

int digit_counter(uint32_t n) {
	int count = 0;
	 while (n != 0) {
	        n /= 10;     // n = n/10
	        ++count;
	    }
	 return count;
}

// median filter

/* Function to sort an array using insertion sort*/
void insertionSort(uint16_t arr[], int n)
{
    int i, key, j;
    for (i = 1; i < n; i++)
    {
        key = arr[i];
        j = i - 1;

        /* Move elements of arr[0..i-1], that are
        greater than key, to one position ahead
        of their current position */
        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

#define MEDIAN_FILTER_ROW 6
int medianFilter(uint16_t tab[]) {
	insertionSort(tab, MEDIAN_FILTER_ROW);
	if(MEDIAN_FILTER_ROW & 0x1) {
		return tab[MEDIAN_FILTER_ROW / 2 - 1];
	} else {
		return (tab[MEDIAN_FILTER_ROW / 2] + tab[MEDIAN_FILTER_ROW / 2 - 1]) / 2;
	}
}
/// end

void HR_task(void * p) {

	TickType_t now = xTaskGetTickCount();

	HR_init();
	BT_init();

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

		// checking if the loop isnt too slow
		//if(availableSamples == 32) {
		//	__NOP();
		//}

		if(availableSamples > 0) {
			// read buffer
			I2C_ReadReg(HR_DEV_ADDRESS, HR_FIFO_DATA_ADD, pulseData, availableSamples * HR_SAMPLE_SIZE);
			// write tail pointer
			//I2C_WriteReg(HR_DEV_ADDRESS, HR_FIFO_RD_PTR_ADD, &fifo_wr_ptr, 1);
			fifo_rd_ptr = fifo_wr_ptr;
		}
		I2C_release();

		// construct samples
		for(int i = 0; i < availableSamples; ++i) {
			int byte = i * HR_SAMPLE_SIZE;
			sampleBuffer[i] = (((uint16_t)pulseData[byte+4]) << 8) | ((uint16_t)pulseData[byte+5]);
		}

		/*
		// median filtering
		for(int i = (MEDIAN_FILTER_ROW / 2); i < availableSamples - (MEDIAN_FILTER_ROW / 2); ++i) {
			uint16_t copy[MEDIAN_FILTER_ROW];
			for(int j = i; j < i + MEDIAN_FILTER_ROW; ++j)
				copy[j - i] = sampleBuffer[j];
			sampleBuffer[i] = medianFilter(copy);
		}*/

		// 0.5 <- part of second we waited, divided by available data we get time between samples
		float delta = 0.5f / availableSamples;

		// samples analytics
		for(int i = 0; i < availableSamples; ++i) {
			lastBeatCounter++;

			// transform to text for uart transmission
			//itoa(sampleBuffer[i], cbuffer + cbufferp, 10);
			//cbufferp += digit_counter(sampleBuffer[i]);
			//cbuffer[cbufferp] = '\n';
			//cbufferp++;

			if(checkForBeat(sampleBuffer[i])) {

				float beatsPerMinute =  (60 / delta) / lastBeatCounter; // value 0.27 was calculated

				if (beatsPerMinute < 150 && beatsPerMinute > 30) {
					rates[rateSpot++] = beatsPerMinute; //Store this reading in the array
					if(rateSpot >= HR_AV_ROW) rateSpot = 0; //Wrap variable

					//Take average of readings
					float _beatAvg = 0;
					for (int x = 0; x < HR_AV_ROW; x++)
						_beatAvg += rates[x];
					beatAvg = _beatAvg / HR_AV_ROW;
				}

				lastBeatCounter = 0;
			}
		}

		//BT_Transmitt((uint8_t*)cbuffer, cbufferp);
		//cbufferp = 0;
	}


}

#define HR_STACK_SIZE 200
static StackType_t stack[HR_STACK_SIZE];
static StaticTask_t task;

void HR_createTask(void) {
	xTaskCreateStatic(HR_task, "HR", HR_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, stack, &task);
}
