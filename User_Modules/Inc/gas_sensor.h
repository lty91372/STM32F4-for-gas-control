#ifndef __GAS_SENSOR_H
#define __GAS_SENSOR_H

#include "main.h"

#define GAS_SENSOR_ADDR 0x01

// 传感器状态枚举
typedef enum {
    SENSOR_IDLE,        // 空闲
    SENSOR_WAIT_TX,     // 等待发送完成
    SENSOR_WAIT_RX,     // 等待接收完成
    SENSOR_COMPLETE,    // 一次采集完成
    SENSOR_ERROR        // 采集出错/超时
} SensorStatus_t;



typedef struct {
    float concentration;
    float temperature;
    float humidity;

    uint16_t full_range;
    uint8_t state;
    uint8_t unit_type;
    uint8_t rx_temp_buf[25];

    uint16_t temp_ad;

    // 控制成员
    UART_HandleTypeDef *huart; // 该传感器对应的串口
    SensorStatus_t status;     // 当前状态
    uint32_t start_tick;       // 用于软件超时检测

} GasSensor_Data_t;

// 外部全局变量
extern GasSensor_Data_t sensor_node[3];

// 函数声明
void GasSensor_Init(void);
void GasSensor_StartNext(void); // 启动下一轮异步采集
void GasSensor_Scheduler(void); // 放在 while(1) 中调度
float Get_Sensor_Concentration(uint8_t id);
float Get_Sensor_Temperature(uint8_t id);
float Get_Sensor_Humidity(uint8_t id);
void Sensor_data_transmit(uint8_t id);
#endif
