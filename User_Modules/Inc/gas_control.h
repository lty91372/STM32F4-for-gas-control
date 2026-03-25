/*
 * gas_control.h
 *
 *  Created on: Nov 9, 2025
 *      Author: HP
 */

#ifndef INC_GAS_CONTROL_H_
#define INC_GAS_CONTROL_H_
#include <stdlib.h>
#include "main.h"
#include "motor.h"
#include "PID_control.h"
#include "usart.h"
#include "gas_sensor.h"
#include <string.h>
#include <stdio.h>

#define GAS_CHANNEL_NUM 3

typedef enum{
	STOPPED,
	AUTO_RUNNING,
	MANUAL_CONFIGURATION
}Gas_Channel_State;

typedef struct{
	uint8_t channel;
	PID_controller pid;

	//独立初始化部件
	Motor* System_motors;
	GasSensor_Data_t* GasSensor;

	float target_gas_concentration;
	float current_gas_concentration;
	Gas_Channel_State current_gas_channel_state;

}Gas_Channel_Controller;
//
////按键状态标志位
//extern uint8_t Button_Channel_Selction_flag[GAS_CHANNEL_NUM];
//extern uint8_t Button_Channel_Running_flag[GAS_CHANNEL_NUM];
//extern uint8_t Button_Num_Input[10];
//extern uint8_t Button_Confirm_flag;

//extern uint8_t Input_Buff[256];

extern Gas_Channel_Controller System_Gas_Channel_Controller[];

void Gas_Channel_Control_Init();
void Gas_Channel_Control_Update();
void Channel_Selection_SM();
void Pid_Calculate_Enable(uint8_t index);
#endif /* INC_GAS_CONTROL_H_ */
