#ifndef __DEBUG_UTILS_H
#define __DEBUG_UTILS_H

#include "main.h"

// 调试开关结构体：通过变量控制，可以在程序运行时通过仿真器或指令动态更改
typedef struct {
    uint8_t keyboard_log;    // 键盘录入调试开关
} Debug_Config_t;

// 声明全局调试配置（在.c中定义）
extern Debug_Config_t DebugConfig;

// 函数声明
void Debug_Init(void);
void Debug_Loop_Process(void);

// 进阶：定义打印宏（可选，如果不习惯用结构体开关，可以用这个一键关闭）
#define DEBUG_ENABLE 1
#if DEBUG_ENABLE
  #define LOG_INFO(fmt, ...) printf("[INFO] " fmt, ##__VA_ARGS__)
#else
  #define LOG_INFO(fmt, ...)
#endif

#endif
