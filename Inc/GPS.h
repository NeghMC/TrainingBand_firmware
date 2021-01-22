/*
 * GPS.h
 *
 *  Created on: 5 sty 2021
 *      Author: NeghM
 */

#ifndef GPS_H_
#define GPS_H_

typedef struct __attribute__((packed)) {
	float latitude;
	float longitude;
	uint8_t isValid;
} location_t;

void GPS_init();
void GPS_enable();
void GPS_createTask(void);
location_t GPS_getCurrentLocation();

#endif /* GPS_H_ */
