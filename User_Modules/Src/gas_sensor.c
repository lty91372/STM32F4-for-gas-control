#include "gas_sensor.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "uart_transmit_moudle.h"
#define RECEIVE_INTERVAL 1000
#define TIMEOUT_LIMIT 500
#define SENSOR_STATE_DEBUG 0
#define SENSOR_DATA_DEBUG 1
#define SENSOR_QUERY_DEBUG 0

// 静态缓冲区，必须是全局或静态，防止 DMA 访问失效地址
static uint8_t query_frame[] = {GAS_SENSOR_ADDR, 0x03, 0x00, 0x00, 0x00, 0x0A, 0xC5, 0xCD};
GasSensor_Data_t sensor_node[3];
char sensor_debug_buf[256];

const char* SensorStatus_Names[] = {
    "IDLE",
    "WAIT_TX",
    "WAIT_RX",
    "COMPLETE",
    "ERROR"
};

/**
 * @brief  气体传感器组初始化函数
 * @note   该函数建立结构体与物理 UART 硬件的映射关系，并清空 DMA 接收缓冲区。
 * 应在 main 函数的 HAL_DMA_Init() 和 MX_USARTx_UART_Init() 之后调用。
 * @param  None
 * @retval None
 */
void GasSensor_Init(void) {
    sensor_node[0].huart = &huart1;
    sensor_node[1].huart = &huart2;
    sensor_node[2].huart = &huart3;
    for(int i=0; i<3; i++)
    {
    	sensor_node[i].status = SENSOR_IDLE;
    	memset(sensor_node[i].rx_temp_buf, 0, sizeof(sensor_node[i].rx_temp_buf));

    	sprintf(sensor_debug_buf,"Sensor,%d,has been constructed/n",i);
    	Uart_Write_Buff((uint8_t*)sensor_debug_buf,strlen(sensor_debug_buf));
    }

}

/**
 * @brief  启动指定通道的非阻塞 DMA 采样任务
 * @note   该函数通过 DMA 方式同时启动串口的发送与接收，CPU 调用后立即返回，不产生阻塞。
 * 采用“先开启接收 DMA，后开启发送 DMA”的策略，确保能完整捕捉传感器的起始响应字节。
 * @param  idx: 传感器节点索引 (0, 1, 2)
 * @retval None
 */
static void GasSensor_TriggerDMA(uint8_t idx) {
    sensor_node[idx].status = SENSOR_WAIT_TX;
    sensor_node[idx].start_tick = HAL_GetTick();
    // 1. 开启 DMA 接收准备
	HAL_UARTEx_ReceiveToIdle_DMA(sensor_node[idx].huart, sensor_node[idx].rx_temp_buf,50);
	// 2. 开启 DMA 发送
    HAL_UART_Transmit_DMA(sensor_node[idx].huart, query_frame, 8);
}

// 串口发送完成回调
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    for(int i=0; i<3; i++) {
        if(huart == sensor_node[i].huart) {

        	sensor_node[i].status = SENSOR_WAIT_RX;
#if SENSOR_QUERY_DEBUG
        	sprintf(sensor_debug_buf,"Sensor,%d,query transmit\n",i);
        	Uart_Write_Buff((uint8_t*)sensor_debug_buf,strlen(sensor_debug_buf));
#endif
#if SENSOR_STATE_DEBUG
        	int len = sprintf(sensor_debug_buf,"SS[%d],%s\n",i,SensorStatus_Names[sensor_node[i].status]);
        	Uart_Write_Buff((uint8_t*)sensor_debug_buf,len);
#endif

        }
    }
    if (huart->Instance == UART4)
    {
    	FIFO_Callback(&huart4);
    }

}

/**
 * @brief  UART 接收完成中断回调函数 (由 HAL 库在 DMA 接收满 25 字节时自动调用)
 * @note   该函数实现了“数据分流”与“协议解析”：
 * 1. 根据触发中断的 huart 句柄，确定是哪一路传感器回传了数据。
 * 2. 进行 Modbus 协议格式校验（地址与功能码）。
 * 3. 按照传感器规格书的寄存器定义，将原始字节解析为物理量（浓度、温湿度等）。
 * @param  huart: 指向触发中断的 UART 句柄指针
 * @retval None
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    for(int i=0; i<3; i++)
    {
        if(huart == sensor_node[i].huart)
        {
        	HAL_UART_DMAStop(huart);
            // 此时 rx_temp_buf 已填满，立即解析
            GasSensor_Data_t *d = &sensor_node[i];
            //Size代表回传接收到的字节数，达到25才开始解包
            if (Size == 25)
            {
            	 // 解析逻辑 (和你之前的解析代码一致)
				if (d->rx_temp_buf[0] == GAS_SENSOR_ADDR && d->rx_temp_buf[1] == 0x03)
				{
					uint8_t decimal_pos = d->rx_temp_buf[3] & 0x0F;
					uint16_t raw_conc = (d->rx_temp_buf[5] << 8) | d->rx_temp_buf[6];
					int16_t raw_temp = (int16_t)((d->rx_temp_buf[17] << 8) | d->rx_temp_buf[18]);
					uint16_t raw_humi = (uint16_t)((d->rx_temp_buf[21] << 8) | d->rx_temp_buf[22]);
					uint8_t unit = d->rx_temp_buf[3]>>4;
					uint16_t tem_ad = (d->rx_temp_buf[15] << 8) | (d->rx_temp_buf[16]);

					if (decimal_pos == 0x04)
					{      // 0100 -> 一位小数
					    d->concentration = raw_conc / 10.0f;
					}
					else if (decimal_pos == 0x08)
					{ // 1000 -> 两位小数
					    d->concentration = raw_conc / 100.0f;
					}
					else if (decimal_pos == 0x0C)
					{ // 1100 -> 三位小数
					    d->concentration = raw_conc / 1000.0f;
					}
					else {                         // 0000 -> 无小数
					    d->concentration = (float)raw_conc;
					}

					d->temperature = raw_temp / 10.0f;
					d->humidity = raw_humi / 10.0f;
					d->status = SENSOR_COMPLETE;
					d->temp_ad = tem_ad;
				}
				else
				{
					d->status = SENSOR_ERROR;
				}
            }
            else
            {
            	d->status = SENSOR_ERROR;
            }
            //清除缓冲区
            memset(d->rx_temp_buf, 0, sizeof(d->rx_temp_buf));

#if SENSOR_DATA_DEBUG
            if(huart == &huart1)
            Sensor_data_transmit(i);
#endif
        }
    }
}


// 调度器：放在 main 循环中
void GasSensor_Scheduler(void) {
	static uint32_t last_poll_time = 0;
	static uint32_t poll_interval = RECEIVE_INTERVAL;
	static uint32_t timeout_limit = TIMEOUT_LIMIT;

	uint32_t current_tick = HAL_GetTick();

	if(current_tick - last_poll_time >= poll_interval)
	{
		last_poll_time = current_tick;

		for(int i = 0;i < 3;i++)
		{
			if(sensor_node[i].status == SENSOR_IDLE ||
			   sensor_node[i].status == SENSOR_COMPLETE ||
			   sensor_node[i].status == SENSOR_ERROR)
			{
#if SENSOR_STATE_DEBUG
				if(sensor_node[i].huart == &huart1)
				{
					SensorStatus_t current_status = sensor_node[i].status;
					sprintf(sensor_debug_buf, "Tstart,Sensor %d State: %s\r\n", i,SensorStatus_Names[current_status]);
					Uart_Write_Buff((uint8_t*)sensor_debug_buf, strlen(sensor_debug_buf));
				}
#endif
				//清除错误标志位
				__HAL_UART_CLEAR_OREFLAG(sensor_node[i].huart);
				GasSensor_TriggerDMA(i);
			}

		}
	}
	for (int i = 0; i < 3; i++) {
	        if (sensor_node[i].status == SENSOR_WAIT_TX || sensor_node[i].status == SENSOR_WAIT_RX) {
	            if (current_tick - sensor_node[i].start_tick > timeout_limit) {

	                // 强制停止当前路 DMA，防止占用总线
	                HAL_UART_DMAStop(sensor_node[i].huart);

	                // 更新状态为错误，以便下一轮 Scheduler 能重新尝试启动
	                sensor_node[i].status = SENSOR_ERROR;

	                // (可选) 在这里清除缓冲区或记录日志
	            }
	        }
	}

}

float Get_Sensor_Concentration(uint8_t id)
{
	return sensor_node[id].concentration;
}

float Get_Sensor_Temperature(uint8_t id)
{
	return sensor_node[id].temperature;
}

float Get_Sensor_Humidity(uint8_t id)
{
	return sensor_node[id].humidity;
}

void Sensor_data_transmit(uint8_t id)
{
	static uint16_t transmit_inc = 0;
	transmit_inc++;
	float concentration = Get_Sensor_Concentration(id);
	float temperature = Get_Sensor_Temperature(id);
	float humidity = Get_Sensor_Humidity(id);

	// 2. 准备发送缓冲区 (长度建议 64 以上，防止浮点数展开后溢出)
	char transmit_buff[80];



	// 4. 写入串口环形缓冲区并触发 DMA 发送
	if (transmit_inc >= 4)
	{// 3. 格式化字符串
		// 格式示例: [ID:0] Conc:12.50ppm, Temp:25.3C, Humi:50.2%
		int len = sprintf(transmit_buff,
				  "[ID:%d] Conc:%.2f, Temp:%.1f, Temp_ad:%hu,Humi:%.1f\r\n",
				  id, concentration, temperature, sensor_node[id].temp_ad,humidity);
		Uart_Write_Buff((uint8_t*)transmit_buff, (uint16_t)len);
		transmit_inc = 0;
	}
}

