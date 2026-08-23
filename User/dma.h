#ifndef __DMA_H
#define __DMA_H

#include "gd32h7xx.h"
#include "gd32h7xx_dma.h"


#define ADC_BUF_LEN 1024
/* DMA Cache */
__attribute__((aligned(32)))
extern uint16_t adc_buffer[ADC_BUF_LEN];


void ADC_GPIO_Config(void);
void ADC_DMA_Start(void);
void TRGSEL_CONFIG(void);
uint16_t GET_VAL_NOW(void);



#endif