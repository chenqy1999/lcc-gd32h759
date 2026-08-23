#include "IO_Agreement.h"
#include "main.h"
#include "usart/bsp_usart_debug.h"


extern void Soft_Delay(__IO uint32_t nCount);


volatile uint8_t XL74HC595_states = 0xFF;

void TIMER14_Config(void)
{

    rcu_periph_clock_enable(RCU_TIMER14);
    timer_deinit(TIMER14);

	
		timer_parameter_struct timer_initpara;
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler         = 2399;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 99;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;

	
    timer_init(TIMER14, &timer_initpara);

		timer_interrupt_flag_clear(TIMER14, TIMER_INT_FLAG_UP);
		timer_interrupt_enable(TIMER14, TIMER_INT_UP);
		nvic_irq_enable(TIMER14_IRQn, 2, 2);

    timer_enable(TIMER14);
}



void XL74HC595_Init(void)
{
		rcu_periph_clock_enable(RCU_GPIOG);
		gpio_mode_set(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14);
    gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14);
	
		// sck, rck, set low
		gpio_bit_reset(GPIOG, GPIO_PIN_12| GPIO_PIN_13| GPIO_PIN_14);

	
		TIMER14_Config();
}


void XL74HC595_Send(void)
{	
		for(volatile uint16_t i = 0x80; i ; i>>=1)
		{
#if 1

			if(XL74HC595_states & i) 
			{

				GPIO_BOP(GPIOG)=GPIO_PIN_14;
			}
			else 
			{
				GPIO_BC(GPIOG)=GPIO_PIN_14;
			}
#endif
			gpio_bit_set(GPIOG, GPIO_PIN_13);
			gpio_bit_reset(GPIOG, GPIO_PIN_13);
		}
		gpio_bit_set(GPIOG, GPIO_PIN_12);
		gpio_bit_reset(GPIOG, GPIO_PIN_12);
}

void CD4013_Reset(void)
{
		gpio_bit_set(GPIOB, GPIO_PIN_0);
		Soft_Delay(10);
		gpio_bit_reset(GPIOB, GPIO_PIN_0);
}


void CD4013_Init(void)
{
		rcu_periph_clock_enable(RCU_GPIOB);

		gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_0);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_0);
	
		CD4013_Reset();
}





