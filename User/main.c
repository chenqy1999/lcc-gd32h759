/******************************************************************************
* @file     main.c
* @brief    GD32H7 ??? ADC(ADC2??0/1) + PWM ???? + ??????
*           PC2_C = ADC2_IN0 (ADC_CHANNEL_0)
*           PC3_C = ADC2_IN1 (ADC_CHANNEL_1)
* @note     ????(???):
*           1) PC2C ?? > ???? VOLT_THRESHOLD_PC2C
*           2) PC3C ?? > ???? VOLT_THRESHOLD_PC3C
*           3) PC3C ?? < ???? VOLT_THRESHOLD_PC3C_LOW
*           ??????????PWM??,??????
******************************************************************************/

#include "gd32h7xx.h"
#include "adc/bsp_adc.h"
#include "usart/bsp_usart_debug.h"
#include "systick/systick.h"
#include <stdio.h>
#include <stdint.h>
#include "main.h"
#include "dma.h"
#include "pwm.h"
#include "filament.h"
#include "IO_Agreement.h"


/* ======================== ?????? ======================== */
// PC2_C ??????
#define VOLT_THRESHOLD_PC2C       2.5f
// PC3_C ??????
#define VOLT_THRESHOLD_PC3C       2.5f
// PC3_C ??????(??:PC3C ??1V???PWM)
#define VOLT_THRESHOLD_PC3C_LOW   0.0f

// ADC???? 3.3V
#define REF_VOLTAGE               3.3f
// 12?ADC????? 2^12 - 1 = 4095
#define ADC_MAX                   4095U

/* ======================== PWM ????? ======================== */
// PWM??????:TIMER0
#define PWM_TIMER                 TIMER0
// PWM?????
#define PWM_TIMER_RCU             RCU_TIMER0

/* ======================== ???? ======================== */
// PC2_C(ADC2_IN0)?????
__IO uint16_t adc_val_pc2c = 0;
// PC3_C(ADC2_IN1)?????
__IO uint16_t adc_val_pc3c = 0;

// PWM?????:1=?? 0=??
static uint8_t pwm_enabled  = 1;
// ???????:1=???? 0=????
static uint8_t trip_latched = 0;

static uint16_t heartbeat_cnt = 0;


// ????????????
timer_free_complementary_parameter_struct timer_freecompara;

/* ======================== ???? ======================== */
static void cache_enable(void);
static void NVIC_PriorityGroupConfig(void);
void Soft_Delay(__IO uint32_t nCount);


static void AD7606_Hardware_Init(uint32_t sample_rate_hz);


/* ======================== ???? ======================== */
/**
 * @brief  ??CPU?????????,??????
 * @param  ?
 * @retval ?
 */
static void cache_enable(void)
{
    SCB_EnableICache();
    SCB_EnableDCache();
}

/**
 * @brief  ?????????:4??????,0?????
 * @param  ?
 * @retval ?
 */
static void NVIC_PriorityGroupConfig(void)
{
		// interrupt set
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
}

static void FPU_Enable(void)
{
    /* 开启 CP10 和 CP11 的完全访问权限 */
    SCB->CPACR |= (0xF << 20);

    /* 等待配置生效 */
    __DSB();
    __ISB();
}

/**
 * @brief  ????????
 * @param  nCount: ??????
 * @retval ?
 */
void Soft_Delay(__IO uint32_t nCount)
{
    for (; nCount != 0; nCount--);
}



void DAC_PA4_init(void)
{
		rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_DAC);
    gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_4);

    dac_deinit(DAC0);

    dac_trigger_disable(DAC0, DAC_OUT0);
    dac_enable(DAC0, DAC_OUT0);
		// 2.5V / 3.3V * 4095 = 3102
		dac_data_set(DAC0, DAC_OUT0, DAC_ALIGN_12B_R, 3102);
}

// 系统时钟初始化
void systick_init(void) {
    // 系统主频 600MHz，重装载值 = 600000000/1000 - 1 = 599999
    if (SysTick_Config(SystemCoreClock / 1000UL)) {
        // 配置失败，死循环
        while (1);
    }
    // SysTick 中断优先级设为最低（不影响其他中断）
    NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
}

void Handler_Capture_Init(void) 
{
    /* 1. 使能GPIOD端口的时钟 */
    rcu_periph_clock_enable(RCU_GPIOD);
		rcu_periph_clock_enable(RCU_GPIOA);

    /* 2. 配置PD11为输入模式，并使能内部上拉 */
    gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_11);
    // GPIO_MODE_INPUT: 设置为输入模式
    // GPIO_PUPD_PULLUP: 使能内部上拉电阻
		gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_7 | GPIO_PIN_8);
}




void Soft_Start(void)
{
		rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_GPIOJ);
		gpio_mode_set(GPIOJ, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_10);
		while(gpio_input_bit_get(GPIOJ, GPIO_PIN_10) == SET){};
			
		gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_12);
		gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_12);
		gpio_bit_set(GPIOD, GPIO_PIN_12);
			
		Soft_Delay(60000);
}




/* ======================== ??? ======================== */
int main(void)
{
    // PC2_C?PC3_C?????c
    static volatile float pt_inv_pc2c = 0.0f, volt_pc3c = 0.0f;
		Soft_Start();

    // enable D-Cache
    cache_enable();
	
    // interrupt priority set
    NVIC_PriorityGroupConfig();

    /* debug set */
    USART_Config();
	
		/* Enable FPU*/
		FPU_Enable();
		printf("Start Init\r\n");
		

#if 0
    
		PWM_TIMER0_Config();
			AD7606C_Init();

#endif
    //PWM_TIMER_Config();
		
		/* LED Init*/
		// 
#if 1
		PWM_GPIO_Config(); 
		PWM_TIMER0_Config();
		ADC_DMA_Start();
		XL74HC595_Init();
		Timer15_1s_Init();
		CD4013_Init();
		Fil_Init();
		
		Handler_Capture_Init();
		AD7606C_Init_Soft();
		Systick_Int();
#endif
		
		/* ERR_Protect Init*/

    // ??????
    printf("========== ADC2(IN0/IN1) + PWM Protect Start ==========\r\n");
    printf("Mapping: PC2_C=ADC2_IN0(ADC_CHANNEL_0), PC3_C=ADC2_IN1(ADC_CHANNEL_1)\r\n");
    printf("Threshold: PC2C_H=%.2fV, PC3C_H=%.2fV, PC3C_L=%.2fV\r\n",
           VOLT_THRESHOLD_PC2C, VOLT_THRESHOLD_PC3C, VOLT_THRESHOLD_PC3C_LOW);

		uint16_t ret = 3;
    while(usart_flag_get(DEBUG_USARTX, USART_FLAG_RBNE) == SET) {
        usart_data_receive(DEBUG_USARTX);
    }
    usart_interrupt_enable(DEBUG_USARTX, USART_INT_RBNE);
		
		
		
		uint32_t safe_cnt = 0;
		uint32_t shutdown_cnt = 0;
		uint64_t start_tick = SysTick_GetMs_Now();
		uint64_t now_tick = 0;
		uint8_t allow_shutdown_flag = 0x00;
		stage = 100;
    while (1)
    {
				#if 1
			// 手闸曝光检测
				if (safe_cnt == 0 && gpio_input_bit_get(GPIOD, GPIO_PIN_11) == RESET) 
				{
					// 防抖动
					Soft_Delay(1000);
					if (gpio_input_bit_get(GPIOD, GPIO_PIN_11) == RESET)
					{
						safe_cnt = 50000;
						// 50000合适
						cmd_ready = 1;
						rx_ctrl_byte = 0x01;
						printf("press button\n");
						if (stage == 100)
						{
							stage = 0;	
						}
						else
						{
							printf("PWM仍然在工作\n");
						}
					}
				}
				#endif
			
				
				now_tick = SysTick_GetMs_Now();
				if (allow_shutdown_flag!= 0x01 && now_tick - start_tick > 500) allow_shutdown_flag = 0x01;
				if (allow_shutdown_flag && gpio_input_bit_get(GPIOJ, GPIO_PIN_10) == RESET)	
				{
					Soft_Delay(10000);
					if (gpio_input_bit_get(GPIOJ, GPIO_PIN_10) == RESET)
					{
						gpio_bit_reset(GPIOD, GPIO_PIN_12);
					}
				}
			
			
				// 调用指令解析执行函数
        Command_Process();
				safe_cnt = safe_cnt > 0 ? safe_cnt-1 : 0;
        
        // PWM计时完成提示
        if(pwm_1s_finish)
        {
            pwm_1s_finish = 0;
            printf("[提示] 2秒计时结束，PWM已自动关闭\r\n");
        }

        pt_inv_pc2c = (float)ret * 3.3 / 4095.0;
        volt_pc3c = (float)adc_val_pc3c * REF_VOLTAGE / (float)ADC_MAX;


        if (!trip_latched &&
            (pt_inv_pc2c > VOLT_THRESHOLD_PC2C ||
             volt_pc3c > VOLT_THRESHOLD_PC3C ||
             volt_pc3c < VOLT_THRESHOLD_PC3C_LOW))
        {
            // ????????
            trip_latched = 1;

            // ???????????
            if (pt_inv_pc2c > VOLT_THRESHOLD_PC2C) {
                printf("TRIP! PC2C High: %.3fV > %.2fV\r\n", pt_inv_pc2c, VOLT_THRESHOLD_PC2C);
            } else if (volt_pc3c > VOLT_THRESHOLD_PC3C) {
                printf("TRIP! PC3C High: %.3fV > %.2fV\r\n", volt_pc3c, VOLT_THRESHOLD_PC3C);
            } else {
                printf("TRIP! PC3C Low : %.3fV < %.2fV\r\n", volt_pc3c, VOLT_THRESHOLD_PC3C_LOW);
            }

            // ??PWM????
            //PWM_Disable();
        }

        /* ????,????????? */
        Soft_Delay(10000);
    }
}




/************************ (C) COPYRIGHT *****END OF FILE****/


