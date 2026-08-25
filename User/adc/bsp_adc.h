#ifndef __BSP_ADC_H
#define	__BSP_ADC_H


#include "gd32h7xx.h"


/*  注意：用作ADC采集的IO必须没有复用，否则采集电压会有影响
 *  ADC 编号选择
 *  根据官方数据手册可知，内部温度传感器与ADC2_CH18相连
 */ 
#define    ADC_APBXCLKCMD_FUN            rcu_periph_clock_enable
#define    ADCX                          ADC2
#define    ADC_CLK                       RCU_ADC2

/* ADC GPIO宏定义 */ 
#define    ADC_GPIO_APBXCLOCK_FUN        rcu_periph_clock_enable
#define    ADC_GPIO_CLK                  RCU_GPIOC  
#define    ADC_PORT                      GPIOC
#define    ADC_PIN                       GPIO_PIN_2

/* ADC 通道宏定义 */ 
#define    ADC_CHANNEL                   ADC_CHANNEL_0
#define 	 ADC_INT2FLOAT								 0.00030517578f	//  10 / 32768
#define 	 ADC_INT2FLOAT_5V								 (1.0899467669e-4f*1.5) // 5 / 2^15-1 / 1.4


// 灯丝RMS的ADC周期数
#define RMS_N 20
#define INV_128 1 / 128.f

// ADC7606C buffer
extern volatile int16_t adc_data[8];

// ADC7606C co
extern float k[8];
extern uint32_t adc_cnts[500];
extern  uint32_t adc_cnt;
extern  uint32_t ptr;
extern float KVs[500];
extern float FILs[500];
extern uint8_t rms_Window_Index;
extern float rms_Window[128];
extern float rms_Window_Sum;

/* on chip ADC0 setup */
void ADCX_Init(void);
uint16_t ADC_Read_Channel(uint8_t channel);

/* AD7606C Setup */
void AD7606C_Init(void);
void AD7606C_ReadChannels(void);

void ReadData(void);
void ospim_cfg_init(void);
void Ospi0GpioConfig(void);
void OspiStartEeadTransfer(void);
void BusyTriggerDmaConfig(void);
void OspiStartEeadTransfer(void);
void OspiAd7606CInit(void);
void AD7606C_Init_Soft(void);
void Start_ADC(void);
void Stop_ADC(void);
void ADC_Result(void);


void RMS_Avg(float FilCurrent);


#endif /* __BSP_ADC_H */

