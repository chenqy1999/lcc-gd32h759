/**
 ******************************************************************************
 * @file        bsp_usart.c
 * @author     embedfire
 * @version     V1.0
 * @date        2023
 * @brief   重定向c库printf函数到usart端口
 ******************************************************************************
 * @attention
 *
 * 实验平台：野火 GDH759IMK6 GD32开发板
 * 论坛      ：http://www.firebbs.cn
 * 官网      ：https://embedfire.com/
 * 淘宝      ：https://yehuosm.tmall.com/
 *
 ******************************************************************************
 */

#include "usart/bsp_usart_debug.h"



/* 开启接收中断 */



/**
 * @brief  USART GPIO 配置,工作参数配置
 * @param  无
 * @retval 无
 */
void USART_Config(void)
{
    // 打开GPIO外设的时钟
    DEBUG_USART_GPIO_APBXCLKCMD(DEBUG_USART_GPIO_CLK_TX);
    DEBUG_USART_GPIO_APBXCLKCMD(DEBUG_USART_GPIO_CLK_RX);

    // 打开串口外设的时钟
    DEBUG_USART_APBXCLKCMD(DEBUG_USART_CLK);

    // 将USART Tx的GPIO配置为推挽复用模式
    gpio_af_set(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_RX_GPIO_AF, DEBUG_USART_TX_GPIO_PIN);
    gpio_mode_set(DEBUG_USART_TX_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, DEBUG_USART_TX_GPIO_PIN);
    gpio_output_options_set(DEBUG_USART_TX_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, DEBUG_USART_TX_GPIO_PIN);

    // 将USART Rx的GPIO配置为浮空输入模式
    gpio_af_set(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_TX_RX_GPIO_AF, DEBUG_USART_RX_GPIO_PIN);
    gpio_mode_set(DEBUG_USART_RX_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, DEBUG_USART_RX_GPIO_PIN);
    gpio_output_options_set(DEBUG_USART_RX_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, DEBUG_USART_RX_GPIO_PIN);

    // 配置串口的工作参数
    usart_deinit(DEBUG_USARTX);

    // 配置波特率
    usart_baudrate_set(DEBUG_USARTX, DEBUG_USART_BAUDRATE);
    // 配置 针数据字长
    usart_word_length_set(DEBUG_USARTX, USART_WL_8BIT);
    // 配置停止位
    usart_stop_bit_set(DEBUG_USARTX, USART_STB_1BIT);
    // 配置校验位
    usart_parity_config(DEBUG_USARTX, USART_PM_NONE);
    // 配置硬件流控制
    usart_hardware_flow_rts_config(DEBUG_USARTX, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(DEBUG_USARTX, USART_CTS_DISABLE);

    // 配置工作模式，收发一起，使能串口接收中断
    usart_receive_config(DEBUG_USARTX, USART_RECEIVE_ENABLE);
    usart_transmit_config(DEBUG_USARTX, USART_TRANSMIT_ENABLE);

		// 串口中断优先级配置
    /* 配置中断源：USART 、配置抢占优先级、 配置响应优先级、使能中断通道*/
    nvic_irq_enable(DEBUG_USART_IRQ, 1U, 1U);


    // 使能串口
    usart_enable(DEBUG_USARTX);
		
		
		// 开启接收中断
		usart_interrupt_enable(DEBUG_USARTX, USART_INT_RBNE);
		

}

/**
 * @brief 发送一个字节函数
 * @param usart_periph：USARTx(x=0,1,2)/UARTx(x=3,4)
 * @param ch:要发送的数据
 * @note  无
 * @retval 无
 */
void Usart_SendByte(uint32_t usart_periph, uint8_t ch) /*****************  发送一个字节 **********************/
{
    /* 发送一个字节数据到USART */
    usart_data_transmit(usart_periph, ch);

    /* 等待发送数据寄存器为空 */
    while (RESET == usart_flag_get(usart_periph, USART_FLAG_TBE))
        ;
}

/**
 * @brief 发送8位的数组函数
 * @param usart_periph：USARTx(x=0,1,2)/UARTx(x=3,4)
 * @param array:要发送的数组
 * @param num:数组大小
 * @note  无
 * @retval 无
 */
void Usart_SendArray(uint32_t usart_periph, uint8_t *array, uint16_t num) /****************** 发送8位的数组 ************************/
{
    uint8_t i;

    for (i = 0; i < num; i++)
    {
        /* 发送一个字节数据到USART */
        Usart_SendByte(usart_periph, array[i]);
    }
    /* 等待发送完成 */
    while (RESET == usart_flag_get(usart_periph, USART_FLAG_TC))
        ;
}

/**
 * @brief 发送字符串函数
 * @param usart_periph：USARTx(x=0,1,2)/UARTx(x=3,4)
 * @param str:要发送的数据
 * @note  无
 * @retval 无
 */
void Usart_SendString(uint32_t usart_periph, char *str) /*****************  发送字符串 **********************/
{
    unsigned int k = 0;
    do
    {
        Usart_SendByte(usart_periph, *(str + k));
        k++;
    } while (*(str + k) != '\0');

    /* 等待发送完成 */
    while (RESET == usart_flag_get(usart_periph, USART_FLAG_TC))
        ;
}

/**
 * @brief 发送指定长度的字符串
 * @param pusart_periph(x=0,1,2)/UARTx(x=3,4)
 * @param str:要发送的数据
 * @param strlen:长度
 * @note  无
 * @retval 无
 */
void Usart_SendString_Length(uint32_t pusart_periph, uint8_t *str, uint32_t strlen) /****************** 发送8位的数组 ************************/
{
    unsigned int k = 0;
    do
    {
        /* 发送一个字节数据到USART */
        Usart_SendByte(pusart_periph, *(str + k));
        k++;
    } while (k < strlen);
}

/**
 * @brief 发送一个16位数函数
 * @param usart_periph：USARTx(x=0,1,2)/UARTx(x=3,4)
 * @param data:要发送的数据
 * @note  无
 * @retval 无
 */
void Usart_SendHalfWord(uint32_t usart_periph, uint16_t data) /*****************  发送一个16位数 **********************/
{
    uint8_t temp_h, temp_l;

    /* 取出高八位 */
    temp_h = (data & 0XFF00) >> 8;
    /* 取出低八位 */
    temp_l = data & 0XFF;

    /* 发送高八位 */
    usart_data_transmit(usart_periph, temp_h);
    while (RESET == usart_flag_get(DEBUG_USARTX, USART_FLAG_TBE))
        ;

    /* 发送低八位 */
    usart_data_transmit(usart_periph, temp_l);
    while (RESET == usart_flag_get(DEBUG_USARTX, USART_FLAG_TBE))
        ;
}

/**
 * @brief 将一个字符写入到文件中)
 * @param ch: 要写入的字符
 * @param f: 指向FILE结构的指针
 * @note  无
 * @retval 成功，返回该字符
 */
int fputc(int ch, FILE *f) /// 重定向c库函数printf到串口，重定向后可使用printf函数
{
    /* 发送一个字节数据到串口 */
    usart_data_transmit(DEBUG_USARTX, (uint8_t)ch);

    /* 等待发送完毕 */
    while (RESET == usart_flag_get(DEBUG_USARTX, USART_FLAG_TBE))
        ;
    return ch;
}

/**
 * @brief 从文件中读出一个字符
 * @param f: 指向FILE结构的指针
 * @note  无
 * @retval 成功，返回接收字符
 */
int fgetc(FILE *f) /// 重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
{
    /* 等待串口输入数据 */
    while (RESET == usart_flag_get(DEBUG_USARTX, USART_FLAG_RBNE))
        ;
    return (int)usart_data_receive(DEBUG_USARTX);
}
/*********************************************END OF FILE**********************/
