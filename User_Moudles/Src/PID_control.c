/*
 * PID_control.c
 *
 *  Created on: Nov 9, 2025
 *      Author: HP
 */
#include "PID_control.h"
#include "uart_transmit_moudle.h"
#include <stdio.h>



uint8_t pid_info_buff[256];

PID_controller pid;

/**
 * @brief  PID初始化函数
 */
void PID_Init(PID_controller* pid,float p,float i,float d,float pwm_output_max,float pwm_output_min)
{
	pid->l_kp = p;
	pid->l_ki = i;
	pid->l_kd = d;
	pid->integral = 0;
	pid->last_error = 0;
	pid->pid_t = (float)PID_TIME/1000;
	pid->PWMoutput_max = pwm_output_max;
	pid->PWMoutput_min = pwm_output_min;
}

/**
 * @brief  PID计算函数初始化函数
 * @param pid : 传入的pid计算模块地址
 * @param target: 目标浓度
 * @param current_value:当前浓度，由传感器传入
 * @note 计算结果会保存在pid->output中
 */
void PID_Calculate(PID_controller* pid,float target,float current_value)
{
	float error = target - current_value;
//比例部分
	float proportional_item = pid->l_kp * error;
//积分部分，如果积分累积项太大，则设置为最大输出
	pid->integral += error * pid->pid_t;
	if(pid->integral > pid->PWMoutput_max) pid->integral = pid->PWMoutput_max;
	if(pid->integral < pid->PWMoutput_min) pid->integral = pid->PWMoutput_min;
	float integral_item = pid->integral * pid->l_ki;
//微分部分
	float derivative = (error-pid->last_error)/pid->pid_t;
	float derivative_item = pid->l_kd * derivative;
//输出控制
	float output = proportional_item + integral_item + derivative_item;
	if(output > pid->PWMoutput_max) output = pid->PWMoutput_max;
	if(output < pid->PWMoutput_min) output = pid->PWMoutput_min;
	pid->output = output;

#if PID_PARA_DEBUG_MODE
	//建议不要放在很快的定时器中
	sprintf((char *)pid_info_buff, "Err:%.2f | P:%.2f | I:%.2f | D:%.2f | Out:%.2f\r\n",
	        error, proportional_item, integral_item, derivative_item, output);
#endif
//更新误差
	pid->last_error = error;
}
/*
 * @brief 返回值的是pid输出值，为PWM的arr参数
 * @return pid->output pid运算值
*/
float Get_PID_Output(PID_controller* pid)
{
	return pid->output;
}
/*
 * @brief 关闭pid运算
*/
void PID_Reset(PID_controller* pid)
{
	pid->integral = 0;
	pid->last_error= 0;
	pid->output = 0;
}

void PID_Debug_Transmit()
{
	Uart_Write_Buff((uint8_t*)pid_info_buff,sizeof(pid_info_buff));
}



