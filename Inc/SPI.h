/*
 * SPI.h
 *
 *  Created on: 11 paź 2020
 *      Author: NeghM
 */

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

void SPI_init(void);
void SPI_blokingRead(uint8_t * buffer, uint8_t size);
void SPI_blokingWrite(uint8_t * buffer, uint8_t size);
void SPI_csSet(void);
void SPI_csClear(void);

#endif /* SPI_H_ */
