/*
 * ui_logic.h
 *
 *  Created on: Mar 7, 2026
 *      Author: HP
 */
#ifndef UI_LOGIC_H
#define UI_LOGIC_H
#define CHANNEL_NUM 3

extern uint8_t Button_Channel_Selction_flag[CHANNEL_NUM];
extern uint8_t Button_Channel_Running_flag[CHANNEL_NUM];
extern uint8_t Button_Num_Input[10];
extern uint8_t Button_Confirm_flag;
extern uint8_t Button_Backward_flag;

extern uint8_t Input_Buff[256];

typedef struct{
	RUNNING;
	IDLE;
	WRITING;
};

void Button_Input_SM(void);



#endif




