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

#define GPGGA   0
#define GPGLL   1
#define GPGLV   2
#define GPGSA   3
#define GPRMC   4
#define GPVTG   5

typedef struct { char payload[NMEA_LEN]; } nmea_conf_package;

void setupGPS(uint32_t baud, uint16_t hertz);
void NEO_M6_setSentence(nmea_conf_package * configPacket, uint8_t sentence);
void changeFreq(uint16_t hertz);

#define NMEA_LEN   16
#define FREQ_LEN   14
#define BAUD_LEN   28

#endif /* NEO_M6_CMD_PARSER_H_ */
