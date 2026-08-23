
#include "dma.h"
#include "gd32h7xx_trigsel.h"
#include "adc/bsp_adc.h"
#include "gd32h7xx_adc.h"


uint16_t adc_buffer[ADC_BUF_LEN];

void ADC_GPIO_Config(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);

    gpio_mode_set(GPIOC,
                  GPIO_MODE_ANALOG,
                  GPIO_PUPD_NONE,
                  GPIO_PIN_2);
}

void ADC_DMA_Config(void)
{
		rcu_periph_clock_enable(RCU_DMAMUX);
    rcu_periph_clock_enable(RCU_DMA0);
	
    dma_deinit(DMA0, DMA_CH0);
	
		// dma parameter clear & init
		dma_single_data_parameter_struct dma_init;
		dma_single_data_para_struct_init(&dma_init);
    dma_init.direction = DMA_PERIPH_TO_MEMORY;
    dma_init.periph_addr = (uint32_t)&ADC_RDATA(ADC2);
    dma_init.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init.memory0_addr = (uint32_t)adc_buffer;
    dma_init.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;
		dma_init.request = DMA_REQUEST_ADC2;
    dma_init.circular_mode = DMA_CIRCULAR_MODE_ENABLE;
    dma_init.number = ADC_BUF_LEN;

    dma_init.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(
        DMA0,
        DMA_CH0,
        &dma_init
    );
		adc_dma_request_after_last_enable(ADC2);
		dma_flag_clear(DMA0, DMA_CH0, DMA_FLAG_FTF);
    dma_channel_enable(DMA0, DMA_CH0);
}

/*--------------------------------------------------
 * TIMER15
 * ????ADC
 *-------------------------------------------------*/


/*--------------------------------------------------
 * ADC2
 *-------------------------------------------------*/
void ADC2_Config(void)
{
    rcu_periph_clock_enable(RCU_ADC2);

    adc_deinit(ADC2);

    /* ADC?? */
    adc_clock_config(ADC2, ADC_CLK_SYNC_HCLK_DIV6);
	
	    /* ???? */
    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);

    /* 扫描模式，多通道才要，单通道不需要 */
    adc_special_function_config(ADC2, ADC_SCAN_MODE, DISABLE);

    /* 使能连续转换模式 */
    adc_special_function_config(ADC2, ADC_CONTINUOUS_MODE, ENABLE);

    /* 转换结果右对齐 */
    adc_data_alignment_config(ADC2, ADC_DATAALIGN_RIGHT);

    /* ADC分辨率 12Bit */
    adc_resolution_config(ADC2, ADC_RESOLUTION_12B);

    /* ??? */
    adc_channel_length_config(ADC2, ADC_REGULAR_CHANNEL, 1U);

    /* Rank0 = CH0 */
    adc_regular_channel_config(ADC2, 0, ADC_CHANNEL_0, 480);
		

    /* ????? */
    adc_external_trigger_config(ADC2, ADC_REGULAR_CHANNEL, EXTERNAL_TRIGGER_DISABLE);
		

    /* DMA */
    adc_dma_mode_enable(ADC2);
		/* continus request DMA*/
		adc_dma_request_after_last_enable(ADC2);

    /* ADC?? */
    adc_enable(ADC2);

    for(volatile uint32_t i=0; i<10000; i++);

    /* ?? */
		adc_calibration_mode_config(ADC2, ADC_CALIBRATION_OFFSET_MISMATCH);
    adc_calibration_number(ADC2, ADC_CALIBRATION_NUM32);
    adc_calibration_enable(ADC2);
		volatile FlagStatus eoc = adc_flag_get(ADC2, ADC_FLAG_EOC);
		adc_software_trigger_enable(ADC2, ADC_REGULAR_CHANNEL);
}

/*--------------------------------------------------
 * ???
 *-------------------------------------------------*/
void ADC_DMA_Start(void)
{
    ADC_GPIO_Config();
    ADC_DMA_Config();
    ADC2_Config();
}

uint16_t GET_VAL_NOW(void)
{
		static volatile uint32_t remain;	
		remain = dma_transfer_number_get(
            DMA0,
            DMA_CH0);
    		// volatile uint16_t ADC_Result = ADC_Read_Channel(ADC_CHANNEL_0);
		return adc_buffer[ADC_BUF_LEN - remain];
}




