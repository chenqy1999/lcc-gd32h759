#include "filament.h"



#define PK1SET 				GPIO_PIN_1
#define PK2SET 				GPIO_PIN_2
#define PK12RESET 		((GPIO_PIN_1|GPIO_PIN_2 ) << 16)
#define FIL_PORT 			(GPIOK)
#define FIL_PINS 			(GPIO_PIN_1|GPIO_PIN_2)
#define FIL_DMA				DMA1
#define FIL_DMA_CH		DMA_CH2


uint32_t duty = 200;
uint32_t cmd[4] = {PK12RESET, PK1SET, PK12RESET, PK2SET};

uint32_t cmdHigh[2] = {PK1SET, PK12RESET};
uint32_t cmdLow[2] = {PK12RESET, PK2SET};



void Fil_GPIO_Init(void)
{
		rcu_periph_clock_enable(RCU_GPIOK);
		gpio_mode_set(FIL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, FIL_PINS);
    gpio_output_options_set(FIL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_12MHZ, FIL_PINS);
}


void Fil_Pwn_Timer_Init(void)
{
    rcu_periph_clock_enable(RCU_TIMER3);
    timer_deinit(FILAMENT_TIMER);
    
    // 300MHz / 30000 = 10kHz计数时钟，计数20000次 = 2秒
		timer_parameter_struct timer_initpara;
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler         = 6 - 1;
    timer_initpara.alignedmode       = TIMER_COUNTER_CENTER_BOTH;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 1250 - 1;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(FILAMENT_TIMER, &timer_initpara);
	

		timer_oc_parameter_struct timer_ocinitpara;
    timer_channel_output_struct_para_init(&timer_ocinitpara);
    timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_channel_output_config(FILAMENT_TIMER, TIMER_CH_0, &timer_ocinitpara);
    timer_channel_output_pulse_value_config(FILAMENT_TIMER, TIMER_CH_0, duty+100);
		timer_channel_output_mode_config(FILAMENT_TIMER, TIMER_CH_0, TIMER_OC_MODE_TIMING);
		timer_auto_reload_shadow_enable(FILAMENT_TIMER);
		/* 使能TIMER3的CMP的DMA请求（更新事件） */
		timer_dma_enable(FILAMENT_TIMER, TIMER_DMA_CH0D);
		
		
		
		timer_channel_output_struct_para_init(&timer_ocinitpara);
    timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_channel_output_config(FILAMENT_TIMER, TIMER_CH_1, &timer_ocinitpara);
    timer_channel_output_pulse_value_config(FILAMENT_TIMER, TIMER_CH_1, duty);
		timer_channel_output_mode_config(FILAMENT_TIMER, TIMER_CH_1, TIMER_OC_MODE_TIMING);
		timer_auto_reload_shadow_enable(FILAMENT_TIMER);
		/* 使能TIMER3的CMP的DMA请求（更新事件） */
		timer_dma_enable(FILAMENT_TIMER, TIMER_DMA_CH1D);
	
		timer_disable(FILAMENT_TIMER);
}


void Start_Heat(void)
{
		GPIO_BOP(FIL_PORT) = PK12RESET;
		timer_enable(FILAMENT_TIMER);
}

void Stop_Heat(void)
{
		timer_disable(FILAMENT_TIMER);
		GPIO_BOP(FIL_PORT) = PK12RESET;
}


void Fil_Dma_Init1(void)
{
		rcu_periph_clock_enable(RCU_DMAMUX);
		rcu_periph_clock_enable(RCU_DMA1);
	
	    /* 配置两个DMA通道 */
    dma_multi_data_parameter_struct dma_initpara;
	
		/* 公共参数 */
		dma_multi_data_para_struct_init(&dma_initpara);
		dma_initpara.periph_addr  				= (uint32_t)&GPIO_BOP(FIL_PORT);
    dma_initpara.periph_inc   				= DMA_PERIPH_INCREASE_DISABLE;
    dma_initpara.memory_inc   				= DMA_MEMORY_INCREASE_ENABLE;
    dma_initpara.direction    				= DMA_MEMORY_TO_PERIPH;
    dma_initpara.number       				= 2;
    dma_initpara.priority     				= DMA_PRIORITY_HIGH;
    dma_initpara.circular_mode 				= DMA_CIRCULAR_MODE_ENABLE;
		dma_initpara.memory_burst_width = DMA_MEMORY_BURST_SINGLE;
		dma_initpara.periph_burst_width = DMA_PERIPH_BURST_SINGLE;
		dma_initpara.critical_value     = DMA_FIFO_1_WORD;    // FIFO 阈值
		dma_initpara.memory_width				=	DMA_MEMORY_WIDTH_32BIT;
		dma_initpara.periph_width				=	DMA_PERIPH_WIDTH_32BIT;
	
    // --- 配置 DMA 通道2，响应比较事件 ---
    dma_deinit(FIL_DMA, FIL_DMA_CH);
		dma_initpara.request							= DMA_REQUEST_TIMER3_CH0;
    dma_initpara.memory0_addr					= (uint32_t)cmdHigh;
    dma_multi_data_mode_init(FIL_DMA, FIL_DMA_CH, &dma_initpara);
    dma_channel_enable(FIL_DMA, FIL_DMA_CH);
		
		dma_deinit(DMA1, DMA_CH3);
		dma_initpara.request							= DMA_REQUEST_TIMER3_CH1;
    dma_initpara.memory0_addr					= (uint32_t)cmdLow;
    dma_multi_data_mode_init(FIL_DMA, DMA_CH3, &dma_initpara);
    dma_channel_enable(FIL_DMA, DMA_CH3);
}


void Fil_Init(void)
{
		Fil_GPIO_Init();
		Fil_Pwn_Timer_Init();
		Fil_Dma_Init1();
}




