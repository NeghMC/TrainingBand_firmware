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

extern volatile fix16_t beatAvg;

#endif /* HR_SENSOR_H_ */
