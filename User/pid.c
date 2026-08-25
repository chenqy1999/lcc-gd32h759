#include <stdlib.h>
#include <math.h>
#include "pid.h"


PID_Incre pwmPID, filPID;

float coeff = -1.0f;

uint32_t freq_cnt = 0;



/*******增量式 PID**********/
uint32_t freq_tbl[21000] = {0};
void PIDIncre_Init(PID_Incre *pid, float kp, float ki, float kd, float target_val, float max, float min) 
{
    *pid = (PID_Incre){0};
		pid->Kp = kp;
		pid->Ki = ki;
		pid->Kd = kd;
		pid->errk_2 = pid->errk_1 = pid->errk =  pid->target_val = target_val;
		pid->out_max = max;
		pid->out_min = min;
}


// 增量式PID
float PID_Update(PID_Incre *pid, float feedback_val) {
	
		if (pid == NULL)	return 0.0f;
    // 1. 计算当前偏差 e(k)
    // 计算当前误差
    pid->errk = pid->target_val - feedback_val;
		if (fabs(pid->errk) < 500.f)	return 0.0f;


    // 增量式PID
    pid->delta_u = pid->Kp * (pid->errk - pid->errk_1) + pid->Ki * pid->errk
        
        +pid->Kd * (pid->errk - 2.0f * pid->errk_1 + pid->errk_2);
    // 当前输出
    // 保存历史数据
    pid->errk_2 = pid->errk_1;
    pid->errk_1 = pid->errk;

    // 输出限幅
    if(pid->delta_u > pid->out_max)
    {
        pid->delta_u = pid->out_max;
    }
    if(pid->delta_u < pid->out_min)
    {
        pid->delta_u = pid->out_min;
    }
    return pid->delta_u;
}





