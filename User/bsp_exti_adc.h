#ifndef __BSP_EXTI_ADC_H
#define __BSP_EXTI_ADC_H

#include "gd32h7xx.h"

// 引脚定义
#define EXTI_ADC_GPIO_CLK_ENABLE       rcu_periph_clock_enable
#define EXTI_ADC_GPIO_CLK              RCU_GPIOA

#define EXTI_ADC_GPIO_PORT             GPIOA
#define EXTI_ADC_GPIO_PIN              GPIO_PIN_0

#define EXTI_ADC_MODE                  EXTI_EVENT
#define EXTI_ADC_LINE                  EXTI_0
#define EXTI_ADC_TRIG_TYPE             EXTI_TRIG_RISING

void Exti_ADC_Config(void);

#endif // !__BSP_EXTI_ADC_H
