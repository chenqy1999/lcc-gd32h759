#include "bsp_exti_adc.h"

static void Exti_Int(void)
{
    //GPIO初始化
    // 1-gpio时钟使能
    EXTI_ADC_GPIO_CLK_ENABLE(EXTI_ADC_GPIO_CLK);

    // 2-GPIO模式设置
	  gpio_af_set(EXTI_ADC_GPIO_PORT,EXTI_ADC_GPIO_PIN,GPIO_AF_13);
    gpio_mode_set(EXTI_ADC_GPIO_PORT,GPIO_MODE_INPUT,GPIO_PUPD_PULLDOWN,EXTI_ADC_GPIO_PIN);

    //exti初始化
    exti_init(EXTI_ADC_LINE,EXTI_ADC_MODE,EXTI_ADC_TRIG_TYPE);

    //exti触发事件使能
    exti_event_enable(EXTI_ADC_LINE);

}




void Exti_ADC_Config(void)
{
    //EXTI初始化
    Exti_Int();
}

