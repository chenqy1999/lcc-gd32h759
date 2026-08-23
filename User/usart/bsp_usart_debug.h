#ifndef __BSP_USART_DEBUG_H
#define	__BSP_USART_DEBUG_H


#include "gd32h7xx.h"
#include <stdio.h>

#define  DEBUG_USARTX_MODE                   0

#if (DEBUG_USARTX_MODE == 0)
    //串口0-USART0
    #define  DEBUG_USARTX                   USART0
    #define  DEBUG_USART_CLK                RCU_USART0
    #define  DEBUG_USART_APBXCLKCMD         rcu_periph_clock_enable
    #define  DEBUG_USART_BAUDRATE           115200
    
    // USART GPIO 引脚宏定义
    #define  DEBUG_USART_GPIO_APBXCLKCMD    rcu_periph_clock_enable
    #define  DEBUG_USART_GPIO_CLK_TX        RCU_GPIOA
    #define  DEBUG_USART_GPIO_CLK_RX        RCU_GPIOA
    #define  DEBUG_USART_TX_GPIO_PORT       GPIOA   
    #define  DEBUG_USART_TX_GPIO_PIN        GPIO_PIN_9
    #define  DEBUG_USART_RX_GPIO_PORT       GPIOA
    #define  DEBUG_USART_RX_GPIO_PIN        GPIO_PIN_10
    #define  DEBUG_USART_TX_RX_GPIO_AF      GPIO_AF_7

    #define  DEBUG_USART_IRQ                USART0_IRQn
    #define  DEBUG_USART_IRQHANDLER         USART0_IRQHandler
		
		#define RX_BUF_SIZE 64
		static volatile char uart_rxbuf[RX_BUF_SIZE];
		static volatile uint8_t uart_rx_index = 0;
		static volatile uint8_t uart_cmd_ready = 0;
		
#endif



void USART_Config(void);
void Usart_SendByte( uint32_t usart_periph, uint8_t ch);
void Usart_SendString( uint32_t usart_periph, char *str);
void Usart_SendString_Length(uint32_t pusart_periph, uint8_t *str, uint32_t strlen);
void Usart_SendHalfWord( uint32_t usart_periph, uint16_t ch);

#endif /* __BSP_USART_H */
