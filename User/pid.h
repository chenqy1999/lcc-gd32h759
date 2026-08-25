#ifndef __PID_H
#define __PID_H

#include <stdlib.h>

#include "stdint.h"

#define KP 0.05f
#define KI 0.0f
#define KD 0.0f
#define VOREF 2.0e5f
#define MAX	10.0f
#define MIN	-10.0f




// PID 参数与状态结构体
typedef struct {
    float Kp;           // 比例系数
    float Ki;           // 积分系数
    float Kd;           // 微分系数
    
    float target_val;   // 目标设定值
    float err_curr;     // 当前偏差 e(k)
    float err_last;     // 上一次偏差 e(k-1)
    float integral;     // 积分累加值
    
    float out_max;      // 输出上限（对应最大 PWM 占空比，如 10000）
    float out_min;      // 输出下限（通常为 0 或者是负最大值）
} PID_TypeDef;


volatile typedef struct {
    float Kp;           // 比例系数
    float Ki;           // 积分系数
    float Kd;           // 微分系数
    
    float target_val;   // 目标设定值
    float errk;     		// 当前偏差 e(k)
    float errk_1;     // 上一次偏差 e(k-1)
		float errk_2;     
    float integral;     // 积分累加值
	
		float delta_u;
		float uk;
		float uk_1;
		
    float out_max;      // 输出上限（对应最大 PWM 占空比，如 10000）
    float out_min;      // 输出下限（通常为 0 或者是负最大值）
} PID_Incre ;

extern PID_Incre pwmPID, filPID;

extern float coeff;

extern uint32_t freq_cnt;
extern uint32_t freq_tbl[21000];

void PIDIncre_Init(PID_Incre *pid, float kp, float ki, float kd, float target_val, float min, float max);

// 增量式PID
float PID_Update(PID_Incre *pid, float feedback_val);



#endif