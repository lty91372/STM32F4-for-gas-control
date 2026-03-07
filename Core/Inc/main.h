/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED1_Pin GPIO_PIN_6
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_7
#define LED2_GPIO_Port GPIOA
#define keyboard_Pin GPIO_PIN_15
#define keyboard_GPIO_Port GPIOB
#define keyboardD9_Pin GPIO_PIN_9
#define keyboardD9_GPIO_Port GPIOD
#define keyboardD11_Pin GPIO_PIN_11
#define keyboardD11_GPIO_Port GPIOD
#define keyboardD13_Pin GPIO_PIN_13
#define keyboardD13_GPIO_Port GPIOD
#define keyboardx_Pin GPIO_PIN_15
#define keyboardx_GPIO_Port GPIOD
#define PWM_Pin GPIO_PIN_6
#define PWM_GPIO_Port GPIOC
#define keyboardxC7_Pin GPIO_PIN_7
#define keyboardxC7_GPIO_Port GPIOC
#define keyboaedx_Pin GPIO_PIN_9
#define keyboaedx_GPIO_Port GPIOC
#define keyboardxA9_Pin GPIO_PIN_9
#define keyboardxA9_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_1
#define LED_GPIO_Port GPIOD
#define key_touch_Pin GPIO_PIN_1
#define key_touch_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
