/**
  ******************************************************************************
  * @file    bsp_adc.c
  * @author  embedfire
  * @version V1.0
  * @date    2023-xx-xx
  * @brief   ADC2 ??????(PC2_C / PC3_C)
  ******************************************************************************
  */

#include "adc/bsp_adc.h"
#include "usart/bsp_usart_debug.h"
#include "gd32h7xx_ospi.h"
#include "gd32h7xx_ospim.h"
#include "pid.h"
#include <string.h>


extern void Soft_Delay(__IO uint32_t nCount);

volatile int16_t adc_data[8];

float k[8] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

uint8_t rms_Window_Index;
float rms_Window[128] = {0};
float rms_Window_Avg = 0;



/*-------------------- NVIC ??????,??? --------------------*/
static void ADC_NVIC_Config(void)
{
    /* ???????,?????? NVIC */
}

/*-------------------- ADC GPIO ???(PC2?PC3) --------------------*/
static void ADCX_GPIO_Config(void)
{
    /* ?? GPIOC ?? */
    ADC_GPIO_APBXCLOCK_FUN(ADC_GPIO_CLK);

    /* PC2 analog input set*/
    gpio_mode_set(ADC_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, ADC_PIN);
}

/*-------------------- ADC ?????? --------------------*/
static void ADCX_Mode_Config(void)
{
    /* 打开ADC时钟 */
    ADC_APBXCLKCMD_FUN(ADC_CLK);

    /* 初始化adc */
    adc_deinit(ADCX);

    /* 配置ADC时钟  */
    adc_clock_config(ADCX, ADC_CLK_SYNC_HCLK_DIV6);

    /* ???? */
    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);

    /* 扫描模式，多通道才要，单通道不需要 */
    adc_special_function_config(ADCX, ADC_SCAN_MODE, DISABLE);

    /* 使能连续转换模式 */
    adc_special_function_config(ADCX, ADC_CONTINUOUS_MODE, ENABLE);

    /* 转换结果右对齐 */
    adc_data_alignment_config(ADCX, ADC_DATAALIGN_RIGHT);

    /* ADC分辨率 12Bit */
    adc_resolution_config(ADCX, ADC_RESOLUTION_12B);

    /* 转换通道1个 */
    adc_channel_length_config(ADCX, ADC_REGULAR_CHANNEL, 1U);

    /* 不用外部触发转换，软件开启即可 ,ADC外部触发配置 */
    adc_external_trigger_config(ADCX, ADC_REGULAR_CHANNEL, EXTERNAL_TRIGGER_DISABLE);

    /* ??????????? */
    // adc_internal_channel_config(ADC_CHANNEL_INTERNAL_TEMPSENSOR, DISABLE);

    /* ?? ADC */
    adc_enable(ADCX);

    /* ??????? */
    adc_calibration_mode_config(ADCX, ADC_CALIBRATION_OFFSET_MISMATCH);
    adc_calibration_number(ADCX, ADC_CALIBRATION_NUM1);
    adc_calibration_enable(ADCX);
		
}

/**
 * @brief  ??? ADC(GPIO + ??)
 */
void ADCX_Init(void)
{
    ADCX_GPIO_Config();
    ADCX_Mode_Config();
}

/**
 * @brief  ???? ADC ????
 * @param  channel: ADC_CHANNEL_0 (PC2_C) ? ADC_CHANNEL_1 (PC3_C)
 * @retval 12bit ?? (0~4095)
 */
uint16_t ADC_Read_Channel(uint8_t channel)
{
    /* ??????? 0 ????????,???? 480 ?? */
    adc_regular_channel_config(ADCX, 0, channel, 480);

    /* ? EOC ?? */
    adc_flag_clear(ADCX, ADC_FLAG_EOC);

    /* ???????? */
    adc_software_trigger_enable(ADCX, ADC_REGULAR_CHANNEL);

    /* ?????? */
    while (RESET == adc_flag_get(ADCX, ADC_FLAG_EOC));

    /* ???? */
    return adc_regular_data_read(ADCX);
}

void AD7606C_Init_Soft(void)
{
	#if 1
		rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOF);
		/***************************GPIO setup*******************************/
		//
	
		// GPIO C output pins, PC8(timer7,ch2)->convst, PC12->SDI, PC13->RESET, PC11->CS
		gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
		gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, 
										GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13);
		gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, 
																  GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13);
		gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_8);
		gpio_af_set(GPIOC, GPIO_AF_3, GPIO_PIN_8);

		
		// GPIO F output pins, PF10->SCLK
		gpio_mode_set(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_10);
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_10);
	
		// GPIO C input pins
		gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, 
                  GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10);
									
	#endif	
	

	
		// exti 7
		/* set EXTI7 downedge interrupt */
		rcu_periph_clock_enable(RCU_SYSCFG); // 1. ????????
		syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN7); // 2. ? EXTI7 ????????? PC7 ?
    exti_init(EXTI_7, EXTI_INTERRUPT, EXTI_TRIG_FALLING); // downedge exti
    exti_interrupt_flag_clear(EXTI_7);                   // clear flag
    exti_interrupt_enable(EXTI_7);                      // enable exti

    // priority:EXTI5_9_IRQn
    nvic_irq_enable(EXTI5_9_IRQn, 2, 2);

#if 1
		// set CS disable
    gpio_bit_set(GPIOC, GPIO_PIN_11);
		
		// set SCLK high
		GPIO_BOP(GPIOF) = GPIO_PIN_10;

		// reset AD7606C
		gpio_bit_set(GPIOC, GPIO_PIN_13);
		Soft_Delay(100);
		
		gpio_bit_reset(GPIOC, GPIO_PIN_13);
#endif
	
		/****************************CONVST Timer setup**********************************************/
    rcu_periph_clock_enable(RCU_TIMER7);
    timer_deinit(TIMER7);

    /*timer7 init */
		timer_parameter_struct timer_initpara;
    timer_initpara.prescaler         = 29;          // 300MHZ->10MHZ
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 100;          // 10MHZ->1MHZ
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_init(TIMER7, &timer_initpara);
		
		
		// CH2 primary output config
		
		timer_oc_parameter_struct timer_chinitpara;
		timer_channel_output_struct_para_init(&timer_chinitpara);
    timer_chinitpara.outputstate   = TIMER_CCX_ENABLE;
    timer_chinitpara.ocpolarity    = TIMER_OC_POLARITY_HIGH; 
    timer_chinitpara.ocidlestate   = TIMER_OC_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER7, TIMER_CH_2, &timer_chinitpara);
		// single pulse mode
		timer_primary_output_config(TIMER7, ENABLE);
		timer_channel_output_mode_config(TIMER7, TIMER_CH_2, TIMER_OC_MODE_PWM1);

		timer_channel_output_pulse_value_config(TIMER7, TIMER_CH_2, 10);		// pulse 2us
		// timer_single_pulse_mode_config(TIMER7, TIMER_SP_MODE_SINGLE);
		// gpio_output_options_set(GPIOK,GPIO_OTYPE_PP,GPIO_OSPEED_60MHZ,GPIO_PIN_0 | GPIO_PIN_1);
		
		timer_disable(TIMER7);
}

// AD7606C Pin set, work @ input & output mode
void AD7606C_Init(void)
{
	#if 0
		rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOF);
		/***************************GPIO setup*******************************/
		//
	
		// GPIO C output pins, PC8(timer7,ch2)->convst, PC12->SDI, PC13->RESET, PC11->CS
		gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
		gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, 
										GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13);
		gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, 
																  GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13);
		gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_8);
		gpio_af_set(GPIOC, GPIO_AF_3, GPIO_PIN_8);

		
		// GPIO F output pins, PF10->SCLK
		gpio_mode_set(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_10);
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_10);
	
		// GPIO C input pins
		gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, 
                  GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10);
									
	#endif	
	
		// reset设置
		gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_13);
		gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_13);
		// reset AD7606C
		#if 1
		gpio_bit_set(GPIOC, GPIO_PIN_13);
		Soft_Delay(100);
		gpio_bit_reset(GPIOC, GPIO_PIN_13);
		
		#endif
	
		// exti 7
		/* set EXTI7 downedge interrupt */
		rcu_periph_clock_enable(RCU_SYSCFG); // 1. ????????
		syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN7); // 2. ? EXTI7 ????????? PC7 ?
    exti_init(EXTI_7, EXTI_INTERRUPT, EXTI_TRIG_FALLING); // downedge exti
    exti_interrupt_flag_clear(EXTI_7);                   // clear flag
    exti_interrupt_enable(EXTI_7);                      // enable exti

    // priority:EXTI5_9_IRQn
    nvic_irq_enable(EXTI5_9_IRQn, 2, 2);

#if 0
		// set CS disable
    gpio_bit_set(GPIOC, GPIO_PIN_11);
		
		// set SCLK high
		GPIO_BOP(GPIOF) = GPIO_PIN_10;

		// reset AD7606C
		gpio_bit_set(GPIOC, GPIO_PIN_13);
		Soft_Delay(100);
		
		gpio_bit_reset(GPIOC, GPIO_PIN_13);
#endif
	
		/****************************CONVST Timer setup**********************************************/
    rcu_periph_clock_enable(RCU_TIMER7);
    timer_deinit(TIMER7);

    /*timer7 init */
		timer_parameter_struct timer_initpara;
    timer_initpara.prescaler         = 29;          // 240MHZ->10MHZ
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 100;          // 10MHZ->1MHZ
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_init(TIMER7, &timer_initpara);
		
		
		// CH2 primary output config
		
		timer_oc_parameter_struct timer_chinitpara;
		timer_channel_output_struct_para_init(&timer_chinitpara);
    timer_chinitpara.outputstate   = TIMER_CCX_ENABLE;
    timer_chinitpara.ocpolarity    = TIMER_OC_POLARITY_HIGH; 
    timer_chinitpara.ocidlestate   = TIMER_OC_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER7, TIMER_CH_2, &timer_chinitpara);
		// single pulse mode
		timer_primary_output_config(TIMER7, ENABLE);
		timer_channel_output_mode_config(TIMER7, TIMER_CH_2, TIMER_OC_MODE_PWM1);

		timer_channel_output_pulse_value_config(TIMER7, TIMER_CH_2, 5);		// pulse 2us
		// timer_single_pulse_mode_config(TIMER7, TIMER_SP_MODE_SINGLE);
		// gpio_output_options_set(GPIOK,GPIO_OTYPE_PP,GPIO_OSPEED_60MHZ,GPIO_PIN_0 | GPIO_PIN_1);
		
		timer_enable(TIMER7);
}



uint32_t adc_cnt = 0;
uint32_t ptr = 0;
uint32_t adc_cnts[500] = {0};
float KVs[500];
float FILs[500];


#if 1
void AD7606C_ReadChannels(void)
{
    volatile uint64_t sdoa_data = 0;
    volatile uint64_t sdob_data = 0;
    uint32_t istat = 0;	
	
		// set CS enable
    gpio_bit_reset(GPIOC, GPIO_PIN_11);
		Soft_Delay(1); 
		
    // 8 channels * 16bit / 2 wires = 64
    for(int i = 0; i < 64; i++) {

				// SDOA, SDOB buffer
				istat = GPIO_ISTAT(GPIOC);
				sdoa_data = (sdoa_data << 1) | ((istat & GPIO_PIN_9) ? 1 : 0);
			  sdob_data = (sdob_data << 1) | ((istat & GPIO_PIN_10) ? 1 : 0);			
			
				// set SCLK low
				GPIO_BC(GPIOF) = GPIO_PIN_10;
				Soft_Delay(0);

        // set SCLK high
				GPIO_BOP(GPIOF) = GPIO_PIN_10;
				Soft_Delay(0);
    }

    // set CS disable
    gpio_bit_set(GPIOC, GPIO_PIN_11);

    // write to buffer
    adc_data[0] = (int16_t)((sdoa_data >> 48) & 0xFFFF); // CH1
    adc_data[1] = (int16_t)((sdoa_data >> 32) & 0xFFFF); // CH2
    adc_data[2] = (int16_t)((sdoa_data >> 16) & 0xFFFF); // CH3
    adc_data[3] = (int16_t)(sdoa_data & 0xFFFF);         // CH4

    adc_data[4] = (int16_t)((sdob_data >> 48) & 0xFFFF); // CH5
    adc_data[5] = (int16_t)((sdob_data >> 32) & 0xFFFF); // CH6
    adc_data[6] = (int16_t)((sdob_data >> 16) & 0xFFFF); // CH7
    adc_data[7] = (int16_t)(sdob_data & 0xFFFF);         // CH8
		
		
		/*
		if ((adc_cnt++ % 100) == 0) 
		{
			adc_cnts[ptr] = adc_cnt - 1;
			KVs[ptr] = adc_data[3] * ADC_INT2FLOAT_5V;
			FILs[ptr++] = adc_data[2] * ADC_INT2FLOAT_5V;
		}
		*/
}

#endif


volatile uint8_t buffer[16];	// 临时缓冲区
void ReadData(void)
{	
#if 1
		OSPI_INS(OSPI0) = 0xffffffff;
		//ospi_data_length_config(OSPI0, 16-1);
		ospi_receive(OSPI0, (uint8_t*)adc_data);
	/*
		uint32_t* adc_data_ptr = (uint32_t*)adc_data;
    for(volatile uint8_t i=0; i<4; i++)
    {
        while(RESET == ospi_flag_get(OSPI0, OSPI_FLAG_TC));
        *(adc_data_ptr++) = OSPI_DATA(OSPI0);
    }
	*/
	
#endif
		
		if ((adc_cnt++ % 100) == 0)
		{
			adc_cnts[ptr] = adc_cnt - 1;
			KVs[ptr++] = adc_data[3] * ADC_INT2FLOAT_5V;
		}
}




/*********************************OSPI始终设置***************************************/
#define ADC_BUFFER_LEN	16
uint16_t adc_raw_data[8 * ADC_BUFFER_LEN]; 



void Ospi0GpioConfig(void)
{
    /* 使能 GPIO 时钟 (假设 SCK/CSN/IO0/IO1 在 GPIOB/GPIOD 上) */
		rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOD);
		rcu_periph_clock_enable(RCU_GPIOF);


    /* 1. SCK 引脚配置为 OSPIM_P0_SCK 复用，PF10 */
    gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);
		gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_10);
		gpio_af_set(GPIOF, GPIO_AF_9, GPIO_PIN_10);



    /* 2. CSN 引脚配置为 OSPIM_P0_CSN 复用，PC11 */
		
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
		gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_11);
		gpio_af_set(GPIOC, GPIO_AF_9, GPIO_PIN_11);
	
	/*
		gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_11);
		gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_11);
		*/

    /* 3. 只配置 IO0 (DOUTA) 引脚复用 */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
		gpio_af_set(GPIOC, GPIO_AF_9, GPIO_PIN_9); // 假设 PD11 是 OSPIM_P0_IO0

    /* 4. 只配置 IO1 (DOUTB) 引脚复用 */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);
		gpio_af_set(GPIOC, GPIO_AF_9, GPIO_PIN_10); // 假设 PD12 是 OSPIM_P0_IO1
		
		/* busy 引脚配置，PC7 */
		gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_7);
		

    /* 
       ?? 注意：
       对应 OSPIM_P0_IO2 和 OSPIM_P0_IO3 的芯片引脚：
       不要调用 gpio_af_set()，保持默认浮空输入或配置为普通 GPIO 即可。
       这样它们就不会参与 OSPI 数据总线传输。
    */
		/*  convst  引脚配置*/
		gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
		gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_8);
		gpio_af_set(GPIOC, GPIO_AF_3, GPIO_PIN_8);

}


// OSPIM 接入 OSPI0
void ospim_cfg_init(void)
{
/* 1. 使能 OSPIM 时钟 (通常与 OSPI0 一起使能) */
    rcu_periph_clock_enable(RCU_OSPIM);
    rcu_periph_clock_enable(RCU_OSPI0);

    /* 2. 复位 OSPIM 模块 */
    ospim_deinit();

    /* 3. 配置 SCK 信号线 */
    ospim_port_sck_source_select(OSPIM_PORT0, OSPIM_SCK_SOURCE_OSPI0_SCK); // 时钟源选择 OSPI0_SCK
    ospim_port_sck_config(OSPIM_PORT0, OSPIM_PORT_SCK_ENABLE);             // 使能 PORT0 的 SCK

    /* 4. 配置 CSN (片选) 信号线 */
    ospim_port_csn_source_select(OSPIM_PORT0, OSPIM_CSN_SOURCE_OSPI0_CSN); // 片选源选择 OSPI0_CSN
    ospim_port_csn_config(OSPIM_PORT0, OSPIM_PORT_CSN_ENABLE);             // 使能 PORT0 的 CSN

    /* 5. 配置 低4位数据线 IO[3:0] (AD7606 的 DOUTA/DOUTB 在 IO0/IO1 上) */
    ospim_port_io3_0_source_select(OSPIM_PORT0, OSPIM_SRCPLIO_OSPI0_IO_LOW); // 数据源选择 OSPI0_IO[3:0]
    ospim_port_io3_0_config(OSPIM_PORT0, OSPIM_IO_LOW_ENABLE);             // 使能 PORT0 的 IO[3:0]

    /* 6. 配置 高4位数据线 IO[7:4] (因为只用了双线 SDR 接收 AD7606，高 4 位可以禁用以省电) */
    ospim_port_io7_4_config(OSPIM_PORT0, OSPIM_IO_HIGH_DISABLE);
}

uint32_t ospi_cmd_trigger_word = 0;


void OspiAd7606CInit(void)
{
		Ospi0GpioConfig();
		ospim_cfg_init();
		OspiStartEeadTransfer();
		AD7606C_Init();
	
	
		// BusyTriggerDmaConfig();
}


void BusyTriggerDmaConfig(void)
{
    rcu_periph_clock_enable(RCU_DMA0);
		rcu_periph_clock_enable(RCU_DMA1);
    rcu_periph_clock_enable(RCU_DMAMUX);
		rcu_periph_clock_enable(RCU_MDMA);


    /* ---------------- 1. 配置 DMAMUX ---------------- */
    // 将 EXTI0 (BUSY 引脚 PD0) 映射为 DMAMUX 的事件生成器源
		dmamux_gen_parameter_struct gen_init;
    dmamux_gen_struct_para_init(&gen_init);

    gen_init.trigger_id       = DMAMUX_TRIGGER_EXTI7;      // 绑定 EXTI7（对应PD0的BUSY引脚）
    gen_init.trigger_polarity = DMAMUX_GEN_FALLING;        // BUSY下降沿触发
    gen_init.request_number   = 1;                         // 每次触发只产生1个DMA请求

    dmamux_request_generator_init(DMAMUX_GENCH0, &gen_init); // 初始化生成器0
    dmamux_request_generator_channel_enable(DMAMUX_GENCH0);   // 使能生成器
	

    /* ---------------- 2. 配置 DMA1_CH0 , memory->OSPI_ADDR, 生成ospi 请求 ---------------- */
    dma_deinit(DMA1, DMA_CH0);
		dma_single_data_parameter_struct dma_init_struct;
    dma_single_data_para_struct_init(&dma_init_struct);
		dma_init_struct.request 						= DMA_REQUEST_GENERATOR0;
    dma_init_struct.periph_addr         = (uint32_t)&OSPI_INS(OSPI0);  // 目标: OSPI ins 寄存器，被写时会立刻出发工作
    dma_init_struct.memory0_addr        = (uint32_t)&ospi_cmd_trigger_word; // 源: 内存里的配置字
    dma_init_struct.direction           = DMA_MEMORY_TO_PERIPH;        // 内存 -> 外设
    dma_init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc          = DMA_MEMORY_INCREASE_DISABLE;
    dma_init_struct.periph_memory_width   = DMA_PERIPH_WIDTH_32BIT;       // 32 位寄存器写入
    dma_init_struct.number              = 1;                           // 触发一次只写 1 个 Word
    dma_init_struct.priority            = DMA_PRIORITY_ULTRA_HIGH;
    
    dma_single_data_mode_init(DMA1, DMA_CH0, &dma_init_struct);
   
    // 开启循环模式/自动重加载，确保每次 BUSY 下降沿都能自动再次触发
    dma_circulation_enable(DMA1, DMA_CH0);
    dma_channel_enable(DMA1, DMA_CH0);
		
		
		
		/* ---------------- 3. 配置 MDMA_CH0 , OSPI_DATA->memory, 内存接受数据 ---------------- */
		
		mdma_parameter_struct mdma_init_struct;
		mdma_para_struct_init(&mdma_init_struct);
    mdma_init_struct.request = MDMA_REQUEST_OSPI0_FT;		/* OSPI FIFO threshold request */
    mdma_init_struct.trans_trig_mode =  MDMA_BUFFER_TRANSFER;    /* hardware trigger mode */
    mdma_init_struct.priority = MDMA_PRIORITY_HIGH;
		
		mdma_init_struct.source_addr = (uint32_t)&OSPI_DATA(OSPI0);
		mdma_init_struct.source_inc = MDMA_SOURCE_INCREASE_DISABLE;
		mdma_init_struct.source_data_size = MDMA_SOURCE_DATASIZE_32BIT;
		
		mdma_init_struct.destination_addr = (uint32_t)&adc_raw_data;
		mdma_init_struct.dest_inc = MDMA_DESTINATION_INCREASE_32BIT;
		mdma_init_struct.dest_data_dize = MDMA_DESTINATION_DATASIZE_32BIT;
		mdma_init_struct.endianness = MDMA_LITTLE_ENDIANNESS;
		
		mdma_init_struct.buff_trans_len = 16;
		mdma_init_struct.tbytes_num_in_block = 2 * 8 * ADC_BUFFER_LEN;
		
		mdma_init(MDMA_CH0, &mdma_init_struct);
    mdma_channel_enable(MDMA_CH0);
}




// 配置 OSPI 每次读取 8 字节 (IO0 发4字节, IO1 发4字节 -> 相当于总共64个SCLK)
void OspiStartEeadTransfer(void)
{
	    /* 1. 使能 OSPI 时钟 */
    rcu_periph_clock_enable(RCU_OSPI0);
    // 配置 OSPI 引脚 (GPIO_AF_x 需替换为你实际板子的引脚配置)
    // SCLK -> OSPI_CLK, DOUTA -> OSPI_IO0, DOUTB -> OSPI_IO1

    /* 2. 复位 OSPI */
    ospi_deinit(OSPI0);

    /* 3. 配置 OSPI 基础参数 */
		ospi_parameter_struct param;
    ospi_struct_init(&param);
    param.prescaler         = 23;                 // 设置 SCLK 频率 (根据系统时钟分频，建议 < 20MHz)
    param.fifo_threshold    = OSPI_FIFO_THRESHOLD_4;                 // FIFO 阈值
    param.sample_shift      = OSPI_SAMPLE_SHIFTING_HALF_CYCLE; 
    param.cs_hightime      	= OSPI_CS_HIGH_TIME_1_CYCLE;
		param.memory_type				= OSPI_STANDARD_MODE;
		param.delay_hold_cycle	= OSPI_DELAY_HOLD_NONE;
	
		/*  配置 OSPI 读取模式 */
    ospi_regular_cmd_struct cmd = {0};
		cmd.operation_type	 = OSPI_OPTYPE_COMMON_CFG; 
    cmd.ins_mode 				 = OSPI_INSTRUCTION_NONE; // 无指令阶段
    cmd.addr_mode    		 = OSPI_ADDRESS_NONE;     // 无地址阶段
    cmd.alter_bytes_mode = OSPI_ALTERNATE_BYTES_NONE;
    cmd.dummy_cycles     = 8;                     // 无 Dummy 周期
    cmd.data_mode        = OSPI_DATA_2_LINES;     // 双线接收 (IO0 + IO1)
    cmd.nbdata      		 = 16-1;                     // 总共读取 16字节
    cmd.data_dtr_mode    = OSPI_DADTR_MODE_DISABLE;     // SDR 模式

    // 配置 OSPI 为间接接收模式 (Indirect Read)
		OSPI_INS(OSPI0) = 1;
		ospi_init(OSPI0, &param);

		
    ospi_enable(OSPI0);
		ospi_command_config(OSPI0, &param, &cmd);
		ospi_functional_mode_config(OSPI0, OSPI_INDIRECT_READ);
		ospi_data_mode_config(OSPI0, OSPI_DATA_2_LINES);
		ospi_fifo_level_config(OSPI0, 3);

		
		// 关闭dma
		ospi_dma_disable(OSPI0);
}

void Start_ADC(void)
{
	adc_cnt = 0;
	ptr = 0;
	timer_enable(TIMER7);
}

void Stop_ADC(void)
{
	timer_disable(TIMER7);
}

void ADC_Result(void)
{
		printf("ADC_Result\n");
		for(int i = 0; i < 500 ; ++i)
		{
			printf("At %d step, KV result %.4f, FIL: %.4f\n", adc_cnts[i], KVs[i], FILs[i]);
		}
		printf("End Print\n");
}


float Set_PID_Fil_Current(float setVal)
{
	return setVal * setVal;
}


float RMS_Avg(float FilCurrent)
{
		float tmp = FilCurrent * FilCurrent * INV_RMS_N;
		rms_Window_Avg = rms_Window_Avg - rms_Window[rms_Window_Index] + tmp;
		rms_Window[rms_Window_Index] = tmp;
		rms_Window_Index = (rms_Window_Index + 1) % RMS_N;
		// %128 == &128-1
		return rms_Window_Avg;
}

void RMS_Init(void)
{
		memset(rms_Window, 0, sizeof(rms_Window));
		rms_Window_Avg = 0;
		rms_Window_Index = 0;
}



/*********************************************END OF FILE**********************/

