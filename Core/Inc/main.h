/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Dimmer_Pin GPIO_PIN_5
#define Dimmer_GPIO_Port GPIOE
#define InputCapture_Pin GPIO_PIN_6
#define InputCapture_GPIO_Port GPIOF
#define UsDelayTimer_Pin GPIO_PIN_8
#define UsDelayTimer_GPIO_Port GPIOF
#define EmitterPWM_Pin GPIO_PIN_9
#define EmitterPWM_GPIO_Port GPIOF
#define WR0_Pin GPIO_PIN_1
#define WR0_GPIO_Port GPIOG
#define COL1_Pin GPIO_PIN_8
#define COL1_GPIO_Port GPIOD
#define ROW5_Pin GPIO_PIN_10
#define ROW5_GPIO_Port GPIOD
#define ROW4_Pin GPIO_PIN_12
#define ROW4_GPIO_Port GPIOD
#define ROW1_Pin GPIO_PIN_13
#define ROW1_GPIO_Port GPIOD
#define ROW3_Pin GPIO_PIN_14
#define ROW3_GPIO_Port GPIOD
#define ROW2_Pin GPIO_PIN_15
#define ROW2_GPIO_Port GPIOD
#define DB7_Pin GPIO_PIN_2
#define DB7_GPIO_Port GPIOG
#define DB6_Pin GPIO_PIN_3
#define DB6_GPIO_Port GPIOG
#define DB5_Pin GPIO_PIN_4
#define DB5_GPIO_Port GPIOG
#define DB4_Pin GPIO_PIN_5
#define DB4_GPIO_Port GPIOG
#define DB3_Pin GPIO_PIN_6
#define DB3_GPIO_Port GPIOG
#define DB2_Pin GPIO_PIN_7
#define DB2_GPIO_Port GPIOG
#define DB1_Pin GPIO_PIN_8
#define DB1_GPIO_Port GPIOG
#define COL4_Pin GPIO_PIN_0
#define COL4_GPIO_Port GPIOD
#define COL3_Pin GPIO_PIN_4
#define COL3_GPIO_Port GPIOD
#define COL2_Pin GPIO_PIN_6
#define COL2_GPIO_Port GPIOD
#define DB0_Pin GPIO_PIN_9
#define DB0_GPIO_Port GPIOG
#define WR1_Pin GPIO_PIN_12
#define WR1_GPIO_Port GPIOG
#define CD_Pin GPIO_PIN_15
#define CD_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */