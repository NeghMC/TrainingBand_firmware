/*
 * power_men.c
 *
 *  Created on: 7 sty 2021
 *      Author: NeghM
 */

#include <stm32l031xx.h>

void clock_setup() {
	/* Because of balance between power consumption and performance
	 * I decided to setup 1MHz internal clock
	 */

	//turn on internal crystal, divide by 4
	 RCC->CR |= RCC_CR_HSION | RCC_CR_HSIDIVF | RCC_CR_HSIDIVEN;

	 //wait for HSI crystal be stable
	 while(!(RCC->CR & RCC_CR_HSIRDY))
	 ;

	 //activate prefetch buffer but it should already be on
	 FLASH->ACR |= FLASH_ACR_PRFTEN;

	 // Flash 2 wait state
	 FLASH->ACR |= (uint32_t)0x1;


	 //configure RCC and PLL settings while PLL is off
	 RCC->CFGR |= RCC_CFGR_HPRE_DIV4;  //AHB prescaler

	 //set clock source
	 RCC->CFGR |= RCC_CFGR_SW_HSI;
	 while (!(RCC->CFGR & RCC_CFGR_SWS_PLL)) ;
}

void ADC_init(void) { // PB0, ADC_IN8
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	ADC1->IER |= ADC_IER_EOCIE;
	NVIC_EnableIRQ(ADC1_IRQn);
	ADC1->CFGR1 |= ADC_CFGR1_AUTOFF;
	ADC1->CHSELR = ADC_CHSELR_CHSEL8;
	//ADC1->SMPR |= ADC_SMPR_SMP_0 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_2;

	// calibration
	ADC1->CR |= ADC_CR_ADCAL;
	while ((ADC1->ISR & ADC_ISR_EOCAL) == 0);
	ADC1->ISR |= ADC_ISR_EOCAL;

	ADC1->CR |= ADC_CR_ADEN;


	//while ((ADC1->ISR & ADC_ISR_EOC) == 0) /* wait end of conversion */


}

void ADC_startConvertion(void) {
	ADC1->CR |= ADC_CR_ADSTART;
}

void ADC1_IRQHandler(void) {
	return;
}

void Charger_init(void) {
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~(GPIO_MODER_MODE11_0 | GPIO_MODER_MODE11_1);
}

void peryph_en(void) {
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~(GPIO_MODER_MODE12_1);
}
