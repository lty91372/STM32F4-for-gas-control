/*
 * LED.c
 *
 *  Created on: Oct 26, 2025
 *      Author: HP
 */


#include "main.h"
#include "led.h"
#include "key.h"

//PA6作为LED输出引脚
LED_t LEDs[1] = {{.port = GPIOA,.pin = GPIO_PIN_6,.led_currentstate = LED_OFF}};


//LED开关状态机
void LED_STATE_MACHINE()
{
	if(Keys[0].istriggered == 1)
	{
		if(LEDs[0].led_currentstate == LED_ON)
		{
			LEDs[0].led_currentstate = LED_OFF;
			HAL_GPIO_TogglePin(LEDs[0].port,LEDs[0].pin);
		}
		else
		{
			LEDs[0].led_currentstate = LED_ON;
			HAL_GPIO_TogglePin(LEDs[0].port,LEDs[0].pin);
		}
		Keys[0].istriggered = 0;
	}
}



