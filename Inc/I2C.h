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

void I2C_ReadReg(uint8_t devAddress, uint8_t regAddress, uint8_t * data, uint8_t length);
void I2C_WriteReg(uint8_t devAddress, uint8_t regAddress, uint8_t * data, uint16_t length);

#endif /* I2C_H_ */
