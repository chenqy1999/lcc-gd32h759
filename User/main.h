/*!
    \file    main.h
    \brief   the header file of main
    
    \version 2023-03-31, V1.0.0, demo for GD32H7xx
*/

/*
    Copyright (c) 2023, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#ifndef __MAIN_H
#define __MAIN_H


#include "gd32h7xx.h"


// ADC7606C port
#define AD7606_SPI               SPI0
#define AD7606_SPI_RCU           RCU_SPI0

// CONVST port
#define AD7606_CONVST_PORT       GPIOC
#define AD7606_CONVST_PIN        GPIO_PIN_8
#define AD7606_CONVST_RCU        RCU_GPIOC

// BUSY interrupt, finish data convt at down edge and request interrupt (PC7)
#define AD7606_BUSY_PORT         GPIOC
#define AD7606_BUSY_PIN          GPIO_PIN_7
#define AD7606_BUSY_RCU          RCU_GPIOC

// CS SPI (PC11)
#define AD7606_CS_PORT           GPIOC
#define AD7606_CS_PIN            GPIO_PIN_11
#define AD7606_CS_RCU            RCU_GPIOC

// ??????? (? TIMER1_CH0 ??? PA0 ???? CONVST)
#define AD7606_TIMER_RCU         RCU_TIMER1
#define AD7606_TIMER             TIMER1
#define AD7606C_SCLK						 4998988       
#define AD7606C_SCLK_CH				 3

static int16_t ad7606_raw_data[8];


/* led spark function */
void led_spark(void);



#endif /* MAIN_H */
