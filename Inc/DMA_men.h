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
	DMA_5,
	DMA_6,
	DMA_7
};

void DMA_init(void);
void DMA_transfer(enum dma_number nr, uint8_t peryph, void * peryphAddr, void * memAddr, uint16_t size, uint8_t bool_memToPeryph);
void DMA_reserve(enum dma_number nr);
void DMA_waitForTransferEnd(enum dma_number nr);

#endif /* DMA_MEN_H_ */
