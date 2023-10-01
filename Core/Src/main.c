/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "ILI9341_Touchscreen.h"

#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"

#include "snow_tiger.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint32_t VR[2];

/*Game Control*/
uint8_t spawnPosX[10] = {10,11,12,13,13,13,14,15,16,17};
uint8_t spawnPosY[10] = {10,10,10,10,11,12,12,12,12,12};
bool isGameStart = false;
uint32_t playerControlDelay = 20;//In millis
uint32_t playerMoveDelay = 150;
uint32_t currentMoveDelay = 0;

/*Player Control*/
uint8_t p1X[10] = {10,11,12,13,13,13,14,15,16,17};
uint8_t p1Y[10] = {10,10,10,10,11,12,12,12,12,12};
uint8_t p1Length = 10;
uint8_t p1PrevMoveType = 1; //0 = IDLE 1 = LEFT 2 = RIGHT 3 = UP 4 = DOWN
uint32_t p1MoveDelay = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void resetP1Position() {
	p1PrevMoveType = 1;
	for(int i = 0 ; i < p1Length ; i++) {
		p1X[i] = spawnPosX[i];
	}

	for(int i = 0 ; i < p1Length ; i++) {
		p1Y[i] = spawnPosY[i];
	}

	for(int i = 0 ; i < p1Length ; i++) {
		ILI9341_Draw_Rectangle(10*p1X[i], 10*p1Y[i], 10, 10, 0);
	}
}

void resetMap() {
	isGameStart = false;
	ILI9341_Fill_Screen(WHITE);
	ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
	for(int i = 0 ; i < 24 ; i++) {
		  if(i==0 || i == 23) {
			  for(int j = 0 ; j < 32 ; j++) {
				  ILI9341_Draw_Rectangle(10*j, 10*i, 10, 10, 30000);
			  }
		  } else {
			  ILI9341_Draw_Rectangle(0, 10*i, 10, 10, 30000);
			  ILI9341_Draw_Rectangle(310, 10*i, 10, 10, 30000);
		  }
	  }
	resetP1Position();
	isGameStart = true;
}

bool checkIsP1BiteSelf() {
	for(int i = 1 ; i < p1Length ; i++) {
		if(p1X[i] == p1X[0] && p1Y[i] == p1Y[0]) {
			resetMap();
			return true;
		}
	}
	return false;
}

void moveP1AutoMove(uint8_t moveType) {
	uint8_t tempp1X[p1Length];
	uint8_t tempp1Y[p1Length];
	bool isMove = false;
	if(moveType == 1) {
		if(p1X[0] > 1) {
			tempp1X[0] = p1X[0]-1;
			isMove = true;
		} else {
			//tempp1X[0] = p1X[0];
			resetMap();
			return;
		}
		tempp1Y[0] = p1Y[0];
	} else if(moveType == 2) {
		if(p1X[0] < 30) {
			tempp1X[0] = p1X[0]+1;
			isMove = true;
		} else {
			//tempp1X[0] = p1X[0];
			resetMap();
			return;
		}
		tempp1Y[0] = p1Y[0];
	} else if(moveType == 3) {
		if(p1Y[0] > 1) {
			tempp1Y[0] = p1Y[0]-1;
			isMove = true;
		} else {
			//tempp1Y[0] = p1Y[0];
			resetMap();
			return;
		}
		tempp1X[0] = p1X[0];
	} else if(moveType == 4) {
		if(p1Y[0] < 22) {
			tempp1Y[0] = p1Y[0]+1;
			isMove = true;
		} else {
			//tempp1Y[0] = p1Y[0];
			resetMap();
			return;
		}
		tempp1X[0] = p1X[0];
	}
	if(isMove && !checkIsP1BiteSelf()) {
		ILI9341_Draw_Rectangle(10*p1X[p1Length-1], 10*p1Y[p1Length-1], 10, 10, 65535); //Clear last index
		for(int i = 0 ; i < p1Length-1 ; i++) {
			tempp1X[i+1] = p1X[i];
			tempp1Y[i+1] = p1Y[i];
		}
		for(int i = 0 ; i < p1Length ; i++) {
			p1X[i] = tempp1X[i];
		    p1Y[i] = tempp1Y[i];
		}
	}

}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_SPI5_Init();
  MX_TIM1_Init();
  MX_RNG_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  ILI9341_Init();//initial driver setup to drive ili9341
  HAL_ADC_Start_DMA(&hadc1, VR, 2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  resetMap();

  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  int scaledValueX = VR[0];
	  int scaledValueY = VR[1];

	  // ส่งค่าที่�?ปลง�?ล้วไปยัง UART
	  char hexStringX[8];
	  char hexStringY[8];
	  char line[] = "\n\r";

	 /* snprintf(hexStringX, sizeof(hexStringX), "%d", scaledValueX);
	  snprintf(hexStringY, sizeof(hexStringY), " %d", scaledValueY);

	  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
	  HAL_UART_Transmit(&huart3, (uint8_t*)hexStringX, strlen(hexStringX), HAL_MAX_DELAY);

	  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
	  HAL_UART_Transmit(&huart3, (uint8_t*)hexStringY, strlen(hexStringY), HAL_MAX_DELAY);

	  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
	  HAL_UART_Transmit(&huart3, (uint8_t*)line, strlen(line), HAL_MAX_DELAY);*/

	  /*
	  for(int i = 0 ; i < 24 ; i++) {
		  for(int j = 0 ; j < 32 ; j++) {
			  if(i%2 == 0 && j%2 == 0) {
				  ILI9341_Draw_Rectangle(10*j, 10*i, 10, 10, 0);
			  } else if(i%2 == 0 && j%2 != 0) {
				  ILI9341_Draw_Rectangle(10*j, 10*i, 10, 10, 63488);
			  } else if(i%2 != 0 && j%2 == 0) {
				  ILI9341_Draw_Rectangle(10*j, 10*i, 10, 10, 63488);
			  } else if(i%2 != 0 && j%2 != 0) {
				  ILI9341_Draw_Rectangle(10*j, 10*i, 10, 10, 0);
			  }
		  }
	  }*/
	  if(isGameStart) {
		  //Movement Logic
		  if(HAL_GetTick() - p1MoveDelay >= 0) {
			  if(VR[0] <= 15) { //Left Movement
				  char type[] = "LEFT MOVEMENT";
				  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
				  	  HAL_UART_Transmit(&huart3, (uint8_t*)type, strlen(type), HAL_MAX_DELAY);
				  if(p1PrevMoveType != 2) {
					  p1PrevMoveType = 1;
				  }
			  } else if(VR[0] >= 3055) { //Right Movement
				  char type[] = "RIGHT MOVEMENT";
				  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
				 	HAL_UART_Transmit(&huart3, (uint8_t*)type, strlen(type), HAL_MAX_DELAY);
				 	if(p1PrevMoveType != 1) {
						 p1PrevMoveType = 2;
				 	}
			  } else if(VR[1] <= 15) { //Up Movement
				  char type[] = "UP MOVEMENT";
				  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
				 	HAL_UART_Transmit(&huart3, (uint8_t*)type, strlen(type), HAL_MAX_DELAY);
				 	if(p1PrevMoveType != 4) {
						 p1PrevMoveType = 3;
				 	}
			  } else if(VR[1] >= 3025) { //Down Movement
				  char type[] = "DOWN MOVEMENT";
				  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
				 	HAL_UART_Transmit(&huart3, (uint8_t*)type, strlen(type), HAL_MAX_DELAY);
				 	if(p1PrevMoveType != 3) {
						 p1PrevMoveType = 4;
				 	}
			  } else {
				  char type[] = "IDLE MOVEMENT";
				  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
				  	HAL_UART_Transmit(&huart3, (uint8_t*)type, strlen(type), HAL_MAX_DELAY);
			  }
			  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
			  	  HAL_UART_Transmit(&huart3, (uint8_t*)line, strlen(line), HAL_MAX_DELAY);

			  p1MoveDelay = HAL_GetTick() + playerControlDelay;
		  }

		  //Draw Logic
		  if(HAL_GetTick() - currentMoveDelay >= 0) {
			  moveP1AutoMove(p1PrevMoveType);
			  currentMoveDelay = HAL_GetTick() + playerMoveDelay;
		  }

		  for(int i = 0 ; i < p1Length ; i++) {
			  ILI9341_Draw_Rectangle(10*p1X[i], 10*p1Y[i], 10, 10, 0);
		  }

		  HAL_Delay(200);
	  }


  }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 200;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
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
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
