/*!
    \file    gd32h7xx_it.c
    \brief   interrupt service routines

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


#include <string.h>
#include "gd32h7xx_it.h"
#include "adc/bsp_adc.h"
#include "led/bsp_gpio_led.h"
#include "usart/bsp_usart_debug.h"
#include "main.h"
#include "IO_Agreement.h"
#include "pwm.h"
#include "pid.h"
#include "soft_protect.h"
#include "filament.h"
#include "systick/systick.h"


uint32_t MIN_PERIOD_LIMIT = 2000;
uint32_t MAX_PERIOD_LIMIT = 2000;

volatile uint64_t g_ms = 0;



extern __IO uint16_t adc_convertedvalue;
extern float adc_vol;

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
    /* if NMI exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
    /* if DebugMon exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
    /* if SVC exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PendSV_Handler(void)
{
    /* if PendSV exception occurs, go to infinite loop */
    while (1)
    {
    }
}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{
    g_ms++;
}



// AD7606 interrupt hanlder, triggered after AD7606C BUSY downedge
#if 1

static uint32_t maxFreq = 400000;
static uint32_t minFreq = 190000;
static uint32_t maxDuty = 575;
static uint32_t minDuty = 5;

void EXTI5_9_IRQHandler(void)
{
    if (SET == exti_interrupt_flag_get(EXTI_7))
    {
				AD7606C_ReadChannels();
				//ReadData();
				float err;
				float freq_now;
				float fil_duty_now = Get_Timer_Duty(TIMER3);
				float Fil_Current_Real = adc_data[2] * ADC_INT2FLOAT_5V *10.f;
				float fil_Current_Rms_Now;

			
				switch(stage) 
				{
					case 2:
					case 3:
						// PWM PID
					
							err = PID_Update(&pwmPID, adc_data[3] * ADC_INT2FLOAT_5V * 20000.f);
							
							if ( (Get_Timer_Freq() + coeff * err) > maxFreq )
							{
									freq_now = (float) maxFreq;
							}		
							else if ( (Get_Timer_Freq() + coeff * err) < minFreq )
							{
									freq_now = (float) minFreq;
							}
							else
							{
									freq_now = Get_Timer_Freq() + coeff * err;
							}
					
							//Update_PWM_Frequency((uint32_t)freq_now);
							
					case 1:
					case 4:
						// 灯丝PID
							fil_Current_Rms_Now = RMS_Avg(Fil_Current_Real);
							if (rms_Window_Index == 0)
							{
									err = PID_Update(&filPID, fil_Current_Rms_Now);
									if (fil_duty_now + err > maxDuty)
									{
											fil_duty_now = (float) maxDuty;
									}		
									else if (fil_duty_now + err < minDuty)
									{
											fil_duty_now = (float) minDuty;
									}
									else
									{
											fil_duty_now += err;
									}
									//Set_Timer_Duty(TIMER3, (uint32_t)fil_duty_now);
							}
					break;
					
					default:
						
					break;
				}
				// clean interrupt
        exti_interrupt_flag_clear(EXTI_7);
    }
    /* ARC故障处理 */
    if(SET == exti_interrupt_flag_get(ARC_EXTI_LINE))
    {
        arc_err = ERR_FAULT;
        pwm_emergency_stop();
        exti_interrupt_flag_clear(ARC_EXTI_LINE);
    }
    /* FIL故障处理 */
    if(SET == exti_interrupt_flag_get(FIL_EXTI_LINE))
    {
        fil_err = ERR_FAULT;
        pwm_emergency_stop();
        exti_interrupt_flag_clear(FIL_EXTI_LINE);
    }
		
}
#endif

static volatile uint16_t cnt;
void TIMER14_IRQHandler(void)
{
    if(SET == timer_interrupt_flag_get(TIMER14, TIMER_INT_FLAG_UP))
    {
			timer_interrupt_flag_clear(TIMER14, TIMER_INT_FLAG_UP);
			// update XL74HC595 states
			if(cnt++==500) {cnt = 0; XL74HC595_states ^= 0x01;}
			XL74HC595_Send();
        
    }
}


void TIMER0_IRQHandler(void)
{
    if(SET == timer_interrupt_flag_get(TIMER0, TIMER_INT_FLAG_UP))
    {
			timer_disable(TIMER0);
			timer_interrupt_disable(TIMER0, TIMER_INT_UP);
			timer_interrupt_flag_clear(TIMER0, TIMER_INT_FLAG_UP);
			// update XL74HC595 states
    }
}


// 软起动
const uint32_t period[8] = {};
const uint32_t freq[8] = {};
static uint8_t period_cnt = 0;
static uint8_t num_cnt = 0;
void TIMER1_IRQHandler(void)
{
	if(SET == timer_interrupt_flag_get(TIMER0, TIMER_INT_FLAG_UP))
	{
		if (period_cnt >= num_cnt) {
			timer_disable(TIMER0);
			timer_disable(TIMER1);
			timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);
			return;
		}
		
		TIMER_CREP0(TIMER1) = (uint32_t)period[period_cnt] & 0xFFU;
		TIMER_CAR(TIMER0) = (uint32_t)freq[period_cnt];
		period_cnt++;
		
		timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);
	}
}



/* receive CMD from debug CLI */

#if 0
void USART0_IRQHandler(void)
{
		// printf("USART0_IRQHANDLER\n");
    if(usart_interrupt_flag_get(DEBUG_USARTX, USART_INT_FLAG_RBNE)==SET)
    {
        volatile char ch;

        ch = usart_data_receive(DEBUG_USARTX);

        if(ch == '\r' || ch == '\n')
        {
            uart_rxbuf[uart_rx_index] = '\0';

            uart_cmd_ready = 1;

            uart_rx_index = 0;
					
						if (strcmp((char *)uart_rxbuf,"ERRRESET")==0) {
							CD4013_Reset();
							printf("CD4013_Reset\n");
							uart_cmd_ready = 0;
						}
        }
        else
        {
            if(uart_rx_index < RX_BUF_SIZE-1)
            {
								cnt++;
                uart_rxbuf[uart_rx_index++] = ch;
            }
            else
            {
                uart_rx_index = 0;
            }
        }
    }
}
#endif

#if 1
void USART0_IRQHandler(void)
{
    if(usart_interrupt_flag_get(DEBUG_USARTX, USART_INT_FLAG_RBNE) == SET)
    {
        uint8_t ch = usart_data_receive(DEBUG_USARTX);
        uart_ctrl_var = ch; // 同步更新观测变量，调试可见
        
        switch(uart_rx_state)
        {
            case UART_PWM_CTRL:
                // 接收第1字节：PWM控制字
                rx_ctrl_byte = ch;
                uart_rx_state = UART_STATE_DATA;
                break;
                
            case UART_STATE_DATA:
                // 接收第2字节：数据值，收齐完整指令
                rx_data_byte = ch;
                cmd_ready = 1;
                uart_rx_state = UART_PWM_CTRL; // 回到初始状态等待下一条指令
                break;
						
						case 02:
								CD4013_Reset();
								printf("ERR Protect Reset\n!");
								break;
                
            default:
                uart_rx_state = UART_PWM_CTRL;
                break;
        }
    }
}
#endif



/**
 * @brief  TIMER15 中断服务函数：2秒到后关闭PWM 
 */
void TIMER15_IRQHandler(void)
{
	if (timer_interrupt_flag_get(TIMER15, TIMER_INT_FLAG_UP) == SET)
	{
			switch (stage)
			{
				// 灯丝开始工作，pwm不工作，由
				case 0:
					Start_ADC();
					Start_Heat();
					PWM_Start();
					printf("Stage %d， timer count:%d, System Mils:%lld\n", stage, TIMER_CNT(TIMER15), SysTick_GetMs_Now());
					++stage;
				break;
				
				// 灯丝工作，pwm也开启
				case 1:
					freq_cnt = 0;
					PWM_Stop();
					Stop_Heat();
					Stop_ADC();
					// PWM_Start();
					printf("Stage %d， timer count:%d, System Mils:%lld\n", stage, TIMER_CNT(TIMER15), SysTick_GetMs_Now());
					++stage;
				break;
				
				case 2:
					//Start_Heat();
				printf("Stage %d， timer count:%d, System Mils:%lld\n", stage, TIMER_CNT(TIMER15), SysTick_GetMs_Now());
					++stage;
				break;
				
				// pwm 结束，灯丝依然工作
				case 3:
					//PWM_Stop();                // 关闭PWM输出
					printf("Stage %d， timer count:%d, System Mils:%lld\n", stage, TIMER_CNT(TIMER15), SysTick_GetMs_Now());
					pwm_1s_finish = 1;         // 置位完成标志
					++stage;
				break;
				
				// 灯丝结束
				case 4:
					//Stop_Heat();
					//PWM_Stop();
					printf("Stage %d， timer count:%d, System Mils:%lld\n", stage, TIMER_CNT(TIMER15), SysTick_GetMs_Now());
					timer_disable(TIMER15); // 停止定时器

					stage = 101;
				break;
				
				// 默认输出
				default:
					printf("At Stage %d, timer15 still works\n", stage);
				break;
			}
			timer_interrupt_flag_clear(TIMER15, TIMER_INT_FLAG_UP);
	}
}





/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHANDLER(void)
{

}*/
/*********************************************END OF FILE**********************/
