#ifndef __IO_AGREEMENT_H
#define __IO_AGREEMENT_H

#include "gd32h7xx.h"

void XL74HC595_Init(void);

void XL74HC595_Send(void);

void CAN_Init(void);

void TIMER14_Config(void);

void CD4013_Init(void);

void CD4013_Reset(void);

extern volatile uint8_t XL74HC595_states;



#endif