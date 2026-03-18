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
#include "tim.h"
#include "events_init.h"

#define PID_UPDATE_TIME 200

#define CHANNEL_DEBUG_MODE 1
#define BUTTON_LOGIC_DEBUG 1
#define CCR_DEBUG_MODE 0
#define BUTTON_SM_DEBUG 1


Gas_Channel_Controller System_Gas_Channel_Controller[GAS_CHANNEL_NUM];
char Channel_Dict[GAS_CHANNEL_NUM] ={'A','B','C'};
char transmit_buff[256];
char debug_buff[256];
const char* Gas_State_Dict[] = {
    [STOPPED]              = "STOPPED",
    [AUTO_RUNNING]         = "AUTO_RUNNING",
    [MANUAL_CONFIGURATION] = "MANUAL_CONFIG"
};


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
				20.0f,
				5.0f,
				1.0f,
				(float)System_Gas_Channel_Controller[i].System_motors->max_speed,
				0.0f);
		//初始化完成之后串口回传
		sprintf(transmit_buff,"Channel %c has been constructed\n",Channel_Dict[i]);
		Uart_Write_Buff((uint8_t*)(transmit_buff),strlen(transmit_buff));
	}
	//初始化完成之后串口回传
	sprintf(transmit_buff,"Initialization Finish\n");
	Uart_Write_Buff((uint8_t*)(transmit_buff),strlen(transmit_buff));

# if CHANNEL_DEBUG_MODE
	//debug
	System_Gas_Channel_Controller[0].current_gas_channel_state = AUTO_RUNNING;
	System_Gas_Channel_Controller[0].target_gas_concentration = 37;
	System_Gas_Channel_Controller[0].System_motors->current_state = MOTOR_STATE_RUNNING;
	sprintf(transmit_buff,"Channel A Debug Initialization Finish\n");
	Uart_Write_Buff((uint8_t*)transmit_buff,strlen(transmit_buff));
#endif
}


/**
 * @brief  通道状态更新及硬件驱动函数
 * @note   调度频率：建议 20ms/次 (50Hz)。该函数实现了从传感器采样到硬件输出的完整闭环控制。
 * * 程序执行流程：
 * 1. 状态自检：遍历所有通道，检查是否处于 AUTO_RUNNING（自动运行）模式。
 * 2. 采样感知 (Input)：从传感器结构体中同步最新的气体浓度数据。
 * 3. 算法决策 (Process)：将实时浓度作为反馈量放入 PID 算法，计算出目标 PWM 占空比。
 * 4. 逻辑映射 (Map)：通过 Set_Motor_Speed 将计算结果暂存至电机控制镜像中。
 * 5. 物理执行 (Output)：无论处于何种状态，最终统一调用 Hardware_Refresh 将逻辑状态刷新至硬件寄存器。
 *
 * @param  None
 * @retval None
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
			PID_Debug_Transmit();
//#if PID_PARA_DEBUG_MODE
//			static uint8_t debug_cnt = 0;
//			if(debug_cnt++ > 5)
//			{
//				PID_Debug_Transmit();
//				debug_cnt = 0;
//			}
//#endif
			//控制电机输出
			Set_Motor_Speed(System_Gas_Channel_Controller[i].channel,Get_PID_Output(&(System_Gas_Channel_Controller[i].pid)));
		}
#if BUTTON_SM_DEBUG
			int len = sprintf(transmit_buff, "CH[%d] %s Tgt:%d\r\n",
							 i,
							 Gas_State_Dict[System_Gas_Channel_Controller[i].current_gas_channel_state],
							 (int)System_Gas_Channel_Controller[i].target_gas_concentration);

			Uart_Write_Buff((uint8_t*)transmit_buff, len);
#endif

		Motor_Hardware_Refresh(i);

	}

#if CCR_DEBUG_MODE
    uint32_t tim_arr_value = htim3.Instance->CCR1;
    sprintf(debug_buff, "Running, Timer ARR: %lu\r\n", tim_arr_value);
    Uart_Write_Buff((uint8_t*)debug_buff, strlen(debug_buff));
#endif

}




//void Gas_Debug_Transmit(void)
//{
//	float current_gas_concentration = System_Gas_Channel_Controller[0].current_gas_concentration;
//	float PID_output = Get_PID_Output(&System_Gas_Channel_Controller[0].pid);
//	float target = System_Gas_Channel_Controller[0].target_gas_concentration;
//	float integral = System_Gas_Channel_Controller[0].pid.integral;
//	sprintf(debug_buff, "%f,%f,%f,%f",current_gas_concentration,target,integral,PID_output);
//	HAL_UART_Transmit_DMA(&huart4, (uint8_t*)pid_info_buff, strlen(pid_info_buff));
//}


#if BUTTON_LOGIC_DEBUG
/**
 * @brief  气体通道控制状态机
 * @note   该函数负责处理用户交互（按键）并根据当前状态切换通道运行模式。
 * 主要状态流转：STOPPED -> MANUAL_CONFIGURATION -> AUTO_RUNNING -> STOPPED
 * * 程序执行流程：
 * 1. 遍历通道：循环处理所有定义的控制通道。
 * 2. STOPPED (停止模式):
 * - 确保电机物理失能。
 * - 监听“通道选择”按键，触发后进入配置模式。
 * 3. MANUAL_CONFIGURATION (手动配置模式):
 * - 等待用户输入目标值（Button_Confirm_flag）。
 * - 解析输入数据并更新目标气体浓度（target_gas_concentration）。
 * - 监听“启动”按键，触发后进入自动运行模式。
 * 4. AUTO_RUNNING (自动运行模式):
 * - 确保电机使能（允许 PID 控制逻辑干预）。
 * - 持续监听“停止”按键，触发后立即切回停止模式并失能电机。
 */
void Gas_Channel_Control_State_Machine()
{
	for(int i = 0;i < GAS_CHANNEL_NUM;i++)
	{
		switch(System_Gas_Channel_Controller[i].current_gas_channel_state)
		{
			case STOPPED:
				//停滞状态下电机失能
				if(System_Gas_Channel_Controller[i].System_motors->current_state !=MOTOR_STATE_STOPPED)
					Motor_Disable(i);
				if(input_updated[i])
					{
						//检测到选中该通道，则该通道进入配置模式
						float current_Input;
						sscanf((uint8_t*)g_input_values[i],"%f",&current_Input);
						sprintf(transmit_buff, "Set Channel %d,input=,%f",i,current_Input);
						Uart_Write_Buff((uint8_t*)(transmit_buff),strlen(transmit_buff));
						System_Gas_Channel_Controller[i].target_gas_concentration = (float)current_Input;
						input_updated[0] = 0;
					}
				if(g_switch_states[i])
					{
						System_Gas_Channel_Controller[i].current_gas_channel_state = AUTO_RUNNING;
					}
					break;
				break;
			case MANUAL_CONFIGURATION:
				//在配置模式状态下，若检测到回车键（D）被按下，改变目标浓度,并进入自动运行状态

			case AUTO_RUNNING:
				//电机使能
				if(System_Gas_Channel_Controller[i].System_motors->current_state == MOTOR_STATE_STOPPED)
					Motor_Enable(i);
				if(input_updated[i])
				{
					//检测到选中该通道，则该通道进入配置模式
					float current_Input;
					sscanf((uint8_t*)g_input_values[i],"%f",&current_Input);
					sprintf(transmit_buff, "Set Channel %d,input=,%f",i,current_Input);
					Uart_Write_Buff((uint8_t*)(transmit_buff),strlen(transmit_buff));
					System_Gas_Channel_Controller[i].target_gas_concentration = (float)current_Input;
					input_updated[0] = 0;
				}
				//在自动运行状态下，若检测到停止按键（#)，则进入停止状态，电机停止转动
				if(!g_switch_states[i])
				{
					//Motor stop
					System_Gas_Channel_Controller[i].current_gas_channel_state = STOPPED;
					Motor_Disable(i);

					sprintf(transmit_buff, "Stop\n");
					Uart_Write_Buff((uint8_t*)(transmit_buff),strlen(transmit_buff));
				}
				break;

		}
	}
}
#endif





