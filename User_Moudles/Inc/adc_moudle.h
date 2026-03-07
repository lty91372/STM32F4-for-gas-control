/*
 * ADC_Moudle.h
 *
 *  Created on: Oct 26, 2025
 *      Author: HP
 */

#ifndef USER_MODULES_INC_ADC_MOUDLE_H_
#define USER_MODULES_INC_ADC_MOUDLE_H_
#include <stdint.h>
#define ADC_BUF_LEN 3

void N_Filter();

uint16_t Get_adc_Raw(int i);
uint16_t Get_adc_FilterBuff(int i);
uint16_t Get_adc_After_Filter(int i);
uint32_t* Get_adcBuf_Address();

#endif /* USER_MODULES_INC_ADC_MOUDLE_H_ */
