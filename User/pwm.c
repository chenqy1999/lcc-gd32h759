/******************************************************************************
 * @file    pwm.c
 * @brief   PWM 模块实现（两路互补PWM+死区+硬件刹车+软启动）
 ******************************************************************************/
#include "pwm.h"
#include "pid.h"
#include "adc/bsp_adc.h"
#include "usart/bsp_usart_debug.h"
#include "systick/systick.h"
#include <string.h>


#define UART_PWM_CTRL     0         // 等待第1字节：PWM控制字
#define UART_STATE_DATA     1       // 等待第2字节：待保存变量值



/* 全局变量 */
volatile uint32_t current_freq = START_FREQ;;
volatile uint8_t cmd_ready = 0;       // 完整指令就绪标志


uint8_t rx_ctrl_byte = 0;      // 第1字节：PWM控制字 (0x00=不启动, 0x01=启动2秒)
uint8_t rx_data_byte = 0;      // 第2字节：待赋值的变量数据
volatile uint8_t uart_ctrl_var = 0;   // 调试观测变量


volatile uint8_t pwm_1s_finish = 0;  // 计时完成标志位
volatile uint8_t pwm_running = 0;    // PWM运行状态：0空闲，1运行中
uint8_t saved_value = 0;              // 全局保存变量（指令赋值目标）
volatile uint8_t stage = 0;


uint8_t uart_rx_state = UART_PWM_CTRL;

uint8_t run_fil_status = 0;
uint8_t run_pwm_status = 0;



void Print_Hex_Bin(uint8_t val)
{
    printf("Hex: 0x%02X, Bin: ", val);
    // 从高位到低位逐位输出二进制
    for(int8_t i = 7; i >= 0; i--)
    {
        printf("%d", (val >> i) & 0x01);
    }
    printf("\r\n");
}

/**
 * @brief  配置PWM输出的所有GPIO
 */
void PWM_GPIO_Config(void)
{
    /* 开启所有用到的GPIO端口时钟 */
    rcu_periph_clock_enable(RCU_GPIOK);
    rcu_periph_clock_enable(RCU_GPIOJ);

    /* ---------- 第一对互补通道：CH0(PK2/APWM8 -> TIMER7_BRKIN0) + MCH0(PK1/APWM7 -> TIMER7_CH0) ---------- */
    gpio_mode_set(CH0_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, CH0_PIN);
    gpio_output_options_set(CH0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, CH0_PIN);
    gpio_af_set(CH0_PORT, CH0_AF, CH0_PIN);

    gpio_mode_set(MCH0_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, MCH0_PIN);
    gpio_output_options_set(MCH0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, MCH0_PIN);
    gpio_af_set(MCH0_PORT, MCH0_AF, MCH0_PIN);

    /* ---------- 第二对互补通道：MCH1(PK0/APWM6 -> TIMER0_MCH0) + CH1(PJ11/APWM5 -> TIMER0_CH1) ---------- */
    gpio_mode_set(CH1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, CH1_PIN);
    gpio_output_options_set(CH1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, CH1_PIN);
    gpio_af_set(CH1_PORT, CH1_AF, CH1_PIN);

    gpio_mode_set(MCH1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, MCH1_PIN);
    gpio_output_options_set(MCH1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, MCH1_PIN);
    gpio_af_set(MCH1_PORT, MCH1_AF, MCH1_PIN);

}

/**
 * @brief  初始化TIMER0为中央对齐模式，两路带死区互补PWM
 */
void PWM_TIMER0_Config(void)
{
    timer_parameter_struct       timer_initpara;
    timer_oc_parameter_struct    oc_initpara;
    timer_free_complementary_parameter_struct comp_initpara;

    /* 开启定时器时钟 */
    rcu_periph_clock_enable(PWM_TIMER0_RCU);
    timer_deinit(PWM_TIMER0);

    /* 计算周期和比较值（中央对齐模式：频率 = 定时器时钟 / (2*周期) */
    uint32_t period = TIMER_CLK_HZ / (2 * PWM_FREQ) - 1;
    uint32_t cmp_val = (period + 1) / 2;   // 50% 占空比

    /* ---------- 时基配置：中央对齐模式 ---------- */
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler         = 1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE; 
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = period;
    timer_initpara.clockdivision     = DEADTIME_CLK_DIV;
    timer_initpara.repetitioncounter = 0;
    timer_init(PWM_TIMER0, &timer_initpara);
		// timer_initpara.alignedmode       = TIMER_COUNTER_CENTER_DOWN

    /* ---------- 通道输出公共参数 ---------- */
    timer_channel_output_struct_para_init(&oc_initpara);
    oc_initpara.outputstate   = TIMER_CCX_ENABLE;    // 主通道输出使能
    oc_initpara.outputnstate  = TIMER_CCXN_ENABLE;   // 互补通道输出使能
    oc_initpara.ocpolarity    = TIMER_OC_POLARITY_HIGH;
    oc_initpara.ocnpolarity   = TIMER_OCN_POLARITY_HIGH;
    oc_initpara.ocidlestate   = TIMER_OC_IDLE_STATE_LOW;  // 空闲时低电平（安全态）
    oc_initpara.ocnidlestate  = TIMER_OCN_IDLE_STATE_LOW;

		
    /* ---------- CH0通道配置 ---------- */
    timer_channel_output_config(PWM_TIMER0, TIMER_CH_0, &oc_initpara);
    timer_channel_output_pulse_value_config(PWM_TIMER0, TIMER_CH_0, cmp_val);
    timer_channel_output_mode_config(PWM_TIMER0, TIMER_CH_0, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(PWM_TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_ENABLE);
		
    /* ---------- MCH0通道配置 ---------- */
    timer_channel_output_config(PWM_TIMER0, TIMER_MCH_0, &oc_initpara);
    timer_channel_output_pulse_value_config(PWM_TIMER0, TIMER_MCH_0, cmp_val);
    timer_channel_output_mode_config(PWM_TIMER0, TIMER_MCH_0, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(PWM_TIMER0, TIMER_MCH_0, TIMER_OC_SHADOW_ENABLE);

    /* ---------- CH1通道配置 ---------- */
    timer_channel_output_config(PWM_TIMER0, TIMER_CH_1, &oc_initpara);
    timer_channel_output_pulse_value_config(PWM_TIMER0, TIMER_CH_1, cmp_val);
    timer_channel_output_mode_config(PWM_TIMER0, TIMER_CH_1, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(PWM_TIMER0, TIMER_CH_1, TIMER_OC_SHADOW_ENABLE);

    /* ---------- 死区与互补模式配置（CH0和CH1分别配置） ----------  */
    timer_free_complementary_struct_para_init(&comp_initpara);
    comp_initpara.freecomstate = TIMER_FCCHP_STATE_ENABLE;
    comp_initpara.runoffstate  = TIMER_ROS_STATE_DISABLE;
    comp_initpara.ideloffstate = TIMER_IOS_STATE_DISABLE;
    comp_initpara.deadtime     = DEADTIME_CNT;
		
		timer_multi_mode_channel_mode_config(PWM_TIMER0, TIMER_CH_1, TIMER_MCH_MODE_COMPLEMENTARY);
    timer_multi_mode_channel_mode_config(PWM_TIMER0, TIMER_MCH_0, TIMER_MCH_MODE_COMPLEMENTARY);
		timer_multi_mode_channel_mode_config(PWM_TIMER0, TIMER_CH_0, TIMER_MCH_MODE_COMPLEMENTARY);

		
    timer_channel_free_complementary_config(PWM_TIMER0, TIMER_CH_0, &comp_initpara);
    timer_channel_free_complementary_config(PWM_TIMER0, TIMER_MCH_0, &comp_initpara);
    timer_channel_free_complementary_config(PWM_TIMER0, TIMER_CH_1, &comp_initpara);


    /* ---------- 使能主输出和自动重载影子，启动定时器 ---------- */
		timer_event_software_generate(PWM_TIMER0, TIMER_EVENT_SRC_UPG);
		timer_auto_reload_shadow_enable(TIMER0);

		
		#if 0
    timer_auto_reload_shadow_enable(PWM_TIMER0);
    timer_primary_output_config(PWM_TIMER0, ENABLE);
    timer_enable(PWM_TIMER0);
		#endif
}


/**
 * @brief  更新 PWM 频率
 */
void Update_PWM_Frequency(uint32_t new_freq)
{

    uint32_t period = TIMER_CLK_HZ / (2 * new_freq) - 1;
    uint32_t ch_cmp = (period + 1) >> 1;

		TIMER_CAR(TIMER0)=period;
		TIMER_CH0CV(TIMER0)=ch_cmp;
		TIMER_CH1CV(TIMER0)=ch_cmp;
}


float Get_Timer_Freq(void)
{
    uint32_t psc;
    uint32_t arr;

    psc = TIMER_PSC(TIMER0);
    arr = TIMER_CAR(TIMER0);

    return (float)TIMER_CLK_HZ / ((float)(psc + 1) * (float)(arr + 1));
}

float Get_Timer_Duty(uint32_t timer_periph)
{
		return (float) TIMER_CH0CV(timer_periph);
}

void Set_Timer_Duty(uint32_t timer_periph, uint32_t duty)
{
		TIMER_CH0CV(timer_periph) = duty;
}


/**
 * @brief  初始化TIMER7为中央对齐模式，两路带死区互补PWM
 */
void PWM_TIMER7_Config(void)
{
    timer_parameter_struct       timer_initpara;
    timer_oc_parameter_struct    oc_initpara;
    timer_free_complementary_parameter_struct comp_initpara;

    /* 开启定时器时钟 */
    rcu_periph_clock_enable(PWM_TIMER7_RCU);
    timer_deinit(PWM_TIMER7);

    /* 计算周期和比较值（中央对齐模式：频率 = 定时器时钟 / (2*周期) */
    uint32_t period = TIMER_CLK_HZ / (2 * PWM_FREQ2) - 1;
    uint32_t cmp_val = (period + 1) / 2;   // 50% 占空比

    /* ---------- 时基配置：中央对齐模式 ---------- */
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler         = 1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE; 
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = period;
    timer_initpara.clockdivision     = DEADTIME_CLK_DIV;
    timer_initpara.repetitioncounter = 0;
    timer_init(PWM_TIMER7, &timer_initpara);

    /* ---------- 通道输出公共参数 ---------- */
    timer_channel_output_struct_para_init(&oc_initpara);
    oc_initpara.outputstate   = TIMER_CCX_ENABLE;    // 主通道输出使能
    oc_initpara.outputnstate  = TIMER_CCXN_ENABLE;   // 互补通道输出使能
    oc_initpara.ocpolarity    = TIMER_OC_POLARITY_HIGH;
    oc_initpara.ocnpolarity   = TIMER_OCN_POLARITY_HIGH;
    oc_initpara.ocidlestate   = TIMER_OC_IDLE_STATE_LOW;  // 空闲时低电平（安全态）
    oc_initpara.ocnidlestate  = TIMER_OCN_IDLE_STATE_LOW;
		
    /* ---------- CH2通道配置 ---------- */
    timer_channel_output_config(PWM_TIMER7, TIMER_CH_2, &oc_initpara);
    timer_channel_output_pulse_value_config(PWM_TIMER7, TIMER_CH_2, cmp_val);
    timer_channel_output_mode_config(PWM_TIMER7, TIMER_CH_2, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(PWM_TIMER7, TIMER_CH_2, TIMER_OC_SHADOW_ENABLE);
		
    /* ---------- MCH2通道配置 ---------- */
    timer_channel_output_config(PWM_TIMER7, TIMER_MCH_2, &oc_initpara);
    timer_channel_output_pulse_value_config(PWM_TIMER7, TIMER_MCH_2, cmp_val);
    timer_channel_output_mode_config(PWM_TIMER7, TIMER_MCH_2, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(PWM_TIMER7, TIMER_MCH_2, TIMER_OC_SHADOW_ENABLE);

    /* ---------- BRKIN0通道配置 ---------- */
    timer_channel_output_config(PWM_TIMER7, TIMER_BRKIN0, &oc_initpara);
    timer_channel_output_pulse_value_config(PWM_TIMER7, TIMER_BRKIN0, cmp_val);
    timer_channel_output_mode_config(PWM_TIMER7, TIMER_BRKIN0, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(PWM_TIMER7, TIMER_BRKIN0, TIMER_OC_SHADOW_ENABLE);

    /* ---------- 死区与互补模式配置 ----------  */
    timer_free_complementary_struct_para_init(&comp_initpara);
    comp_initpara.freecomstate = TIMER_FCCHP_STATE_ENABLE;
    comp_initpara.runoffstate  = TIMER_ROS_STATE_DISABLE;
    comp_initpara.ideloffstate = TIMER_IOS_STATE_DISABLE;
    comp_initpara.deadtime     = DEADTIME_CNT;
		
    timer_channel_free_complementary_config(PWM_TIMER7, TIMER_CH_2, &comp_initpara);
    timer_channel_free_complementary_config(PWM_TIMER7, TIMER_MCH_2, &comp_initpara);
    timer_channel_free_complementary_config(PWM_TIMER7, TIMER_BRKIN0, &comp_initpara);
		timer_multi_mode_channel_mode_config(PWM_TIMER7, TIMER_MCH_2, TIMER_MCH_MODE_COMPLEMENTARY);
    timer_multi_mode_channel_mode_config(PWM_TIMER7, TIMER_BRKIN0, TIMER_MCH_MODE_COMPLEMENTARY);

    /* ---------- 使能主输出和自动重载影子，启动定时器 ---------- */
		
		
		timer_auto_reload_shadow_enable(PWM_TIMER7);
    timer_primary_output_config(PWM_TIMER7, ENABLE);
    timer_enable(PWM_TIMER7);
		
		timer_interrupt_disable(TIMER0, TIMER_INT_UP);
}

void PWM_Start(void)
{
    timer_enable(TIMER0);
    timer_primary_output_config(TIMER0, ENABLE);
		memset(adc_cnts, 0, sizeof(adc_cnts));
		memset(KVs, 0, sizeof(KVs));
		pwm_running = 1;
}

/**
 * @brief  停止 PWM 输出
 */
void PWM_Stop(void)
{
    timer_primary_output_config(TIMER0, DISABLE);
    timer_disable(TIMER0);
			
    pwm_running = 0;
}



void Command_Process(void)
{
    // 无完整指令则直接返回
    if(stage == 100)
    {
        return;
    }
    // 1. 打印完整接收的双字节指令
    //printf("==============================\r\n");
    //printf("接收指令: 0x%02X 0x%02X\r\n", rx_ctrl_byte, rx_data_byte);
    
    // 2. PWM控制逻辑 + 状态打印
		
		
		switch (stage)
		{
			case 0:
				PIDIncre_Init(
					&filPID,
					1.f,
					1.f,
					0.f,
					3.f,
					100.f,
					-100.f
				);
				PIDIncre_Init(
					&pwmPID,
					50.0f,			//kp
					10.0f,			//ki
					10.0f,			//kd
					30.0e3, 	//Voref
					5000.f,				//max
					-5000.f				//min
				);
				pwm_1s_finish = 0;
				stage = 0;
							// 将 TIMER15 的计数器值清零
				timer_counter_value_config(TIMER15, 0);
				timer_enable(TIMER15);
				// 以TIMER15为例，使用标准库函数
				printf("Before Trigger Stage %d， timer count:%d, System Mils%lld:\n", stage, TIMER_CNT(TIMER15), SysTick_GetMs_Now());
				timer_event_software_generate(TIMER15, TIMER_EVENT_SRC_UPG);
				// printf("PWM输出状态：已启动，2秒后自动关闭\r\n");
			break;
			
			case 101:
				printf("PWM已结束，输出ADCResult\n");
				//ADC_Result();
				stage = 100;
			break;
			
			default:
			break;
		}
}



void Timer15_1s_Init(void)
{
    rcu_periph_clock_enable(RCU_TIMER15);
    timer_deinit(TIMER15);
    
    // 300MHz / 30000 = 10kHz计数时钟，计数20000次 = 2秒
		timer_parameter_struct timer_initpara;
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler         = 30000 - 1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 10 - 1;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 100-1;
    timer_init(TIMER15, &timer_initpara);
    
    timer_interrupt_flag_clear(TIMER15, TIMER_INT_FLAG_UP);
    timer_interrupt_enable(TIMER15, TIMER_INT_FLAG_UP);
    nvic_irq_enable(TIMER15_IRQn, 1, 0);
}




/**
 * @brief  软启动定时器配置
 * @note   如需启用软启动，请在PWM启动前调用本函数
 */
static void SoftStart_Timer_Config(void)
{
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER1);
    timer_deinit(TIMER1);

    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler         = 30000 - 1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 100 - 1;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER1, &timer_initpara);

    timer_interrupt_enable(TIMER1, TIMER_INT_FLAG_UP);
    nvic_irq_enable(TIMER1_IRQn, 0, 0);
    timer_enable(TIMER1);
}


void pwm_emergency_stop(void)
{
    timer_primary_output_config(PWM_TIMER0, DISABLE);
    timer_primary_output_config(PWM_TIMER7, DISABLE);
}