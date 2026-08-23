#ifndef __ERR_FAULT_PROTECT_H
#define __ERR_FAULT_PROTECT_H

#include "gd32h7xx.h"

/************************ 硬件引脚与外设宏（全部在头文件统一定义） ************************/
/* 故障输入端口与时钟 */
#define ERR_GPIO_PORT       GPIOE
#define ERR_GPIO_CLK        RCU_GPIOE

/* 四路故障引脚定义 */
#define AKV_ERR_PIN         GPIO_PIN_3    // PE3 - AKV故障
#define CKV_ERR_PIN         GPIO_PIN_4    // PE4 - CKV故障
#define ARC_ERR_PIN         GPIO_PIN_5    // PE5 - ARC故障
#define FIL_ERR_PIN         GPIO_PIN_6    // PE6 - FIL故障
/* 批量配置用的引脚合集 */
#define ALL_ERR_PINS        (AKV_ERR_PIN | CKV_ERR_PIN | ARC_ERR_PIN | FIL_ERR_PIN)

/* EXTI中断线（与工程AD7606的EXTI_7命名规则完全一致） */
#define AKV_EXTI_LINE       EXTI_3
#define CKV_EXTI_LINE       EXTI_4
#define ARC_EXTI_LINE       EXTI_5
#define FIL_EXTI_LINE       EXTI_6

/* EXTI引脚映射源 */
#define ERR_EXTI_PORT_SRC   EXTI_SOURCE_GPIOE
#define AKV_EXTI_PIN_SRC    EXTI_SOURCE_PIN3
#define CKV_EXTI_PIN_SRC    EXTI_SOURCE_PIN4
#define ARC_EXTI_PIN_SRC    EXTI_SOURCE_PIN5
#define FIL_EXTI_PIN_SRC    EXTI_SOURCE_PIN6

/* 中断向量号 */
#define AKV_EXTI_IRQn       EXTI3_IRQn
#define CKV_EXTI_IRQn       EXTI4_IRQn
#define ARC_FIL_EXTI_IRQn   EXTI5_9_IRQn  // 与AD7606的EXTI7共用该中断向量

/* 故障状态码 */
#define ERR_NORMAL          0U
#define ERR_FAULT           1U

/************************ 全局故障变量声明（extern仅声明，不分配内存） ************************/
extern uint8_t akv_err;
extern uint8_t ckv_err;
extern uint8_t arc_err;
extern uint8_t fil_err;

/************************ 函数接口声明 ************************/
void fault_protect_init(void);
//void pwm_emergency_shutdown(void);
void fault_flag_clear(void);

#endif /* __ERR_FAULT_PROTECT_H */