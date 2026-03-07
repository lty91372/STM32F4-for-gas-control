/*
 * gpio_common.h
 *
 *  Created on: Oct 26, 2025
 *      Author: HP
 */

#ifndef USER_MODULES_INC_KEY_H_
#define USER_MODULES_INC_KEY_H_

#define KEY_NUM 5
#define DEBOUNCE_TIME_CYCLES 5



//按键状态机
typedef enum {
    KEY_STATE_UP,
    KEY_STATE_DOWN,
    KEY_STATE_DEBOUNCING
} KeyState_t;

//输入检测结构体
typedef struct{
	GPIO_TypeDef* port;
	uint16_t pin;
	KeyState_t currentState;
	uint8_t debouncecounter;
	uint8_t istriggered;//标志位，表示是否触发事件
}Key_t;

uint8_t Get_Keys_TriggerState(uint8_t index);
extern Key_t Keys[KEY_NUM];


#endif /* USER_MODULES_INC_KEY_H_ */
