/******************************************************************************
 * @file    pwm.h
 * @brief   PWM 模块（匹配原理图引脚：两路带死区互补PWM + 硬件过流刹车 + 74HC245方向控制）
 ******************************************************************************/
#ifndef __PWM_H
#define __PWM_H
#include "gd32h7xx.h"
#include <stdint.h>

/* ==================== 互补PWM输出引脚定义（对应原理图APWM5~8） ==================== */
/* 第一对互补PWM：CH0主输出 + MCH0互补输出 灯丝电路PWM输出 固定频率200kHz*/

#define CH0_PORT        GPIOK
#define CH0_PIN         GPIO_PIN_2      // PK2/APWM8 -> TIMER7_BRKIN0
#define CH0_AF          GPIO_AF_3

#define MCH0_PORT       GPIOK
#define MCH0_PIN        GPIO_PIN_1      // PK1/APWM7 -> TIMER7_CH0
#define MCH0_AF         GPIO_AF_3

/* 第二对互补PWM：CH1主输出 + MCH1互补输出 逆变电路PWM输出 固定占空比50%*/

#define MCH1_PORT        GPIOK
#define MCH1_PIN         GPIO_PIN_0      // PK0/APWM6 -> TIMER0_MCH0
#define MCH1_AF          GPIO_AF_1

#define CH1_PORT       GPIOJ
#define CH1_PIN        GPIO_PIN_11     // PJ11/APWM5 -> TIMER0_CH1
#define CH1_AF         GPIO_AF_1

/* ==================== PWM定时器参数 ==================== */
#define PWM_TIMER0       TIMER0
#define PWM_TIMER0_RCU   RCU_TIMER0
#define PWM_TIMER7       TIMER7
#define PWM_TIMER7_RCU   RCU_TIMER7
#define TIMER_CLK_HZ    (300000000UL)    // APB2定时器时钟 300MHz

// 固定PWM工作频率
#define PWM_FREQ        300000UL         // 默认200kHz
#define PWM_FREQ2       20000UL         // 默认20kHz
#define PWM_DUTY        50            // 50% (0~100)

// 死区时间参数
#define DEADTIME_CLK_DIV    TIMER_CKDIV_DIV1   // 死区时钟分频
#define DEADTIME_CNT        30                 // 死区计数值

/* ==================== 软启动参数 ==================== */
#define START_FREQ      300000UL         // 起始频率 300kHz
#define END_FREQ        200000UL         // 结束频率 200kHz
#define FREQ_STEP       500UL            // 每步下降 500Hz
#define STEP_MS         10UL             // 每步间隔 10ms


#define UART_PWM_CTRL     0         // 等待第1字节：PWM控制字
#define UART_STATE_DATA     1       // 等待第2字节：待保存变量值



/* ==================== 全局变量声明 ==================== */
extern volatile uint32_t current_freq;   // 当前PWM频率

extern volatile uint8_t pwm_1s_finish;
extern volatile uint8_t cmd_ready;       // 完整指令就绪标志
extern uint8_t rx_ctrl_byte;      // 第1字节：PWM控制字 (0x00=不启动, 0x01=启动2秒)
extern uint8_t rx_data_byte;      // 第2字节：待赋值的变量数据
extern volatile uint8_t pwm_running;
extern uint8_t saved_value;

extern volatile uint8_t uart_ctrl_var;
extern uint8_t uart_rx_state;

extern uint8_t run_fil_status;
extern uint8_t run_pwm_status;
extern volatile uint8_t stage;



/* ==================== 函数声明 ==================== */
void PWM_GPIO_Config(void);          // 配置所有相关GPIO
void PWM_TIMER0_Config(void);         // 初始化TIMER0（互补PWM+死区）
void PWM_TIMER7_Config(void);         // 初始化TIMER7（互补PWM+死区）
//void SoftStart_Timer_Config(void);   // 配置软启动定时器
//void Update_PWM_Frequency(uint32_t new_freq); // 更新PWM频率
void pwm_emergency_stop(void);

void Command_Process(void);

void Timer15_1s_Init(void);

void PWM_Start(void);

void PWM_Stop(void);

void Print_Hex_Bin(uint8_t val);

void Update_PWM_Frequency(uint32_t new_freq);

float Get_Timer_Freq(void);

float Get_Timer_Duty(uint32_t timer_periph);

void Set_Timer_Duty(uint32_t timer_periph, uint32_t duty);




#endif /* __PWM_H */