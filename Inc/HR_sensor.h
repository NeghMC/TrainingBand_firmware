/*
 * HR_sensor.h
 *
 *  Created on: 14 sty 2021
 *      Author: NeghM
 */

#ifndef HR_SENSOR_H_
#define HR_SENSOR_H_

#include <fixmath.h>

void HR_createTask(void);

typedef struct {
	fix16_t average;
	uint8_t id;
} pulseData_t;

pulseData_t HR_getCurrentPuls();

#endif /* HR_SENSOR_H_ */
