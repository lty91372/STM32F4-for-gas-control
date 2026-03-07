/*
 * debug_utils.c
 *
 *  Created on: Mar 4, 2026
 *      Author: HP
 */


#include "debug_utils.h"
#include "matrix_keyboard.h"
#include "adc_moudle.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"

// 初始化默认开关状态
Debug_Config_t DebugConfig = {
    .keyboard_log = 1,    // 默认开启键盘调试
};

// 内部私有缓冲区，避免污染全局变量
static char debug_tx_buff[128];

void Debug_Init(void) {
    LOG_INFO("Debug Module Initialized.\r\n");
}

/**
 * @brief 调试任务处理函数
 * 放置在 main 的 while(1) 中
 */
void Debug_Loop_Process(void) {

    // --- 1. 键盘调试逻辑 ---
    if (DebugConfig.keyboard_log && debug_flag) {
        sprintf(debug_tx_buff, "\r\n[KBD] Key:%c | Buff:[%s] | Idx:%d\r\n",
                debug_data, Input_Buff, input_index);
        HAL_UART_Transmit_DMA(&huart4, (uint8_t*)debug_tx_buff, strlen(debug_tx_buff));
        debug_flag = 0; // 处理完后清除标志
    }
}
