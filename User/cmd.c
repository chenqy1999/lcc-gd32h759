#include <string.h>
#include <stdlib.h>
#include "pwm.h"
#include "gd32h7xx_timer.h"




// 定义命令回调函数指针
typedef void (*cmd_handler_t)(int argc, char *argv[]);

// 命令条目结构体
typedef struct {
    const char *cmd_name;
    cmd_handler_t handler;
} cmd_item_t;

// 具体的业务处理函数
void cmd_led_handler(int argc, char *argv[]) {
    if (argc < 2) return;
    int state = atoi(argv[1]); // 提取参数
    // control_led(state);
}


// 具体的业务处理函数
void pwm_handler(int argc, char *argv[]) {
    if (argc < 2) return;
    int times = atoi(argv[1]); // 提取开通时长, ms为单位
	  int fre = atoi(argv[2]); // 提取频率
	
	
		uint32_t repecnt = times;
		timer_interrupt_enable(TIMER0, TIMER_INT_UP);
		TIMER_CREP0(TIMER0) = repecnt - 1;
		timer_enable(TIMER0);
    // control_led(state);
}




// 注册命令表
cmd_item_t cmd_table[] = {
    {"LED", cmd_led_handler},
    // 在这里可以继续添加其他命令
		{"PWM", pwm_handler}
};

// 核心解析器：将字符串按类似 main(argc, argv) 的方式分割并匹配
void cmd_parser_execute(char *cmd_line) 
{
    char *argv[10];
    int argc = 0;
    
    // 1. 使用 strtok 切割字符串，存入 argv 数组
    char *token = strtok(cmd_line, " ");
    while (token != NULL && argc < 10) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    
    if (argc == 0) return;
    
    // 2. 遍历命令表进行匹配
    for (int i = 0; i < sizeof(cmd_table)/sizeof(cmd_item_t); i++) {
        if (strcmp(argv[0], cmd_table[i].cmd_name) == 0) {
            cmd_table[i].handler(argc, argv); // 匹配成功，调用函数
            return;
        }
    }
}