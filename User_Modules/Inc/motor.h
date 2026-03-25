/*
 * motor.h
 *
 *  Created on: Oct 26, 2025
 *      Author: HP
 */

#ifndef USER_MODULES_INC_MOTOR_H_
#define USER_MODULES_INC_MOTOR_H_
#define PWM_MAX (2100-1)
#define MOTOR_NUM 4
#define MAX_SPEED 2099 //电机速度上限


//电机运行状态
typedef enum{
	MOTOR_STATE_STOPPED,
	MOTOR_STATE_RUNNING
}Motor_State;


typedef struct{
	TIM_HandleTypeDef* address; //电机定时器地址
	uint32_t addrees_channel;   //电机定时器channel编号

	uint8_t id;					//motor tag
	uint16_t current_speed;		//motor current speed?(maybe no need)
	Motor_State current_state;	//motor current running state
	uint16_t max_speed;			//motor maxim speed
}Motor;

extern Motor System_motors[];	//Motor array for other model use motor model,read its state,adjust its speed

void Motor_Init(void);			//motor initialization
void Set_Motor_Speed(int id,uint16_t speed);
void Motor_Hardware_Refresh(uint8_t id);
Motor_State Get_Motor_State(uint8_t id);
void Motor_Enable(uint8_t id);	//Motor enable function,use it to adjust motor's state to RUNNIG
void Motor_Disable(uint8_t id);	//Motor disable function,use it to adjust motor's state to STOP

void ADC_PWM1_Control(uint16_t anodata);
void Motor1_PWM_Control(uint16_t duty);
void Motor2_PWM_Control(uint16_t duty);
void Air_Motor_Control(void);
//uint16_t GetKerboardOutput();
//void Key_Input_PWM_Control();




#endif /* USER_MODULES_INC_MOTOR_H_ */
