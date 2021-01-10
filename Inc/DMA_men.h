/*
 * DMA_men.h
 *
 *  Created on: 10 sty 2021
 *      Author: NeghM
 */

#ifndef DMA_MEN_H_
#define DMA_MEN_H_


enum dma_number {
	DMA_1,
	DMA_2,
	DMA_4,
	DMA_5
};

void DMA_init(void);
void DMA_transfer(enum dma_number nr, uint8_t peryph, void * peryphAddr, void * memAddr, uint16_t size, uint8_t bool_memToPeryph);

#endif /* DMA_MEN_H_ */
