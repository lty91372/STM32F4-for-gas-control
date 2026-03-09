/*
 * gas_control.c
 *
 *  Created on: Nov 9, 2025
 *      Author: HP
 */
#include "gas_control.h"
#include "uart_transmit_moudle.h"
#include <stdio.h>
#include "PID_control.h"
#define GAS_CHANNEL_NUM 3
#define PID_UPDATE_TIME 200
#define DEBUG_MODE 0


Gas_Channel_Controller System_Gas_Channel_Controller[GAS_CHANNEL_NUM];
volatile uint32_t pid_update_counter;
char Channel_Dict[GAS_CHANNEL_NUM] ={'A','B','C'};
char transmit_buff[256];
char debug_buff[256];



/*
 * @brief 控制通道初始化函数
 * @note 注意：需要在电机模块初始化完成之后再调用，初始化每个通道电机、pid计算模块的状态，以及设置通道状态，并初始化每个通道的pid初始值
*/
void Gas_Channel_Control_Init()
{
	for(int i = 0;i < 3;i++)
	{
		System_Gas_Channel_Controller[i].channel = i;
		System_Gas_Channel_Controller[i].current_gas_channel_state = STOPPED;
		System_Gas_Channel_Controller[i].System_motors = &System_motors[i];
		System_Gas_Channel_Controller[i].GasSensor = &sensor_node[i];
		System_Gas_Channel_Controller[i].System_motors->current_state = MOTOR_STATE_STOPPED;
		PID_Init(&System_Gas_Channel_Controller[i].pid,
				200.0f,
				10.0f,
				1.0f,
				(float)System_Gas_Channel_Controller[i].System_motors->max_speed,
				0.0f);
		//初始化完成之后串口回传
		sprintf(transmit_buff,"Channel %c has been constructed\n",Channel_Dict[i]);
		HAL_UART_Transmit(&huart4, (uint8_t*)transmit_buff, strlen(transmit_buff), HAL_MAX_DELAY);//此处为了避免冲突，使用非dma传输模式
	}
	//初始化完成之后串口回传
	sprintf(transmit_buff,"Initialization Finish\n");
	HAL_UART_Transmit(&huart4,(uint8_t*)(transmit_buff),strlen(transmit_buff), HAL_MAX_DELAY);

# if DEBUG_MODE
	//debug
	System_Gas_Channel_Controller[0].current_gas_channel_state = AUTO_RUNNING;
	System_Gas_Channel_Controller[0].target_gas_concentration = 37;
	sprintf(transmit_buff,"Channel A Debug Initialization Finish\n");
	HAL_UART_Transmit(&huart4,(uint8_t*)(transmit_buff),strlen(transmit_buff), HAL_MAX_DELAY);
#endif
}


/*
 * @brief 通道状态更新函数
 * @note  参考时间：20ms触发一次
*/
void Gas_Channel_Control_Update()
{
	//对每个通道进行操作
	for(int i = 0;i < GAS_CHANNEL_NUM;i++)
	{
		//只有运行时才计算，不运行不计算
		if(System_Gas_Channel_Controller[i].current_gas_channel_state == AUTO_RUNNING)
		{
			//先从ADC回传的数据中获取浓度值
			System_Gas_Channel_Controller[i].current_gas_concentration =System_Gas_Channel_Controller[i].GasSensor->concentration;
			//计算PID值
			PID_Calculate(&System_Gas_Channel_Controller[i].pid,
					System_Gas_Channel_Controller[i].target_gas_concentration,
					System_Gas_Channel_Controller[i].current_gas_concentration);
			//控制电机输出
			Set_Motor_Speed(System_Gas_Channel_Controller[i].channel,Get_PID_Output(&System_Gas_Channel_Controller[i].pid));
#if DEBUG_MODE
    uint32_t tim_arr_value = htim3.Instance->CCR1;
    sprintf(debug_buff, "Running, Timer ARR: %lu\r\n", tim_arr_value);
    HAL_UART_Transmit_DMA(&huart4, (uint8_t*)debug_buff, strlen(debug_buff));
#endif
		}
	}
}




void Gas_Debug_Transmit(void)
{
	float current_gas_concentration = System_Gas_Channel_Controller[0].current_gas_concentration;
	float PID_output = Get_PID_Output(&System_Gas_Channel_Controller[0].pid);
	float target = System_Gas_Channel_Controller[0].target_gas_concentration;
	float integral = System_Gas_Channel_Controller[0].pid.integral;
	sprintf(debug_buff, "%f,%f,%f,%f",current_gas_concentration,target,integral,PID_output);
	HAL_UART_Transmit_DMA(&huart4, (uint8_t*)pid_info_buff, strlen(pid_info_buff));
}



//////气体PID控制的状态机函数
////键盘控制：A，B,C分别对应三个通道，按下按键选择通达，输入目标气体浓度，按下D返回，在按下D之前，该通道控制的电机不会运行
//void Gas_Channel_Control_State_Machine()
//{
//	for(int i = 0;i < GAS_CHANNEL_NUM;i++)
//	{
//		switch(System_Gas_Channel_Controller[i].current_gas_channel_state)
//		{
//			case STOPPED:
//				//停滞状态下电机失能
//				if(System_Gas_Channel_Controller[i].System_motors->current_state !=MOTOR_STATE_STOPPED)
//					Motor_Disable(i);
//				if(Keyboard_flag[i][3] & KEY_CHANNEL_SELCTION)
//					{
//						Keyboard_flag[i][3] &= !KEY_CHANNEL_SELCTION;
//						//检测到选中该通道，则该通道进入配置模式
//						System_Gas_Channel_Controller[i].current_gas_channel_state = MANUAL_CONFIGURATION;
//						switch(Channel_Dict[i])
//						{
//						case 'A':
//							sprintf(transmit_buff, "Channel A Selected\n");
//							break;
//						case 'B':
//							sprintf(transmit_buff, "Channel B Selected\n");
//							break;
//						case 'C':
//							sprintf(transmit_buff, "Channel C Selected\n");
//							break;
//						}
//						HAL_UART_Transmit_DMA(&huart4,(uint8_t*)(transmit_buff),strlen(transmit_buff));
//					}
//				break;
//			case MANUAL_CONFIGURATION:
//				//在配置模式状态下，若检测到回车键（D）被按下，改变目标浓度,并进入自动运行状态
//				if((Keyboard_flag[3][3] & KEY_CHANNEL_SELCTION))
//				{
//					Keyboard_flag[3][3] &= !KEY_CHANNEL_SELCTION;
//					sprintf(transmit_buff, "Set\n");
//					HAL_UART_Transmit_DMA(&huart4,(uint8_t*)(transmit_buff),strlen(transmit_buff));
//					uint16_t current_Input = GetKerboardOutput();
//					System_Gas_Channel_Controller[i].target_gas_concentration = (float)current_Input;
//					System_Gas_Channel_Controller[i].current_gas_channel_state = AUTO_RUNNING;
//				}
//			case AUTO_RUNNING:
//				//电机使能
//				if(System_Gas_Channel_Controller[i].System_motors->current_state == MOTOR_STATE_STOPPED)
//					Motor_Enable(i);
//				//在自动运行状态下，若检测到停止按键（#)，则进入停止状态，电机停止转动
//				if(Keyboard_flag[3][2] & KEY_CHANNEL_SELCTION)
//				{
//					Keyboard_flag[3][2] &= !KEY_CHANNEL_SELCTION;
//					//Motor stop
//					System_Gas_Channel_Controller[i].current_gas_channel_state = STOPPED;
//					sprintf(transmit_buff, "Stop\n");
//					HAL_UART_Transmit_DMA(&huart4,(uint8_t*)(transmit_buff),strlen(transmit_buff));
//
//				}
//
//		}
//		transmit_buff[0]='\0';
//		Keyboard_flag[i][3] &= !KEY_CHANNEL_SELCTION;
//	}
//}


void Channel_Selection_SM()
{
	static uint8_t channel_id = 10;
	uint8_t row = KeyBoard_Triggerd_Vector.x;
	uint8_t col = KeyBoard_Triggerd_Vector.y;
	if(row < 3 && col == 3)
	{
		Input_Buff[0] ='\0';
		input_index = 0;
		channel_id = row;
		debug_flag = 1;
		debug_data = KeyBoardDict[KeyBoard_Triggerd_Vector.x][KeyBoard_Triggerd_Vector.y];
		Vector_Clear();
	}
	else if(row == 3 && col == 3)
	{
		if(input_index > 0)
		{
			System_Gas_Channel_Controller[channel_id].target_gas_concentration = atof(Input_Buff);
			Input_Buff[0] ='\0';
			input_index = 0;
			debug_flag = 1;
			debug_data = KeyBoardDict[KeyBoard_Triggerd_Vector.x][KeyBoard_Triggerd_Vector.y];
			System_Gas_Channel_Controller[channel_id].current_gas_channel_state = AUTO_RUNNING;//临时
			Vector_Clear();
		}
	}
}






