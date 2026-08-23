/*!
    \file    systick.c
    \brief   the systick configuration file

    \version 2017-02-10, V1.0.0, firmware for GD32F30x
    \version 2018-10-10, V1.1.0, firmware for GD32F30x
    \version 2018-12-25, V2.0.0, firmware for GD32F30x
    \version 2020-09-30, V2.1.0, firmware for GD32F30x 
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

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

#include "gd32h7xx.h"
#include "systick/systick.h"

volatile static uint32_t delay;

/*!
    \brief      systick初始化配置
    \param[in]  none
    \param[out] none
    \retval     none
*/
void Systick_Int(void)
{
    /* SystemCoreClock / 1000    1ms中断一次
	 * SystemCoreClock / 100000	 10us中断一次
	 * SystemCoreClock / 1000000 1us中断一次
	 */
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000)){
        /* capture error */
        while (1){
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

/*!
    \brief      1s级延时程序
    \param[in]  count: 计数总时长，单位秒
    \param[out] none
    \retval     none
*/
void Delay_S(uint32_t count)
{
    Delay_Ms(count*1000);
}

/*!
    \brief      1ms级延时程序
    \param[in]  count: 计数总时长，单位毫秒
    \param[out] none
    \retval     none
*/
void Delay_Ms(uint32_t count)
{
    delay = count;
    if (SysTick_Config(SystemCoreClock / 1000)){
        /* 配置错误 */
        while (1);
    }
    /* 配置 systick 中断优先级 */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
    while(0U != delay){
    }
}

/*!
    \brief      us级延时程序
    \param[in]  count: 计数总时长，单位微秒
    \param[out] none
    \retval     none
*/
void Delay_Us(uint32_t count)
{
    delay = count;
    if (SysTick_Config(SystemCoreClock / 1000000))
    {
        /* 配置错误 */
        while (1);
    }
    /* 配置 systick 中断优先级 */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
    while(0U != delay);
}

/*!
    \brief      计数递减函数
    \param[in]  none
    \param[out] none
    \retval     none
*/
void Delay_Decrement(void)
{
    if (0U != delay)
    {
        delay--;
    }
}

// couter 减1的时间 等于 1/systick_clk
// 当counter 从 reload 的值减小到0的时候，为一个循环，如果开启了中断则执行中断服务程序，
// 同时 CTRL 的 countflag 位会置1
// 这一个循环的时间为 reload * (1/systick_clk)
/**
  * @brief  us延时程序
  * @param  us：计数总时长，单位微秒
  *	@note   使用软件查询的方式实现 （中断时间1u秒级/10u秒级）
  * @retval  无
  */
void SysTick_Delay_Us(__IO uint32_t us)
{
	uint32_t i;
    uint32_t temp_10us;
    uint32_t temp_us;
    temp_10us = us/10;
    temp_us = us%10;
    if(temp_10us)
    {
        SysTick_Config(SystemCoreClock/100000);
        for(i=0;i<temp_10us;i++)
        {
            // 当计数器的值减小到0的时候，CRTL寄存器的位16会置1	
            while( !((SysTick->CTRL)&(1<<16)) );
        }
        // 关闭SysTick定时器
        SysTick->CTRL &=~SysTick_CTRL_ENABLE_Msk;
    }
    if(temp_us)
    {
        SysTick_Config(SystemCoreClock/1000000);
        for(i=0;i<temp_us;i++)
        {
            // 当计数器的值减小到0的时候，CRTL寄存器的位16会置1	
            while( !((SysTick->CTRL)&(1<<16)) );
        }
        // 关闭SysTick定时器
        SysTick->CTRL &=~SysTick_CTRL_ENABLE_Msk;
    } 
}
/**
  * @brief  ms延时程序
  * @param  ms:计数总时长，单位毫秒
  *	@note   使用软件查询的方式实现 （中断时间毫秒级）
  * @retval  无
  */
void SysTick_Delay_Ms(__IO uint32_t ms)
{
	uint32_t i;	
	SysTick_Config(SystemCoreClock/1000);
	
	for(i=0;i<ms;i++)
	{
		// 当计数器的值减小到0的时候，CRTL寄存器的位16会置1
		// 当置1时，读取该位会清0
		while( !((SysTick->CTRL)&(1<<16)) );
	}
	// 关闭SysTick定时器
	SysTick->CTRL &=~ SysTick_CTRL_ENABLE_Msk;
}

uint64_t SysTick_GetMs_Now(void)
{
		return g_ms;
}




/*********************************************END OF FILE**********************/
