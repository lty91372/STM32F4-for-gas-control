/*
 * PID_control.h
 *
 *  Created on: Nov 9, 2025
 *      Author: HP
 */

#ifndef INC_PID_CONTROL_H_
#define INC_PID_CONTROL_H_

#define PID_TIME 200
#define PID_PARA_DEBUG_MODE 0

#include "main.h"

extern uint8_t pid_info_buff[256];

typedef struct{
	float l_kp;
	float l_ki;
	float l_kd;
	float last_error;
	float integral;
	float PWMoutput_max;
	float PWMoutput_min;
	float output;
	float pid_t;
}PID_controller;

//PID部分
void PID_Init(PID_controller* pid,float p,float i,float d,float pwm_output_max,float pwm_output_min);
void PID_Calculate(PID_controller* pid,float target,float current_value);
void PID_Reset(PID_controller* pid);
float Get_PID_Output(PID_controller* pid);
void PID_Debug_Transmit(void);




#endif /* INC_PID_CONTROL_H_ */
