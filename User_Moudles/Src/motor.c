/*
 * motot.c
 *
 *  Created on: Oct 26, 2025
 *      Author: HP
 */
#include "main.h"
#include "motor.h"
#include "tim.h"
#include "matrix_keyboard.h"

Motor System_motors[MOTOR_NUM];
uint8_t pwm_key_flag = 0;

//需要单独进行初始化
void Motor_Init(void)
{
	//电机0，氧气电机
	System_motors[0].id = 0;
	System_motors[0].address = &htim3;
	System_motors[0].addrees_channel = TIM_CHANNEL_1;
	System_motors[0].max_speed = MAX_SPEED;
	System_motors[0].current_speed = 0;
	System_motors[0].current_state = MOTOR_STATE_STOPPED;
	__HAL_TIM_SET_COMPARE(System_motors[0].address,System_motors[0].addrees_channel,System_motors[0].current_speed);
}


//配置电机速度,输入参数应为占空比
void Set_Motor_Speed(int id,uint16_t speed)
{
	if (id < 0 || id >= MOTOR_NUM) {
		return; // 索引越界
			}
//	if(System_motors[id].current_state == MOTOR_STATE_RUNNING)
//	{
		if(speed <= System_motors[id].max_speed)
		{
			System_motors[id].current_speed = speed;
			__HAL_TIM_SET_COMPARE(System_motors[id].address,System_motors[id].addrees_channel,System_motors[id].current_speed);
			System_motors[id].current_state = MOTOR_STATE_RUNNING;
		}
		else
		{
			System_motors[id].current_speed = System_motors[id].max_speed;
			__HAL_TIM_SET_COMPARE(System_motors[id].address,System_motors[id].addrees_channel,System_motors[id].max_speed);
			System_motors[id].current_state = MOTOR_STATE_RUNNING;
		}
//	}
}
//电机使能
Motor_State Get_Motor_State(uint8_t id)
{
	return System_motors[id].current_state;
}
//电机失能
void Motor_Enable(uint8_t id)
{
	System_motors[id].current_state = MOTOR_STATE_RUNNING;
}

void Motor_Disable(uint8_t id)
{
	System_motors[id].current_state = MOTOR_STATE_STOPPED;
}






//PWM控制函数（用ADC的值来控制PWM的输出）
void ADC_PWM1_Control(uint16_t anodata)
{
	uint8_t duty = (uint16_t)(((float)anodata/4096.00) * 1000);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1,duty);
}


/*
 * duty 占空比（1-1000）
 */
//PWM控制函数，给定值控制
void Motor1_PWM_Control(uint16_t duty)
{
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1,duty);
}

void Key_Input_PWM_Control()
{
		if(GetKerboardOutput() <= PWM_MAX && GetKerboardOutput() >= 0)
			{
				__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1,GetKerboardOutput());
			}
		else if (GetKerboardOutput() > PWM_MAX)
			{
				__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1,PWM_MAX);
			}
		else if (GetKerboardOutput()<0)
			{
				__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1,0);
			}
}

