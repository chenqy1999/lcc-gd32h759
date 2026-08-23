#ifndef __FILAMENT_H
#define __FILAMENT_H


#include "gd32h7xx_timer.h"

#define FILAMENT_TIMER TIMER3


extern uint32_t duty;

void Fil_Pwn_Timer_Init(void);

void Start_Heat(void);

void Stop_Heat(void);

void Fil_Dma_Init(void);

void Fil_Init(void);




#endif