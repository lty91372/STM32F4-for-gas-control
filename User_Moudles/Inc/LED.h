/*
 * LED.h
 *
 *  Created on: Oct 26, 2025
 *      Author: HP
 */

#ifndef USER_MODULES_INC_LED_H_
#define USER_MODULES_INC_LED_H_

//LED状态
typedef enum {
    LED_ON,
    LED_OFF,
} LEDState_t;

//LED结构体
typedef struct{
	GPIO_TypeDef* port;
	uint8_t pin;
	LEDState_t led_currentstate;
}LED_t;



#endif /* USER_MODULES_INC_LED_H_ */
