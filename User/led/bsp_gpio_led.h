#ifndef __BSP_GPIO_LED_H
#define	__BSP_GPIO_LED_H

#include "gd32h7xx.h"

/* 定义LED连接的GPIO端口, 用户只需要修改下面的代码即可改变控制的LED引脚 */
// R-红色
#define LED1_GPIO_PORT    			GPIOD			            /* GPIO端口 */
#define LED1_GPIO_CLK_PORT 	    RCU_GPIOD						/* GPIO端口时钟 */
#define LED1_GPIO_PIN			GPIO_PIN_11	       				/* 连接到LED1的GPIO */

// G-绿色
#define LED2_GPIO_PORT    			GPIOF			            /* GPIO端口 */
#define LED2_GPIO_CLK_PORT 	    RCU_GPIOF						/* GPIO端口时钟 */
#define LED2_GPIO_PIN			GPIO_PIN_7			        	/* 连接到LED2的GPIO */

// B-蓝色
#define LED3_GPIO_PORT    			GPIOD			            /* GPIO端口 */
#define LED3_GPIO_CLK_PORT 	    RCU_GPIOD						/* GPIO端口时钟 */
#define LED3_GPIO_PIN			      GPIO_PIN_4			            /* 连接到LED3的GPIO */


/** the macro definition to trigger the led on or off 
  * 1 - off
  *0 - on
  */
#define ON  0
#define OFF 1

/* 使用标准的固件库控制IO*/
#define LED_1(A)	if (A)	\
					gpio_bit_set(LED1_GPIO_PORT,LED1_GPIO_PIN);\
					else		\
					gpio_bit_reset(LED1_GPIO_PORT,LED1_GPIO_PIN)

#define LED_2(A)	if (A)	\
					gpio_bit_set(LED2_GPIO_PORT,LED2_GPIO_PIN);\
					else		\
					gpio_bit_reset(LED2_GPIO_PORT,LED2_GPIO_PIN)

#define LED_3(A)	if (A)	\
					gpio_bit_set(LED3_GPIO_PORT,LED3_GPIO_PIN);\
					else		\
					gpio_bit_reset(LED3_GPIO_PORT,LED3_GPIO_PIN)


/* 直接操作寄存器的方法控制IO */
#define	DIGITALHI(P,I)		 {GPIO_BOP(P)		=I;}	 //输出为高电平		
#define DIGITALLO(P,I)		 {GPIO_BC(P)		=I;}	 //输出低电平
#define DIGITALTLGGLE(P,I)   {GPIO_OCTL(P)  ^=I;} //输出反转状态


/* 定义控制IO的宏 */
#define LED1_TOGGLE		 DIGITALTLGGLE(LED1_GPIO_PORT,LED1_GPIO_PIN)
#define LED1_OFF		   DIGITALHI(LED1_GPIO_PORT,LED1_GPIO_PIN)
#define LED1_ON			   DIGITALLO(LED1_GPIO_PORT,LED1_GPIO_PIN)

#define LED2_TOGGLE		 DIGITALTLGGLE(LED2_GPIO_PORT,LED2_GPIO_PIN)
#define LED2_OFF		   DIGITALHI(LED2_GPIO_PORT,LED2_GPIO_PIN)
#define LED2_ON			   DIGITALLO(LED2_GPIO_PORT,LED2_GPIO_PIN)

#define LED3_TOGGLE		 DIGITALTLGGLE(LED3_GPIO_PORT,LED3_GPIO_PIN)
#define LED3_OFF		   DIGITALHI(LED3_GPIO_PORT,LED3_GPIO_PIN)
#define LED3_ON			   DIGITALLO(LED3_GPIO_PORT,LED3_GPIO_PIN)

/* 基本混色，后面高级用法使用PWM可混出全彩颜色,且效果更好 */

//红
#define LED_RED  \
					LED1_ON;\
					LED2_OFF\
					LED3_OFF

//绿
#define LED_GREEN		\
					LED1_OFF;\
					LED2_ON\
					LED3_OFF

//蓝
#define LED_BLUE	\
					LED1_OFF;\
					LED2_OFF\
					LED3_ON

					
//黄(红+绿)					
#define LED_YELLOW	\
					LED1_ON;\
					LED2_ON\
					LED3_OFF
//紫(红+蓝)
#define LED_PURPLE	\
					LED1_ON;\
					LED2_OFF\
					LED3_ON

//青(绿+蓝)
#define LED_CYAN \
					LED1_OFF;\
					LED2_ON\
					LED3_ON
					
//白(红+绿+蓝)
#define LED_WHITE	\
					LED1_ON;\
					LED2_ON\
					LED3_ON
					
//黑(全部关闭)
#define LED_RGBOFF	\
					LED1_OFF;\
					LED2_OFF\
					LED3_OFF

void LED_GPIO_Config(void);

#endif /* __BSP_GPIO_LED_H */
