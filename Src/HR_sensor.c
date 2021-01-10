/*
 * HR_sensor.c
 *
 *  Created on: 9 sty 2021
 *      Author: NeghM
 */

#include <stm32l031xx.h>

#define HR_ADDRESS 0x57

static const uint8_t buffer[] = {
		0x09 /*configuration address*/,
		0x02 /*Mode Configuration*/,
		0x06 /*SpO2 Configuration*/,
		0x00 /*RESERVED*/,
		0x0F /*LED Pulse Amplitude*/,
};

uint8_t data[32];

void HR_setup(void) {
	// setting
	I2C_Transmit(buffer, 5, HR_ADDRESS, NULL);
}

void HR_ReadWholeBuffer(void) {
	I2C_ReceiveRegister(data, 32, HR_ADDRESS, 0x07, NULL);
}
