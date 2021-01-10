/*
 * NEO_M^_cmd_parser.h
 *
 *  Created on: 10 sty 2021
 *      Author: NeghM
 */

#ifndef NEO_M6_CMD_PARSER_H_
#define NEO_M6_CMD_PARSER_H_

#include <stdint.h>

#define NMEA_LEN   16
#define FREQ_LEN   14
#define BAUD_LEN   28

typedef struct { char payload[NMEA_LEN]; } nmea_conf_package;
extern const nmea_conf_package nmeaDefaultConfigPackage;

void neo6mGPS_begin(void * send(void*,uint32_t), void* receive(void*,uint32_t));
void setupGPS(uint32_t baud, uint16_t hertz);
void NEO_M6_setSentence(nmea_conf_package * configPacket);
void changeFreq(uint16_t hertz);

#define NMEA_LEN   16
#define FREQ_LEN   14
#define BAUD_LEN   28

#endif /* NEO_M6_CMD_PARSER_H_ */
