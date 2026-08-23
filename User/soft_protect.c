#include "soft_protect.h"
#include "pwm.h"


/************************ 全局故障变量定义（仅在此处分配内存并赋初值） ************************/
uint8_t akv_err = ERR_NORMAL;
uint8_t ckv_err = ERR_NORMAL;
uint8_t ama_err = ERR_NORMAL;
uint8_t cma_err = ERR_NORMAL;
uint8_t arc_err = ERR_NORMAL;
uint8_t fil_err = ERR_NORMAL;
uint8_t inv_err = ERR_NORMAL;


/**
 * @brief  四路故障检测初始化，代码结构与调用逻辑完全对标AD7606C_Init
 * @note   配置顺序：GPIO时钟 → GPIO输入模式 → SYSCFG时钟 → EXTI引脚映射 → EXTI中断 → NVIC
 */
void fault_protect_init(void)
{
    /**************** 1. 开启外设时钟 ****************/
    rcu_periph_clock_enable(ERR_GPIO_CLK);     // 开启GPIOE端口时钟
		rcu_periph_clock_enable(RCU_GPIOF);
    rcu_periph_clock_enable(RCU_SYSCFG);       // 开启SYSCFG时钟，EXTI引脚映射必须

    /**************** 2. 配置GPIO为下拉输入 ****************/
    /* CD4013常态输出低，下拉避免浮空干扰误触发；无需上下拉可改为GPIO_PUPD_NONE */
    gpio_mode_set(ERR_GPIO_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, ALL_ERR_PINS);
		gpio_mode_set(GPIOF, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_6|GPIO_PIN_7);


    /**************** 3. EXTI引脚端口映射（绑定PE引脚到对应EXTI线） ****************/
    syscfg_exti_line_config(ERR_EXTI_PORT_SRC, AKV_EXTI_PIN_SRC);
    syscfg_exti_line_config(ERR_EXTI_PORT_SRC, CKV_EXTI_PIN_SRC);
    syscfg_exti_line_config(ERR_EXTI_PORT_SRC, ARC_EXTI_PIN_SRC);
    syscfg_exti_line_config(ERR_EXTI_PORT_SRC, FIL_EXTI_PIN_SRC);
		syscfg_exti_line_config(EXTI_SOURCE_GPIOF, EXTI_SOURCE_PIN6);
		syscfg_exti_line_config(EXTI_SOURCE_GPIOF, EXTI_SOURCE_PIN7);

    /**************** 4. 配置EXTI中断：上升沿触发、中断模式 ****************/
    /* AKV_ERR 中断配置 */
    exti_init(AKV_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(AKV_EXTI_LINE);
    exti_interrupt_enable(AKV_EXTI_LINE);

    /* CKV_ERR 中断配置 */
    exti_init(CKV_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(CKV_EXTI_LINE);
    exti_interrupt_enable(CKV_EXTI_LINE);

    /* ARC_ERR 中断配置 */
    exti_init(ARC_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(ARC_EXTI_LINE);
    exti_interrupt_enable(ARC_EXTI_LINE);

    /* FIL_ERR 中断配置 */
    exti_init(FIL_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(FIL_EXTI_LINE);
    exti_interrupt_enable(FIL_EXTI_LINE);
		
		/* FIL_ERR 中断配置 */
    exti_init(EXTI_6, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(EXTI_6);
    exti_interrupt_enable(EXTI_6);
		
		/* FIL_ERR 中断配置 */
    exti_init(EXTI_7, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(EXTI_7);
    exti_interrupt_enable(EXTI_7);
		
		
		

    /**************** 5. NVIC中断优先级配置（与AD7606的nvic_irq_enable写法完全一致） ****************/
    /* EXTI3（AKV）：抢占优先级1，子优先级0，故障保护优先级高于普通业务 */
    nvic_irq_enable(AKV_EXTI_IRQn, 1, 0);

    /* EXTI4（CKV）：抢占优先级1，子优先级1 */
    nvic_irq_enable(CKV_EXTI_IRQn, 1, 1);

    /* EXTI5_9（ARC/FIL + AD7606）：抢占优先级1，子优先级2 */
    /* 注意：该中断已被AD7606使用，重复调用nvic_irq_enable不影响，优先级以最后配置为准 */
    nvic_irq_enable(ARC_FIL_EXTI_IRQn, 1, 2);
		
		
}
/**
 * @brief  清除所有故障标志位，故障排除后调用
 */
void fault_flag_clear(void)
{
    akv_err = ERR_NORMAL;
    ckv_err = ERR_NORMAL;
		ama_err = ERR_NORMAL;
		cma_err = ERR_NORMAL;
    arc_err = ERR_NORMAL;
    fil_err = ERR_NORMAL;
		inv_err = ERR_NORMAL;
}

/************************ 独立中断服务函数（EXTI3、EXTI4） ************************/
/**
 * @brief  EXTI3中断服务函数：PE3(AKV_ERR)上升沿触发
 */
void EXTI3_IRQHandler(void)
{
    if(SET == exti_interrupt_flag_get(AKV_EXTI_LINE))
    {
        akv_err = ERR_FAULT;           // 置故障标志
        pwm_emergency_stop();       // 紧急关PWM
        exti_interrupt_flag_clear(AKV_EXTI_LINE); // 清除中断标志
    }
}

/**
 * @brief  EXTI4中断服务函数：PE4(CKV_ERR)上升沿触发
 */
void EXTI4_IRQHandler(void)
{
    if(SET == exti_interrupt_flag_get(CKV_EXTI_LINE))
    {
        ckv_err = ERR_FAULT;
        pwm_emergency_stop(); 
        exti_interrupt_flag_clear(CKV_EXTI_LINE);
    }
}
/********** 新增：EXTI5_9中断服务（处理ARC/FIL故障） **********/

#if 0
void EXTI5_9_IRQHandler(void)
{
    /* ARC故障处理 */
    if(SET == exti_interrupt_flag_get(ARC_EXTI_LINE))
    {
        arc_err = ERR_FAULT;
        pwm_emergency_stop();
        exti_interrupt_flag_clear(ARC_EXTI_LINE);
    }
    /* FIL故障处理 */
    if(SET == exti_interrupt_flag_get(FIL_EXTI_LINE))
    {
        fil_err = ERR_FAULT;
        pwm_emergency_stop();
        exti_interrupt_flag_clear(FIL_EXTI_LINE);
    }
}
#endif
