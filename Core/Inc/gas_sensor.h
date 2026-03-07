/*
 * gas_sensor.h
 *
 *  Created on: Mar 7, 2026
 *      Author: jiang
 */

#ifndef __GAS_SENSOR_H
#define __GAS_SENSOR_H

#include "main.h"

// 传感器 Modbus 默认地址
#define GAS_SENSOR_ADDR 0x01

// 气体类型定义 (根据规格书附录)
#define GAS_TYPE_O2     67   // 氧气 [cite: 158]

// 传感器返回数据结构体
typedef struct {
    float concentration;    // 浓度值
    float temperature;      // 环境温度 [cite: 140]
    float humidity;         // 环境湿度 [cite: 144]
    uint16_t full_range;    // 量程 [cite: 140]
    uint8_t state;          // 工作状态 [cite: 151]
    uint8_t unit_type;      // 单位类型 (0:ppm, 2:%LEL, 4:%VOL等) [cite: 147]
} GasSensor_Data_t;

// 函数声明
HAL_StatusTypeDef GasSensor_ReadData(UART_HandleTypeDef *huart, GasSensor_Data_t *data);

#endif
