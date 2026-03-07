/*
 * gas_sensor.c
 *
 *  Created on: Mar 7, 2026
 *      Author: jiang
 */

#include "gas_sensor.h"
#include <string.h>

/**
 * @brief 读取传感器所有寄存器数据 (0x00 - 0x09)
 * @param huart: 指向串口句柄的指针 (如 &huart2)
 * @param data: 指向存储解析结果的结构体
 * @return HAL_StatusTypeDef: 返回读取状态
 */



// gas_sensor.c 中的临时测试版本
HAL_StatusTypeDef GasSensor_ReadData(UART_HandleTypeDef *huart, GasSensor_Data_t *data) {
    // 模拟传感器返回 20.9% 浓度的逻辑
    data->concentration = 20.9f;
    data->state = 0x01; // 正常状态
    data->temperature = 25.5f;

    return HAL_OK; // 强制返回成功
}

GasSensor_Data_t sensor_data[3];

HAL_StatusTypeDef GasSensor_GetConcentration(char *out_buffer, uint16_t buffer_size) {
    char s[3][10]; // 存储三路临时字符串
    HAL_StatusTypeDef res[3];

    // 1. 依次读取三路数据
    res[0] = GasSensor_ReadData(&huart1, &sensor_data[0]);
    res[1] = GasSensor_ReadData(&huart2, &sensor_data[1]);
    res[2] = GasSensor_ReadData(&huart3, &sensor_data[2]);

    // 2. 转换数据：成功显示数字，失败显示 Err
    for (int i = 0; i < 3; i++) {
        if (res[i] == HAL_OK) {
            snprintf(s[i], sizeof(s[i]), "%.1f", sensor_data[i].concentration);
        } else {
            strncpy(s[i], "Err", sizeof(s[i]));
        }
    }

    // 3. 将结果填入传入的 tx_buffer
    snprintf(out_buffer, buffer_size, "O2_1:%s, O2_2:%s, O2_3:%s %%VOL\r\n", s[0], s[1], s[2]);

    // 只要有一路成功就返回 OK，也可以根据需求改为全部成功才返回 OK
    if (res[0] == HAL_OK || res[1] == HAL_OK || res[2] == HAL_OK) {
        return HAL_OK;
    }
    return HAL_ERROR;
}

/*
HAL_StatusTypeDef GasSensor_ReadData(UART_HandleTypeDef *huart, GasSensor_Data_t *data) {
    // 标准问询帧: [地址][功能码][起始地址H][起始地址L][寄存器数H][寄存器数L][CRC_L][CRC_H]
    // 读取从0x0000开始的10个寄存器 [cite: 108, 116, 140]
    uint8_t query_frame[] = {GAS_SENSOR_ADDR, 0x03, 0x00, 0x00, 0x00, 0x0A, 0xC5, 0xCD};
    uint8_t rx_buf[25] = {0}; // 响应帧长度 = 5 + 寄存器数*2 = 25 字节 [cite: 111, 122]

    // 发送指令
    if (HAL_UART_Transmit(huart, query_frame, 8, 100) != HAL_OK) {
        return HAL_ERROR;
    }

    // 接收响应
    if (HAL_UART_Receive(huart, rx_buf, 25, 1000) != HAL_OK) {
        return HAL_TIMEOUT;
    }

    // 基础校验：地址与功能码 [cite: 112, 119]
    if (rx_buf[0] != GAS_SENSOR_ADDR || rx_buf[1] != 0x03) {
        return HAL_ERROR;
    }

    // 1. 解析参数位 (寄存器0: rx_buf[3,4]) [cite: 140, 147]
    // Bit12-15 为单位, Bit8-11 为小数点位数
    data->unit_type = (rx_buf[3] >> 4) & 0x0F;
    uint8_t decimal_pos = rx_buf[3] & 0x0F;

    // 2. 解析当前浓度 (寄存器1: rx_buf[5,6]) [cite: 125, 140]
    uint16_t raw_conc = (rx_buf[5] << 8) | rx_buf[6];
    if (decimal_pos == 4) {       // 一位小数 [cite: 147]
        data->concentration = raw_conc / 10.0f;
    } else if (decimal_pos == 8) { // 两位小数 [cite: 147]
        data->concentration = raw_conc / 100.0f;
    } else {
        data->concentration = (float)raw_conc;
    }

    // 3. 解析量程 (寄存器4: rx_buf[11,12]) [cite: 130, 140]
    data->full_range = (rx_buf[11] << 8) | rx_buf[12];

    // 4. 解析工作状态 (寄存器5低8位: rx_buf[14]) [cite: 132, 140]
    data->state = rx_buf[14];

    // 5. 解析环境温度 (寄存器7: rx_buf[17,18]) [cite: 134, 140]
    // 假设返回值为原始AD转换后的物理值，通常需根据实际输出调整
    data->temperature = ((int16_t)((rx_buf[17] << 8) | rx_buf[18])) / 10.0f;

    // 6. 解析环境湿度 (寄存器9: rx_buf[21,22]) [cite: 137, 144]
    data->humidity = ((rx_buf[21] << 8) | rx_buf[22]) / 10.0f;

    return HAL_OK;
}
*/

