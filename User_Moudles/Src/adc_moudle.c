/*
 * ADC_Moudle.c
 *
 *  Created on: Oct 26, 2025
 *      Author: HP
 */
#include "main.h"
#include "adc_moudle.h"


//ADC使用的是硬件定时器自行触发的模式，只需要在初始化开启传输，后续就可以一直传输
//ADC的定时器是TIM2，将N_Filter放进TIM2的中断函数中即可正常使用


uint16_t adcBuf[ADC_BUF_LEN] = {0x0000,0x0000,0x0000};
uint32_t adcFilterBuff[ADC_BUF_LEN] = {0,0,0};
uint16_t adc_After_Filter[ADC_BUF_LEN] = {0,0,0};


//平均滤波函数
void N_Filter()
{
	static uint8_t index = 0;
	if(index < 32)
	{
		for (int i = 0;i < 3;i++)
		adcFilterBuff[i] += adcBuf[i];
		index++;
	}
	if(index == 32)
	{
		for(int i = 0;i < 3;i++)
		{
			adc_After_Filter[i] = adcFilterBuff[i] / 32;
			adcFilterBuff[i] = 0;
		}
		index = 0;
	}
}

uint16_t Get_adc_Raw(int i)
{
	return adcBuf[i];
}

uint16_t Get_adc_FilterBuff(int i)
{
	return adcFilterBuff[i];
}
//获取经过滤波处理后的adc数值
uint16_t Get_adc_After_Filter(int i)
{
	return adc_After_Filter[i];
}

uint32_t* Get_adcBuf_Address()
{
	return adcBuf;
}
