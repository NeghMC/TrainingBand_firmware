/*
 * I2C.h
 *
 *  Created on: 7 sty 2021
 *      Author: NeghM
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

void I2C_init(void);
void I2C_Transmit(const uint8_t * buffer, uint16_t length, uint8_t address);
void I2C_ReadRegister(uint8_t * buffer, uint8_t length, uint8_t devAddress, uint8_t regAddress);

#endif /* I2C_H_ */
