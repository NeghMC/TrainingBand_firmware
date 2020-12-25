/*
 * USART.h
 *
 *  Created on: 10 paź 2020
 *      Author: NeghM
 */

#ifndef USART_H_
#define USART_H_

#include <stdint.h>

void UART_init();
void USART_blokingTransmit(uint8_t data);
void USART_prints(char * data);
uint8_t USART_blokingReceive(void);

#endif /* USART_H_ */
