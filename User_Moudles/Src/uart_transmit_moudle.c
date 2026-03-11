/*
 * uart_transmit_moudle.c
 *
 *  Created on: Oct 26, 2025
 *      Author: HP
 */

//UART传输状态,配置在定时器中
#include "main.h"
#include "adc_moudle.h"
#include "string.h"
#include "uart_transmit_moudle.h"
#include  <stdio.h>
#include "usart.h"
#include "matrix_keyboard.h"
#define UART_RE 0


volatile uint8_t uart_trigger = 0;
volatile uint32_t uartCounter = 0;
volatile uint8_t uart_busy = 0;
uint8_t ReceiveData[50];
RingBuffer uart_tx_fifo = {{0}, 0, 0};

//测试用例//
char message[] = "Hello World!";
uint16_t test[3] = {0x0012,0x0025,0x00E1};
char info[50] ="";

//UART触发状态机，用于中断服务函数，定时器配置为TIM14，每隔2秒回触发一次UART数据传输
void UART_STATE_MACHINE()
{
	if(uart_busy != 1 && !uart_trigger)
	{
		uart_trigger = 1;
		uartCounter = 0;

	}
}

//UART传输控制，非耗时操作，放在主循环中，回传有关ADC气体传感器的数据
void UART_Transmit_Control()
{
	if(uart_trigger == 1 && uart_busy == 0)
	{
		uart_busy = 1;
		sprintf(info,"%d,%d,%d,%d,%d,%d\r\n",Get_adc_After_Filter(0),Get_adc_After_Filter(1),Get_adc_After_Filter(2),Get_adc_Raw(0),Get_adc_Raw(1),Get_adc_Raw(2));
		HAL_UART_Transmit_DMA(&huart4,(uint8_t*)info,strlen(info));
		uart_trigger = 0;
		uart_busy = 0;
	}
}


//矩阵键盘输入数据回传
void KeyBoard_Transmit()
{
	if(debug_flag)
	{
		HAL_UART_Transmit_DMA(&huart4,(uint8_t*)&(debug_data),1);
		debug_flag = 0;
	}
}

//接收数据初始化
void Receive_Init()
{
	HAL_UARTEx_ReceiveToIdle_DMA(&huart4, ReceiveData, sizeof(ReceiveData));
}

#if UART_RE
//自接收回传测试例程
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart,uint16_t Size)
{
	if(huart == &huart4)
		HAL_UART_Transmit_DMA(&huart4,ReceiveData,Size);
		HAL_UARTEx_ReceiveToIdle_DMA(&huart4, ReceiveData, sizeof(ReceiveData));
}
#endif

/**
 * @brief  将数据写入 UART 发送环形缓冲区 (FIFO)
 * @param  data: 待发送数据的起始地址
 * @param  len:  待发送数据的长度
 * @note   逻辑说明：
 * 1. 采用循环覆盖检测，如果缓冲区已满（next_head == tail），则丢弃当前字节。
 * 2. 仅负责向内存缓冲区“生产”数据，不触发实际的硬件发送。
 * 3. 线程安全提示：若在中断中使用，需注意对 head 指针的操作保护。
 */
void Uart_Write_Buff(const uint8_t *data,uint16_t len)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq(); // 暂时关中断，保护指针操作

	for(uint16_t i = 0;i < len;i++)
	{
		uint16_t next_head = (uart_tx_fifo.head + 1) % TRANSMIT_BUFF;
		if(next_head != uart_tx_fifo.tail)
		{
			uart_tx_fifo.buffer[uart_tx_fifo.head] = data[i];
			uart_tx_fifo.head = next_head;
		}
	}
	__set_PRIMASK(primask); // 恢复中断状态
	Start_Uart_DMA_Transmit_From_FIFO();
}


void Start_Uart_DMA_Transmit_From_FIFO()
{
	if(huart4.gState == HAL_UART_STATE_READY && uart_tx_fifo.head != uart_tx_fifo.tail)
	{
		uint16_t send_len;

		if(uart_tx_fifo.head > uart_tx_fifo.tail)
		{
			send_len = uart_tx_fifo.head - uart_tx_fifo.tail;
		}else{
			send_len = TRANSMIT_BUFF - uart_tx_fifo.tail;
		}
		HAL_UART_Transmit_DMA(&huart4, (uint8_t*)&uart_tx_fifo.buffer[uart_tx_fifo.tail], send_len);
	}
}

void FIFO_Callback(UART_HandleTypeDef *huart)
{
	// 更新读指针：加上刚刚发送出去的长度
	uint16_t sent_len = huart->TxXferSize;
	uart_tx_fifo.tail = (uart_tx_fifo.tail + sent_len) % TRANSMIT_BUFF;

	// 继续尝试发送剩余部分
	Start_Uart_DMA_Transmit_From_FIFO();
}

//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if (huart->Instance == UART4) {
//        // 更新读指针：加上刚刚发送出去的长度
//        uint16_t sent_len = huart->TxXferSize;
//        uart_tx_fifo.tail = (uart_tx_fifo.tail + sent_len) % TRANSMIT_BUFF;
//
//        // 继续尝试发送剩余部分
//        Start_Uart_DMA_Transmit_From_FIFO();
//    }
//}


