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

void setSentence(char * configPacket, char NMEA_num, uint8_t enable);

extern const char GPGGA_header[];

#define NMEA_LEN   16
#define FREQ_LEN   14
#define BAUD_LEN   28

#endif /* NEO_M6_CMD_PARSER_H_ */
