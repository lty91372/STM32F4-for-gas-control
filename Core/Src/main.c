/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sys.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "touch.h"

#include "motor.h"
#include "gas_sensor.h"
#include "PID_control.h"
#include "uart_transmit_moudle.h"
#include "gas_control.h"
//#include "usart.h"

#include "lvgl.h"
#include "lv_port_indev.h"
#include "lv_port_disp.h"

#include "gui_guider.h"
#include "events_init.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LVGL_INITIALIZE 1
#define LVGL_BUTTON_DEBUG 1
#define TIME_MODE 1
#define BUTTON_SM 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
lv_ui guider_ui;		// GUI Guider生成的全局变量

#if TIME_MODE
/*
 * 算法更新放在硬件定时器中，初步设定更新时间为200ms
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM14)
	{


		static uint32_t time_inc = 0;
		time_inc++;
		if(time_inc % 50 == 0)
		{
#if BUTTON_SM
			Gas_Channel_Control_State_Machine();
#endif
			Air_Motor_Control();
		}
		if(time_inc % 200 == 0)
		{
//			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
			Gas_Channel_Control_Update();
		}

	}
}
#endif

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FSMC_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM14_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_UART4_Init();
  MX_USART1_UART_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
  delay_init();
  LED_Init();
  tp_dev.init();
  KEY_Init();
/*
 * 核心功能初始化
 */
  Motor_Init();
  GasSensor_Init();
  Gas_Channel_Control_Init();

#if LVGL_INITIALIZE
  //lvgl屏幕初始化
  lv_init();                          /* lvgl系统初始化 */
  lv_port_disp_init();                /* lvgl显示接口初始化 */
  lv_port_indev_init();

  POINT_COLOR=RED;
  LCD_ShowString(30,50,200,16,16,"Explorer STM32F407");
  LCD_ShowString(30,70,200,16,16,"TOUCH TEST");
  POINT_COLOR=BROWN;
  LCD_ShowString(30,90,200,16,16,"Create@DA_NIU_BI");
  POINT_COLOR=0XFC07;
  LCD_ShowString(30,110,200,16,16,"2026/2/22");


  if(tp_dev.touchtype!=0XFF)
  {
  	LCD_ShowString(30,130,200,16,16,"Press KEY0 to Adjust");//µç×èÆÁ²ÅÏÔÊ¾
  }
  delay_ms(1000);

  setup_ui(&guider_ui);
  events_init(&guider_ui);
#endif

/*
 * 中断与pwm配置
 */
  HAL_TIM_Base_Start_IT(&htim14);
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim5,TIM_CHANNEL_1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


while(1)
{
#if LVGL_BUTTON_DEBUG
	  tp_dev.scan(0);
	  lv_timer_handler();


	  if (input_updated[0]) {
		  LED0=0; delay_ms(100); LED0=1;
		  input_updated[0] = false;   // 清除标志
	  }
	  if (input_updated[1]) {
		  LED1=0; delay_ms(100); LED1=1;
		  input_updated[1] = false;   // 清除标志
	  }
	  if (input_updated[2]) {
		  LED0=LED1=0; delay_ms(100); LED0=LED1=1;
		  input_updated[2] = false;   // 清除标志
	  }


	  if (switch_updated[0]) {
		  if (g_switch_states[0]) {
			  LED0=0;
		  }
		  else	LED0=1;
		  switch_updated[0] = false;
	  }
	  if (switch_updated[1]) {
		  if (g_switch_states[1]) {
			  LED1=0;
		  }
		  else	LED1=1;
		  switch_updated[1] = false;
	  }
	  if (switch_updated[2]) {
		  if (g_switch_states[2]) {
			  LED0=LED1=0;
		  }
		  else	LED0=LED1=1;
		  switch_updated[2] = false;
	  }
#endif
		  GasSensor_Scheduler();
}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
