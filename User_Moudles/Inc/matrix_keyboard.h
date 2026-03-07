/*
 * matrix_keyboard.h
 *
 *  Created on: Oct 26, 2025
 *      Author: HP
 */

#ifndef USER_MODULES_INC_MATRIX_KEYBOARD_H_
#define USER_MODULES_INC_MATRIX_KEYBOARD_H_

#define KEY_ROW 4
#define KEY_COLS 4
#define OUTPUT_NUM 4
#define KEY_MAIN_DISPLAY (1<<0)
#define KEY_CHANNEL_SELCTION (1<<1)
#define BUFFSIZE 128


//矩阵键盘输出结构体定义
typedef struct{
	GPIO_TypeDef* port;
	uint16_t pin;
	uint8_t istriggerd;
}KeyBoard_Output;

//矩阵键盘数组
extern const char KeyBoardDict[KEY_ROW][KEY_COLS];
extern char Output[BUFFSIZE];
extern char Input_Buff[BUFFSIZE];
extern uint8_t input_index;

//测试用
extern char debug_data;
extern uint8_t debug_flag;
//Keyboard Cell State
typedef struct{
	uint8_t x;
	uint8_t y;
}KeyBoard_Triggered;

extern KeyBoard_Triggered KeyBoard_Triggerd_Vector;

typedef enum{
	RECEIVING,
	WAITING
}KeyBoard_State;

extern volatile uint8_t Keyboard_flag[KEY_ROW][KEY_COLS];
void Vector_Clear();
void Keyboard_State_Detect();
//void KeyBoard_Output_Num_SM();
void Keyboard_Input_Detect_SM();
void KeyBoard_Ouput_Control();
char Get_Button_Value();
char Key_for_Channel();
void clear_channel_flag(uint8_t i);
void Debug_Buffer_Transmit(void);

#endif /* USER_MODULES_INC_MATRIX_KEYBOARD_H_ */
