/**
 ******************************************************************************
 * @file    bsp_gpio_led.c
 * @author     embedfire
 * @version     V1.0
 * @date        2023
 * @brief   led应用函数接口
 ******************************************************************************
 * @attention
 *
 * 实验平台：野火 GDH759IMK6-GD32开发板
 * 论坛      ：http://www.firebbs.cn
 * 官网      ：https://embedfire.com/
 * 淘宝      ：https://yehuosm.tmall.com/
 *
 ******************************************************************************
 */

#include "led/bsp_gpio_led.h"

/**
 * @brief  初始化控制LED的IO
 * @param  无
 * @retval 无
 */
void LED_GPIO_Config(void)
{

    /*开启LED相关的GPIO外设时钟*/
    rcu_periph_clock_enable(LED1_GPIO_CLK_PORT);
    rcu_periph_clock_enable(LED2_GPIO_CLK_PORT);
    rcu_periph_clock_enable(LED3_GPIO_CLK_PORT);

    /*选择要控制的GPIO引脚、设置GPIO模式为通用推挽输出、设置GPIO速率为50MHz*/
    gpio_mode_set(LED1_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED1_GPIO_PIN);
    gpio_output_options_set(LED1_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, LED1_GPIO_PIN);

    /*选择要控制的GPIO引脚、设置GPIO模式为通用推挽输出、设置GPIO速率为50MHz*/
    gpio_mode_set(LED2_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED2_GPIO_PIN);
    gpio_output_options_set(LED2_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, LED2_GPIO_PIN);

    /*选择要控制的GPIO引脚、设置GPIO模式为通用推挽输出、设置GPIO速率为50MHz*/
    gpio_mode_set(LED3_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED3_GPIO_PIN);
    gpio_output_options_set(LED3_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, LED3_GPIO_PIN);

    /* 关闭所有led灯*/
    gpio_bit_set(LED1_GPIO_PORT, LED1_GPIO_PIN);
    gpio_bit_set(LED2_GPIO_PORT, LED2_GPIO_PIN);
    gpio_bit_set(LED3_GPIO_PORT, LED3_GPIO_PIN);
}

/*********************************************END OF FILE**********************/
