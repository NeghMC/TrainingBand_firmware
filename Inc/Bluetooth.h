/*
 * Bluetooth.h
 *
 *  Created on: Dec 24, 2020
 *      Author: NeghM
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

void BT_init();
void BT_enable();
void BT_disable();
void BT_Transmitt(uint8_t * buffer, uint16_t length);

#endif /* BLUETOOTH_H_ */
