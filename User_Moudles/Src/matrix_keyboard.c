/*
 * matrix_keyboard.c
 *
 *  Created on: Oct 26, 2025
 *      Author: HP
 */
#include "main.h"
#include "matrix_keyboard.h"
#include "Key.h"
#include <string.h>
#include "uart_transmit_moudle.h"
#include "usart.h"
#include <stdio.h>


//矩阵键盘高电平输出,由于其读取的特殊性，与先前按键逻辑分开处理
static KeyBoard_Output GPIO_OutPuts[OUTPUT_NUM] = {
		{.port = GPIOA,.pin = GPIO_PIN_9,.istriggerd = 0},
		{.port = GPIOC,.pin = GPIO_PIN_9,.istriggerd = 0},
		{.port = GPIOC,.pin = GPIO_PIN_7,.istriggerd = 0},
		{.port = GPIOD,.pin = GPIO_PIN_15,.istriggerd = 0}
};
//keyboard dictionary
const char KeyBoardDict[KEY_ROW][KEY_COLS]={
		{'1',    '2',    '3',    'A'},
		{'4',    '5',    '6',    'B'},
		{'7',    '8',    '9',    'C'},
		{'*',    '0',    '#',    'D'}
};

//volatile uint8_t Keyboard_flag[KEY_ROW][KEY_COLS] = {
//		{0x00,0x00,0x00,0x00},
//		{0x00,0x00,0x00,0x00},
//		{0x00,0x00,0x00,0x00},
//		{0x00,0x00,0x00,0x00}
//};


char Output[BUFFSIZE] = {'\0'};
char Input_Buff[BUFFSIZE] = {'\0'};
uint8_t debug_flag = 0;
char debug_data = '\0';
uint8_t input_index = 0;
//矩阵键盘当前按键变量，用于存储当前按键位置信息
KeyBoard_Triggered KeyBoard_Triggerd_Vector  = {.x = 100,.y = 100};
uint8_t Output_Finish = 0;

//清除标志位信息
void Vector_Clear()
{
	KeyBoard_Triggerd_Vector.x = 100;
	KeyBoard_Triggerd_Vector.y = 100;
}

//读取ABC通道
char Key_for_Channel()
{
	for(uint8_t i = 0;i < 3;i++)
	{
		return KeyBoardDict[i][3];
	}
}

//清除通道选择标志
void clear_channel_flag(uint8_t i)
{
	Keyboard_flag[i][3] &= !KEY_CHANNEL_SELCTION;
}


//矩阵键盘状态机,沿用基本的按键检测逻辑，在中断服务函数中不断检测四个列引脚电平
//需要经过四次中断服务函数才能完成一次扫描，建议修改消抖计时器的数值，以及修改TIM14实现分时复用
void Keyboard_State_Detect()
{
	static uint8_t row= 0;
	HAL_GPIO_WritePin(GPIO_OutPuts[(row+3)%4].port,GPIO_OutPuts[(row+3)%4].pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO_OutPuts[row].port, GPIO_OutPuts[row].pin, GPIO_PIN_SET);
	for(int col = 0;col < 4;col++)
		{
			if(Keys[col+1].istriggered)
			{
				HAL_UART_Transmit(&huart4, (uint8_t*)"Pin Triggered!\r\n", 16, 10);

				KeyBoard_Triggerd_Vector.x = row;
				KeyBoard_Triggerd_Vector.y = col;
				Keys[col+1].istriggered = 0;
			}
		}
	row = (row + 1) % 4;
}


//矩阵键盘输出控制
void KeyBoard_Ouput_Control()
{
	for(int i = 0;i < 4;i++)
	{
		if(GPIO_OutPuts[i].istriggerd == 1)
		{
			HAL_GPIO_WritePin(GPIO_OutPuts[(i+3)%4].port,GPIO_OutPuts[(i+3)%4].pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIO_OutPuts[i].port,GPIO_OutPuts[i].pin,GPIO_PIN_SET);
			GPIO_OutPuts[i].istriggerd = 0;
		}
	}
}


////矩阵键盘状态机，设有两个状态，按下A开始输入，输入的内容会被保存到buff中，按下D返回输出，存在output中
//void KeyBoard_Output_Num_SM()
//{
//	static int i = 0;
//	switch(KeyBoard_current_state)
//	{
//	case WAITING:
//		{
//			if((Keyboard_flag[0][3] & KEY_MAIN_DISPLAY) == 1)
//			{
//				KeyBoard_current_state = RECEIVING;
//				Vector_Clear();
//				Keyboard_flag[0][3] &= !KEY_MAIN_DISPLAY;
//			}
//		}
//		break;
//	case RECEIVING:
//		{
//			//按下D
//			if ((Keyboard_flag[3][3] & KEY_MAIN_DISPLAY))
//			{
//				strcpy(Output,Buff);
//				Buff[0] = '\0';
//				i = 0;
//				Output_Finish = 1;
//				HAL_UART_Transmit_DMA(&huart4,(uint8_t*)(Output),strlen(Output));
//				Vector_Clear();
//				Keyboard_flag[3][3] &= !KEY_MAIN_DISPLAY;
//				KeyBoard_current_state = WAITING;
//			}
//			else
//			{
//				//按下数字
//				if((KeyBoard_Triggerd_Vector.x <= 2 && KeyBoard_Triggerd_Vector.y <= 2)||
//						(KeyBoard_Triggerd_Vector.x == 3 && KeyBoard_Triggerd_Vector.y == 1))
//				{
//					Buff[i] = KeyBoardDict[KeyBoard_Triggerd_Vector.x][KeyBoard_Triggerd_Vector.y];
//					Keyboard_flag[KeyBoard_Triggerd_Vector.x][KeyBoard_Triggerd_Vector.y] &= !KEY_MAIN_DISPLAY;
//					Vector_Clear();
//					i++;
//					Buff[i] = '\0';
//				}
//				//按下B回退
//				else if(Keyboard_flag[1][3] & KEY_MAIN_DISPLAY)
//				{
//					Buff[--i] = '\0';
//					Keyboard_flag[KeyBoard_Triggerd_Vector.x][KeyBoard_Triggerd_Vector.y] &= !KEY_MAIN_DISPLAY;
//					Vector_Clear();
//				}
//
//			}
//		}
//		break;
//	}
//}

/*输入数字判断
 * 输入数字，存入缓冲区Input_Buff
 * 如果输入“*”,退格
 */
void Keyboard_Input_Detect_SM()
{
	if((KeyBoard_Triggerd_Vector.x < 3 && KeyBoard_Triggerd_Vector.y < 3)||(KeyBoard_Triggerd_Vector.x == 3 && KeyBoard_Triggerd_Vector.y == 1))
	{
		if(input_index < (BUFFSIZE-1))
		{
			Input_Buff[input_index] = KeyBoardDict[KeyBoard_Triggerd_Vector.x][KeyBoard_Triggerd_Vector.y];
			Input_Buff[++input_index] = '\0';
			debug_flag = 1;//debug 指令
			Vector_Clear();
		}
	}
	else if(KeyBoard_Triggerd_Vector.x == 3 && KeyBoard_Triggerd_Vector.y == 0)
	{
		if(input_index > 0)
		{
			Input_Buff[--input_index] = '\0';
			debug_flag = 1;
			Vector_Clear();
		}
	}
}


/**
 * @brief 实时回传缓冲区内容的调试函数
 */
void Debug_Buffer_Transmit(void)
{
    // 利用你原有的 debug_flag (在 SM 函数中存入新字符时会置 1)
    if(debug_flag == 1)
    {
        // 构造回传信息，加上方括号 [] 方便观察缓冲区起始和结尾
        char debug_msg[64];
        sprintf(debug_msg, "Buffer: [%s]\r\n", Input_Buff);
        // 使用阻塞式发送，确保数据完整到达
        HAL_UART_Transmit(&huart4, (uint8_t*)debug_msg, strlen(debug_msg), 100);
        // 复位标志位，等待下一次键盘动作
        debug_flag = 0;
    }
}


uint16_t GetKerboardOutput()
{
	uint16_t result_dec = 0;
	if (sscanf(Output, "%hu", &result_dec) == 1) {
				return result_dec;
		}
}


