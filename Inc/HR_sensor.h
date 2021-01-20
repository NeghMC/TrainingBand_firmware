/*
 * HR_sensor.h
 *
 *  Created on: 14 sty 2021
 *      Author: NeghM
 */

#ifndef HR_SENSOR_H_
#define HR_SENSOR_H_

extern volatile float beatRate;
volatile float average;

void HR_createTask(void);

#endif /* HR_SENSOR_H_ */
