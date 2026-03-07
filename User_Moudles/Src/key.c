/*
 * gpio_common.c
 *
 *  Created on: Oct 26, 2025
 *      Author: HP
 */


#include "main.h"
#include "key.h"
#include "gpio.h"


//按键
Key_t Keys[KEY_NUM] = {
		{.port = GPIOE,.pin = GPIO_PIN_1,.debouncecounter = 0,.currentState = KEY_STATE_UP,.istriggered = 0},
		{.port = GPIOD,.pin = GPIO_PIN_13,.debouncecounter = 0,.currentState = KEY_STATE_UP,.istriggered = 0},
		{.port = GPIOD,.pin = GPIO_PIN_11,.debouncecounter = 0,.currentState = KEY_STATE_UP,.istriggered = 0},
		{.port = GPIOD,.pin = GPIO_PIN_9,.debouncecounter = 0,.currentState = KEY_STATE_UP,.istriggered = 0},
		{.port = GPIOB,.pin = GPIO_PIN_15,.debouncecounter = 0,.currentState = KEY_STATE_UP,.istriggered = 0},
};

//按键检测+消抖处理
void Key_State_Detect()
{
	for(int i = 0;i < KEY_NUM;i++)
	{
		GPIO_PinState pinState = HAL_GPIO_ReadPin(Keys[i].port,Keys[i].pin);
		if(pinState == GPIO_PIN_SET)
		{
			if(Keys[i].currentState == KEY_STATE_UP)
			{
				Keys[i].currentState = KEY_STATE_DEBOUNCING;
				Keys[i].debouncecounter = 0;
			}
			else if(Keys[i].currentState == KEY_STATE_DEBOUNCING)
			{
				Keys[i].debouncecounter++;
				if(Keys[i].debouncecounter >= DEBOUNCE_TIME_CYCLES)
				{
					Keys[i].debouncecounter = 0;
					Keys[i].currentState = KEY_STATE_DOWN;
					Keys[i].istriggered = 1;
				}
			}
		}
		else
		{
			if(Keys[i].currentState == KEY_STATE_DEBOUNCING || Keys[i].currentState == KEY_STATE_DOWN)
			{
				Keys[i].currentState = KEY_STATE_UP;
				Keys[i].debouncecounter = 0;
			}
		}
	}
}





