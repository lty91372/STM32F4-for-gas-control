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


volatile uint8_t uart_trigger = 0;
volatile uint32_t uartCounter = 0;
volatile uint8_t uart_busy = 0;
uint8_t ReceiveData[50];
RingBuffer uart_tx_fifo = {0, 0, 0};

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

//自接收回传测试例程
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart,uint16_t Size)
{
	if(huart == &huart4)
		HAL_UART_Transmit_DMA(&huart4,ReceiveData,Size);
		HAL_UARTEx_ReceiveToIdle_DMA(&huart4, ReceiveData, sizeof(ReceiveData));
}

/*
 * @brief uart写缓冲区，要发的数据放在这里
 * @param，data：目标缓冲数组
 * @param len：数据长度
 */

void Uart_Write_Buff(const uint8_t *data,uint16_t len)
{
	for(uint16_t i = 0;i < len;i++)
	{
		uint16_t next_head = (uart_tx_fifo.head + 1) % TRANSMIT_BUFF;
		if(next_head != uart_tx_fifo.tail)
		{
			uart_tx_fifo.buffer[uart_tx_fifo.head] = data[i];
			uart_tx_fifo.head = next_head;
		}
	}
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

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4) {
        // 更新读指针：加上刚刚发送出去的长度
        uint16_t sent_len = huart->TxXferSize;
        uart_tx_fifo.tail = (uart_tx_fifo.tail + sent_len) % RING_BUFFER_SIZE;

        // 继续尝试发送剩余部分
        Start_DMA_Transmit_From_FIFO();
    }
}


