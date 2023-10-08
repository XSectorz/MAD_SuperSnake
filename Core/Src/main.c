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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "ILI9341_Touchscreen.h"
#include "time.h"
#include "stdbool.h"
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
uint32_t playerControlDelay = 300;//In millis
uint32_t playerMoveDelay = 250;
uint8_t pointLoc[2] = {0,0};
uint32_t p1Score = 0;
uint8_t timer = 100; //In secs
uint8_t gameState = 0; //0 = Title Page, 1 = Setting Mode Page, 2 = Game Start Page , 3 = Game End Page
uint8_t gameDiffType = 3; //1 = Easy 2 = Normal 3 = Hard
uint8_t playerAmount = 1;
uint8_t moveControlType = 0; //0 = IDLE 1 = LEFT 2 = RIGHT 3 = UP 4 = DOWN
uint8_t mainMenuSelectType = 0; //0 = Difficult 1 = Players, 2 =  Start
uint32_t currentStateDelay = 1000;

bool isPointSpawn = false;
bool isCalculateDone = true;
bool isAddSnakeDone = true;

uint32_t lastMoveTime = 0;
uint32_t lastControlTime = 0;
uint32_t lastTimerCounter = 0;

/*Player Control*/
uint8_t p1X[100] = {10,11,12,13,13,13,14,15,16,17};
uint8_t p1Y[100] = {10,10,10,10,11,12,12,12,12,12};
uint32_t textColor[8] = {12058,25859,9466,49459,12532,59613,15794,46211};
uint8_t p1Length = 10;
uint8_t p1TempPrevMoveType = 1; //0 = IDLE 1 = LEFT 2 = RIGHT 3 = UP 4 = DOWN
uint32_t p1MoveDelay = 0;
bool isAddSnake = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void isSnakeBitePoint() {
	if(p1X[0] == pointLoc[0] && p1Y[0] == pointLoc[1]) {
		isAddSnakeDone = false;
		addSnakeLength();
		randomPointLoc();
	}
}

void addSnakeLength() {

	if(p1Length < 17) {
		p1Length = p1Length + 1;
		p1X[p1Length-1] = p1X[p1Length-2]; //Add last index as previous last index
		p1Y[p1Length-1] = p1Y[p1Length-2];
		isAddSnake = true;
		isAddSnakeDone = true;
		lastControlTime = HAL_GetTick();

		char line[] = "\n\r";
		char stri[] = "ADD LENGTH --> ";
			while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
				HAL_UART_Transmit(&huart3, (uint8_t*)stri, strlen(stri), HAL_MAX_DELAY);
		char hexStringX[8];
			snprintf(hexStringX, sizeof(hexStringX), " %d",  p1X[0]);
			while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
				HAL_UART_Transmit(&huart3, (uint8_t*)hexStringX, strlen(hexStringX), HAL_MAX_DELAY);
		char hexStringY[8];
			snprintf(hexStringY, sizeof(hexStringY), " %d",  p1Y[0]);
			while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
				HAL_UART_Transmit(&huart3, (uint8_t*)hexStringY, strlen(hexStringY), HAL_MAX_DELAY);

			while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
				HAL_UART_Transmit(&huart3, (uint8_t*)line, strlen(line), HAL_MAX_DELAY);
	}
	//char hexStringX[8];
	//snprintf(hexStringX, sizeof(hexStringX), " %d", p1Length);
	//while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
	//	HAL_UART_Transmit(&huart3, (uint8_t*)hexStringX, strlen(hexStringX), HAL_MAX_DELAY);
}

void randomPointLoc() {

    //char hexStringX[8];
	uint8_t randX[600]; //Bad case 30 x 20
	uint8_t randY[600]; //Bad case 30 x 20
	uint8_t index = 0;

	for(int i = 8 ; i < 16 ; i++) { //y
		for(int j = 10 ; j < 20 ; j++) { //x
			bool isFound = false;
			for(int k = 0 ; k < p1Length ; k++) {
				if(j == p1X[k] && i == p1Y[k]) {
					isFound = true;
					break;
				}
			}
			if(!isFound) {
				randX[index] = j;
				randY[index] = i;
				index++;
			}
		}
	}


	uint32_t num = (rand() % ((index-1) - 0 + 1)) + 0;
	//snprintf(hexStringX, sizeof(hexStringX), " %d", num);
	//while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
	//	HAL_UART_Transmit(&huart3, (uint8_t*)hexStringX, strlen(hexStringX), HAL_MAX_DELAY);

	pointLoc[0] = randX[num];
	pointLoc[1] = randY[num];
	ILI9341_Draw_Rectangle(10*pointLoc[0]+2, 10*pointLoc[1]+2, 8, 8, 59430);
	isPointSpawn = true;

}

void resetP1Position() {

	p1Length = 10;
	p1TempPrevMoveType = 1;
	for(int i = 0 ; i < p1Length ; i++) {
		p1X[i] = spawnPosX[i];
	}

	for(int i = 0 ; i < p1Length ; i++) {
		p1Y[i] = spawnPosY[i];
	}

	for (int i = 10; i < 100; i++) {
	      p1X[i] = 0;
	      p1Y[i] = 0;
	}

	for(int i = 0 ; i < p1Length ; i++) {
		ILI9341_Draw_Rectangle(10*p1X[i], 10*p1Y[i], 10, 10, 0);
	}
}

void gameInit() {
	ILI9341_Fill_Screen(BLACK);
	ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
}

void resetMap() {
	isGameStart = false;
	isCalculateDone = true;
	ILI9341_Fill_Screen(WHITE);
	ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
	for(int i = 3 ; i < 24 ; i++) {
		  if(i==3 || i == 23) {
			  for(int j = 0 ; j < 32 ; j++) {
				  ILI9341_Draw_Rectangle(10*j, 10*i, 10, 10, 30000);
			  }
		  } else {
			  ILI9341_Draw_Rectangle(0, 10*i, 10, 10, 30000);
			  ILI9341_Draw_Rectangle(310, 10*i, 10, 10, 30000);
		  }
	  }
	resetP1Position();
	randomPointLoc();
	isGameStart = true;
}

bool checkIsP1BiteSelf() {
	for(int i = 1 ; i < p1Length ; i++) {
		if(p1X[i] == p1X[0] && p1Y[i] == p1Y[0]) {
			char line[] = "\n\r";
			char stri[] = "BITE SELF --> ";
				while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
					HAL_UART_Transmit(&huart3, (uint8_t*)stri, strlen(stri), HAL_MAX_DELAY);
			char hexStringX[8];
				snprintf(hexStringX, sizeof(hexStringX), " %d",  p1X[0]);
				while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
					HAL_UART_Transmit(&huart3, (uint8_t*)hexStringX, strlen(hexStringX), HAL_MAX_DELAY);
			char hexStringY[8];
				snprintf(hexStringY, sizeof(hexStringY), " %d",  p1Y[0]);
				while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
					HAL_UART_Transmit(&huart3, (uint8_t*)hexStringY, strlen(hexStringY), HAL_MAX_DELAY);

				while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
					HAL_UART_Transmit(&huart3, (uint8_t*)line, strlen(line), HAL_MAX_DELAY);
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
		if(p1Y[0] > 4) {
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
	isSnakeBitePoint();
	if(isMove) {
		if(!isAddSnake) {
			ILI9341_Draw_Rectangle(10*p1X[p1Length-1], 10*p1Y[p1Length-1], 10, 10, 65535); //Clear last index
		} else {
			isAddSnake = false;
		}
		for(int i = 0 ; i < p1Length-1 ; i++) {
			tempp1X[i+1] = p1X[i];
			tempp1Y[i+1] = p1Y[i];
		}
		for(int i = 0 ; i < p1Length ; i++) {
			p1X[i] = tempp1X[i];
			p1Y[i] = tempp1Y[i];
		}
		checkIsP1BiteSelf();

	}
	isCalculateDone = true;

}

void gameMainMenu() {
	uint32_t currentTime = HAL_GetTick();
	drawBorder(textColor[currentTime%8]);
	ILI9341_Draw_Text("PRESS TO START", 75, 160, textColor[currentTime%8], 2, BLACK);
	drawMenuText();
}

void drawBorder(uint32_t color) {
	for(int i = 0 ; i < 24 ; i++) {
		  if(i==0 || i == 23) {
			  for(int j = 0 ; j < 32 ; j++) {
				  ILI9341_Draw_Rectangle(10*j, 10*i, 10, 10, color);
			  }
		  } else {
			  ILI9341_Draw_Rectangle(0, 10*i, 10, 10, color);
			  ILI9341_Draw_Rectangle(310, 10*i, 10, 10, color);
		  }
	  }
}

void drawMenuText(uint32_t textColor) {
	/* Big S */
	ILI9341_Draw_Rectangle(60, 40, 60, 10, 30000);
	ILI9341_Draw_Rectangle(45, 50, 30, 20, 30000);
	ILI9341_Draw_Rectangle(105, 50, 30, 10, 30000);
	ILI9341_Draw_Rectangle(60, 70, 60, 10, 30000);
	ILI9341_Draw_Rectangle(105, 80, 30, 20, 30000);
	ILI9341_Draw_Rectangle(45, 90, 30, 10, 30000);
	ILI9341_Draw_Rectangle(60, 100, 60, 10, 30000);

	/* U */
	ILI9341_Draw_Rectangle(140, 40, 8, 24, 65472);
	ILI9341_Draw_Rectangle(144, 64, 16, 4, 65472); //END 168 PX
	ILI9341_Draw_Rectangle(156, 40, 8, 24, 65472);

	/* P */
	ILI9341_Draw_Rectangle(168, 40, 8, 28, 65472);
	ILI9341_Draw_Rectangle(174, 40, 12, 4, 65472); //END 194 PX
	ILI9341_Draw_Rectangle(182, 44, 8, 12, 65472);
	ILI9341_Draw_Rectangle(174, 56, 12, 4, 65472);

	/* E */
	ILI9341_Draw_Rectangle(196, 40, 8, 28, 65472);
	ILI9341_Draw_Rectangle(204, 40, 16, 4, 65472); //END 222 PX
	ILI9341_Draw_Rectangle(204, 52, 12, 4, 65472);
	ILI9341_Draw_Rectangle(204, 64, 16, 4, 65472);

	/* R */
	ILI9341_Draw_Rectangle(224, 40, 8, 28, 65472);
	ILI9341_Draw_Rectangle(232, 40, 12, 4, 65472);
	ILI9341_Draw_Rectangle(240, 44, 8, 12, 65472);
	ILI9341_Draw_Rectangle(236, 52, 12, 4, 65472); //END 250 PX
	ILI9341_Draw_Rectangle(232, 56, 8, 4, 65472);
	ILI9341_Draw_Rectangle(236, 60, 8, 4, 65472);
	ILI9341_Draw_Rectangle(240, 64, 8, 4, 65472);

	/* N */
	ILI9341_Draw_Rectangle(140, 80, 8, 28, 65472);
	ILI9341_Draw_Rectangle(148, 88, 4, 8, 65472); //END 168 PX
	ILI9341_Draw_Rectangle(152, 92, 4, 8, 65472);
	ILI9341_Draw_Rectangle(156, 80, 8, 28, 65472);

	/* A */
	ILI9341_Draw_Rectangle(168, 84, 8, 24, 65472);
	ILI9341_Draw_Rectangle(172, 80, 16, 4, 65472); //END 194 PX
	ILI9341_Draw_Rectangle(184, 84, 8, 24, 65472);
	ILI9341_Draw_Rectangle(176, 96, 8, 4, 65472);

	/* K */
	ILI9341_Draw_Rectangle(196, 80, 8, 28, 65472);
	ILI9341_Draw_Rectangle(204, 88, 4, 12, 65472);
	ILI9341_Draw_Rectangle(206, 84, 4, 8, 65472);
	ILI9341_Draw_Rectangle(210, 80, 4, 8, 65472);
	ILI9341_Draw_Rectangle(214, 80, 4, 4, 65472);
	ILI9341_Draw_Rectangle(206, 96, 4, 8, 65472);
	ILI9341_Draw_Rectangle(210, 100, 4, 8, 65472);
	ILI9341_Draw_Rectangle(214, 104, 4, 4, 65472);


	/* E */
	ILI9341_Draw_Rectangle(224, 80, 8, 28, 65472);
	ILI9341_Draw_Rectangle(232, 80, 16, 4, 65472);
	ILI9341_Draw_Rectangle(232, 92, 12, 4, 65472);
	ILI9341_Draw_Rectangle(232, 104, 16, 4, 65472);

}

void drawNumberBar(uint32_t positionX,uint32_t positionY,uint32_t color,uint8_t type,bool isGlow) {

	if(isGlow) {
		color = 7972;
	}

	/* < Text */
	ILI9341_Draw_Rectangle(189+positionX, 59+positionY, 6, 3, color);
	ILI9341_Draw_Rectangle(192+positionX, 56+positionY, 6, 3, color);
	ILI9341_Draw_Rectangle(192+positionX, 62+positionY, 6, 3, color);
	ILI9341_Draw_Rectangle(195+positionX, 53+positionY, 9, 3, color);
	ILI9341_Draw_Rectangle(195+positionX, 65+positionY, 9, 3, color);
	ILI9341_Draw_Rectangle(198+positionX, 50+positionY, 9, 3, color);
	ILI9341_Draw_Rectangle(198+positionX, 68+positionY, 9, 3, color);

	if(type == 1) {
		ILI9341_Draw_Rectangle(228+positionX, 50+positionY, 6, 18, color);
		ILI9341_Draw_Rectangle(225+positionX, 53+positionY, 3, 3, color);
		ILI9341_Draw_Rectangle(225+positionX, 68+positionY, 12, 3, color);
	} else if(type == 2) {
		ILI9341_Draw_Rectangle(222+positionX, 53+positionY, 6, 3, color);
		ILI9341_Draw_Rectangle(225+positionX, 50+positionY, 12, 3, color);
		ILI9341_Draw_Rectangle(234+positionX, 53+positionY, 6, 3, color);
		ILI9341_Draw_Rectangle(231+positionX, 56+positionY, 9, 3, color);
		ILI9341_Draw_Rectangle(228+positionX, 59+positionY, 9, 3, color);
		ILI9341_Draw_Rectangle(225+positionX, 62+positionY, 6, 3, color);
		ILI9341_Draw_Rectangle(222+positionX, 65+positionY, 6, 6, color);
		ILI9341_Draw_Rectangle(228+positionX, 68+positionY, 6, 3, color);
		ILI9341_Draw_Rectangle(234+positionX, 65+positionY, 6, 6, color);
	} else if(type == 3) {
		ILI9341_Draw_Rectangle(222+positionX, 53+positionY, 6, 3, color);
		ILI9341_Draw_Rectangle(225+positionX, 50+positionY, 12, 3, color);
		ILI9341_Draw_Rectangle(234+positionX, 53+positionY, 6, 6, color);
		ILI9341_Draw_Rectangle(228+positionX, 59+positionY, 6, 3, color);
		ILI9341_Draw_Rectangle(234+positionX, 62+positionY, 6, 6, color);
		ILI9341_Draw_Rectangle(222+positionX, 65+positionY, 6, 3, color);
		ILI9341_Draw_Rectangle(225+positionX, 68+positionY, 12, 3, color);
	}

	/* > Text */
	ILI9341_Draw_Rectangle(255+positionX, 50+positionY, 9, 3, color);
	ILI9341_Draw_Rectangle(255+positionX, 68+positionY, 9, 3, color);
	ILI9341_Draw_Rectangle(258+positionX, 53+positionY, 9, 3, color);
	ILI9341_Draw_Rectangle(258+positionX, 65+positionY, 9, 3, color);
	ILI9341_Draw_Rectangle(264+positionX, 56+positionY, 6, 3, color);
	ILI9341_Draw_Rectangle(264+positionX, 62+positionY, 6, 3, color);
	ILI9341_Draw_Rectangle(267+positionX, 59+positionY, 6, 3, color);
}

void drawPlayersText(uint32_t positionX,uint32_t positionY,uint32_t color) {
	ILI9341_Draw_Rectangle(15+positionX, 95+positionY, 15, 3, color);
	ILI9341_Draw_Rectangle(15+positionX, 98+positionY, 6, 18, color);
	ILI9341_Draw_Rectangle(27+positionX, 98+positionY, 6, 9, color);
	ILI9341_Draw_Rectangle(21+positionX, 107+positionY, 9, 3, color);

	ILI9341_Draw_Rectangle(36+positionX, 95+positionY, 6, 21, color);
	ILI9341_Draw_Rectangle(42+positionX, 110+positionY, 12, 6, color);

	ILI9341_Draw_Rectangle(60+positionX, 95+positionY, 12, 3, color);
	ILI9341_Draw_Rectangle(57+positionX, 98+positionY, 6, 18, color);
	ILI9341_Draw_Rectangle(69+positionX, 98+positionY, 6, 18, color);
	ILI9341_Draw_Rectangle(63+positionX, 107+positionY, 6, 3, color);

	ILI9341_Draw_Rectangle(78+positionX, 95+positionY, 6, 9, color);
	ILI9341_Draw_Rectangle(81+positionX, 104+positionY, 12, 3, color);
	ILI9341_Draw_Rectangle(90+positionX, 95+positionY, 6, 9, color);
	ILI9341_Draw_Rectangle(84+positionX, 107+positionY, 6, 9, color);

	ILI9341_Draw_Rectangle(99+positionX, 95+positionY, 6, 21, color);
	ILI9341_Draw_Rectangle(105+positionX, 95+positionY, 12, 3, color);
	ILI9341_Draw_Rectangle(105+positionX, 104+positionY, 9, 3, color);
	ILI9341_Draw_Rectangle(105+positionX, 113+positionY, 12, 3, color);

	ILI9341_Draw_Rectangle(120+positionX, 95+positionY, 6, 21, color);
	ILI9341_Draw_Rectangle(126+positionX, 95+positionY, 9, 3, color);
	ILI9341_Draw_Rectangle(132+positionX, 98+positionY, 6, 9, color);
	ILI9341_Draw_Rectangle(129+positionX, 104+positionY, 3, 9, color);
	ILI9341_Draw_Rectangle(126+positionX, 107+positionY, 3, 3, color);
	ILI9341_Draw_Rectangle(132+positionX, 110+positionY, 3, 6, color);
	ILI9341_Draw_Rectangle(135+positionX, 113+positionY, 3, 3, color);

	ILI9341_Draw_Rectangle(141+positionX, 98+positionY, 6, 6, color);
	ILI9341_Draw_Rectangle(144+positionX, 95+positionY, 12, 3, color);
	ILI9341_Draw_Rectangle(153+positionX, 98+positionY, 6, 3, color);
	ILI9341_Draw_Rectangle(144+positionX, 104+positionY, 12, 3, color);
	ILI9341_Draw_Rectangle(153+positionX, 107+positionY, 6, 6, color);
	ILI9341_Draw_Rectangle(144+positionX, 113+positionY, 12, 3, color);
	ILI9341_Draw_Rectangle(141+positionX, 110+positionY, 6, 3, color);

	bool isGlow = false;
	if(mainMenuSelectType == 1) {
		isGlow = true;
	}

	drawNumberBar(positionX,45,color,playerAmount,isGlow);
}

void drawDifficultText(uint32_t positionX,uint32_t positionY,uint32_t color) {
	ILI9341_Draw_Rectangle(15+positionX, 50+positionY, 12, 3, color);
	ILI9341_Draw_Rectangle(15+positionX, 53+positionY, 6, 15, color);
	ILI9341_Draw_Rectangle(15+positionX, 68+positionY, 12, 3, color);
	ILI9341_Draw_Rectangle(24+positionX, 53+positionY, 6, 3, color);
	ILI9341_Draw_Rectangle(24+positionX, 65+positionY, 6, 3, color);
	ILI9341_Draw_Rectangle(27+positionX, 56+positionY, 6, 9, color);

	ILI9341_Draw_Rectangle(36+positionX, 50+positionY, 6, 21, color);

	ILI9341_Draw_Rectangle(45+positionX, 50+positionY, 6, 21, color);
	ILI9341_Draw_Rectangle(51+positionX, 50+positionY, 12, 3, color);
	ILI9341_Draw_Rectangle(51+positionX, 59+positionY, 9, 3, color);

	ILI9341_Draw_Rectangle(66+positionX, 50+positionY, 6, 21, color);
	ILI9341_Draw_Rectangle(72+positionX, 50+positionY, 12, 3, color);
	ILI9341_Draw_Rectangle(72+positionX, 59+positionY, 9, 3, color);

	ILI9341_Draw_Rectangle(87+positionX, 50+positionY, 6, 21, color);

	ILI9341_Draw_Rectangle(96+positionX, 56+positionY, 6, 9, color);
	ILI9341_Draw_Rectangle(99+positionX, 53+positionY, 6, 3, color);
	ILI9341_Draw_Rectangle(99+positionX, 65+positionY, 6, 3, color);
	ILI9341_Draw_Rectangle(102+positionX, 50+positionY, 9, 3, color);
	ILI9341_Draw_Rectangle(102+positionX, 68+positionY, 9, 3, color);
	ILI9341_Draw_Rectangle(108+positionX, 53+positionY, 6, 3, color);
	ILI9341_Draw_Rectangle(108+positionX, 65+positionY, 6, 3, color);

	ILI9341_Draw_Rectangle(117+positionX, 50+positionY, 6, 18, color);
	ILI9341_Draw_Rectangle(120+positionX, 68+positionY, 12, 3, color);
	ILI9341_Draw_Rectangle(129+positionX, 50+positionY, 6, 18, color);

	ILI9341_Draw_Rectangle(138+positionX, 50+positionY, 6, 21, color);
	ILI9341_Draw_Rectangle(144+positionX, 65+positionY, 12, 6, color);

	ILI9341_Draw_Rectangle(156+positionX, 50+positionY, 18, 3, color); //END AT 174 PX
	ILI9341_Draw_Rectangle(162+positionX, 53+positionY, 6, 18, color);

	bool isGlow = false;
	if(mainMenuSelectType == 0) {
		isGlow = true;
	}

	drawNumberBar(positionX,positionY,color,gameDiffType,isGlow);
}

void drawStartText(uint32_t positionX,uint32_t positionY,uint32_t color) {


	if(mainMenuSelectType == 2) {
		color = 7972;
	}

	ILI9341_Draw_Rectangle(100+positionX, 162+positionY, 16, 4, color);
	ILI9341_Draw_Rectangle(112+positionX, 166+positionY, 8, 4, color);
	ILI9341_Draw_Rectangle(96+positionX, 166+positionY, 8, 8, color);
	ILI9341_Draw_Rectangle(100+positionX, 172+positionY, 16, 4, color); //END 124 PX
	ILI9341_Draw_Rectangle(112+positionX, 176+positionY, 8, 8, color);
	ILI9341_Draw_Rectangle(96+positionX, 180+positionY, 8, 4, color);
	ILI9341_Draw_Rectangle(98+positionX, 184+positionY, 16, 4, color);

	ILI9341_Draw_Rectangle(124+positionX, 162+positionY, 24, 4, color); //END AT 152
	ILI9341_Draw_Rectangle(132+positionX, 166+positionY, 8, 24, color);

	ILI9341_Draw_Rectangle(152+positionX, 166+positionY, 8, 24, color);
	ILI9341_Draw_Rectangle(156+positionX, 162+positionY, 16, 4, color); //END AT 180
	ILI9341_Draw_Rectangle(168+positionX, 166+positionY, 8, 24, color);
	ILI9341_Draw_Rectangle(160+positionX, 178+positionY, 8, 4, color);

	ILI9341_Draw_Rectangle(180+positionX, 162+positionY, 8, 28, color);
	ILI9341_Draw_Rectangle(188+positionX, 162+positionY, 12, 4, color);
	ILI9341_Draw_Rectangle(196+positionX, 166+positionY, 8, 8, color);
	ILI9341_Draw_Rectangle(192+positionX, 174+positionY, 12, 4, color);
	ILI9341_Draw_Rectangle(188+positionX, 178+positionY, 8, 4, color);
	ILI9341_Draw_Rectangle(192+positionX, 182+positionY, 8, 4, color);
	ILI9341_Draw_Rectangle(196+positionX, 186+positionY, 8, 4, color);

	ILI9341_Draw_Rectangle(208+positionX, 162+positionY, 24, 4, color);
	ILI9341_Draw_Rectangle(216+positionX, 166+positionY, 8, 24, color);
}

void clearBG(uint32_t posX,uint32_t posY,uint32_t weight,uint32_t height,uint32_t color) {
	ILI9341_Draw_Rectangle(posX, posY, weight, height, color);
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
  srand(HAL_RNG_GetRandomNumber(&hrng));
  int scaledValueX = VR[0];
  int scaledValueY = VR[1];
  uint32_t controlMoveDelay = 0;
  gameInit();

  //drawDifficultText(12,0,65535);
  //drawPlayersText(12,0,65535);
  //drawStartText(-5,0,65535);


  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  char hexStringX[8];
	  char hexStringY[8];
	  char line[] = "\n\r";

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

	  uint32_t currentTime = HAL_GetTick();
	  scaledValueX = VR[0];
	  scaledValueY = VR[1];

	  snprintf(hexStringX, sizeof(hexStringX), "%d", scaledValueX);
	  snprintf(hexStringY, sizeof(hexStringY), " %d", scaledValueY);

	  if(currentTime-controlMoveDelay >= currentStateDelay) {
		  controlMoveDelay = HAL_GetTick();
		  if(scaledValueX < 300) {
			  //Up
			  moveControlType = 3;
		  } else if(scaledValueX > 3000) {
			  //Down
			  moveControlType = 4;
		  } else if(scaledValueY > 3000) {
			  //Left
			  moveControlType = 1;
		  } else if(scaledValueY < 1000) {
			  //Right
			  moveControlType = 2;
		  } else {
			  //Idle
			  moveControlType = 0;
		  }
		  if(gameState == 1) {
		  	if(moveControlType == 1) {
		  		if(mainMenuSelectType == 0) {
					clearBG(222,50,30,30,0);
			  		if(gameDiffType == 1) {
			  			gameDiffType = 3;
			  		} else {
			  			gameDiffType -= 1;
			  		}
		  		} else if(mainMenuSelectType == 1) {
		  			clearBG(222,95,30,30,0);
		  			if(playerAmount == 1) {
		  				playerAmount = 2;
		  			} else {
		  				playerAmount--;
		  			}
		  		}
		  	} else if(moveControlType == 2) {
		  		if(mainMenuSelectType == 0) {
					clearBG(222,50,30,30,0);
			  		if(gameDiffType == 3) {
			  			gameDiffType = 1;
			  		} else {
			  			gameDiffType += 1;
			  		}
		  		} else if(mainMenuSelectType == 1) {
		  			clearBG(222,95,30,30,0);
		  			if(playerAmount == 2) {
		  				playerAmount = 1;
		  			} else {
		  				playerAmount++;
		  			}
		  		}
		  	} else if(moveControlType == 3) {
		  		if(mainMenuSelectType == 0) {
		  			mainMenuSelectType = 2;
		  		} else {
		  			mainMenuSelectType--;
		  		}
		  	} else if(moveControlType == 4) {
		  		if(mainMenuSelectType == 2) {
		  			mainMenuSelectType = 0;
		  		} else {
		  			mainMenuSelectType++;
		  		}
		  	}
			drawDifficultText(12,0,65535);
			drawPlayersText(12,0,65535);
			drawStartText(-5,0,65535);
		  } else if(gameState == 2) {
			 /* snprintf(hexStringX, sizeof(hexStringX), "%d", moveControlType);
			  snprintf(hexStringY, sizeof(hexStringY), " %d", p1TempPrevMoveType);

			  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
			  HAL_UART_Transmit(&huart3, (uint8_t*)hexStringX, strlen(hexStringX), HAL_MAX_DELAY);

			  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
			  HAL_UART_Transmit(&huart3, (uint8_t*)hexStringY, strlen(hexStringY), HAL_MAX_DELAY);

			  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
			  HAL_UART_Transmit(&huart3, (uint8_t*)line, strlen(line), HAL_MAX_DELAY);*/

			  //Movement Logic
			  uint8_t tempMoveType = moveControlType;
			  bool canMove = true;
			  if((tempMoveType == 2 && p1TempPrevMoveType == 1)) { //Move Right If current is left --> not change!
				  canMove = false;
			  } else if((tempMoveType == 1 && p1TempPrevMoveType == 2)) { //Move Left If current is right --> not change!
				  canMove = false;
			  } else if((tempMoveType == 3 && p1TempPrevMoveType == 4)) { //Move Up If current is down --> not change!
				  canMove = false;
			  } else if((tempMoveType == 4 && p1TempPrevMoveType == 3)) { //Move Down If current is up --> not change!
				  canMove = false;
			  }

			  if(tempMoveType == 0) {
				  tempMoveType = p1TempPrevMoveType;
			  }

			  if(p1TempPrevMoveType != tempMoveType && canMove) {
				  moveP1AutoMove(tempMoveType);
				  lastMoveTime =  HAL_GetTick();
				  for(int i = 0 ; i < p1Length ; i++) {
					  ILI9341_Draw_Rectangle(10*p1X[i], 10*p1Y[i], 10, 10, 0);
				  }
				  p1TempPrevMoveType = tempMoveType;
			  }
		  }

	  }

	  if(gameState == 0) {
		  gameMainMenu();
	  } else if(gameState == 1) {

			drawBorder(textColor[HAL_GetTick()%8]);
	  } else if(gameState == 2) {
		  /*snprintf(hexStringX, sizeof(hexStringX), "%d", scaledValueX);
		  snprintf(hexStringY, sizeof(hexStringY), " %d", scaledValueY);

		  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
		  HAL_UART_Transmit(&huart3, (uint8_t*)hexStringX, strlen(hexStringX), HAL_MAX_DELAY);

		  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
		  HAL_UART_Transmit(&huart3, (uint8_t*)hexStringY, strlen(hexStringY), HAL_MAX_DELAY);

		  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
		  HAL_UART_Transmit(&huart3, (uint8_t*)line, strlen(line), HAL_MAX_DELAY);*/
		  //HAL_Delay(500);
		  currentStateDelay = 100;
		  if(HAL_GetTick() - lastTimerCounter >= 1000) {
			  timer--;
			  lastTimerCounter = HAL_GetTick();
			  char p1ScoreBuffer[20];
			  sprintf(p1ScoreBuffer, "Score %03d", p1Score);
			  ILI9341_Draw_Text(p1ScoreBuffer, 20, 7, textColor[timer%8], 2, WHITE);

			  char timerBuffer[20];
			  sprintf(timerBuffer, "Timer %03d", timer);
			  ILI9341_Draw_Text(timerBuffer, 150, 7, textColor[timer%8], 2, WHITE);
		  }


		  if(isGameStart) {


			  // while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
			  // 	  HAL_UART_Transmit(&huart3, (uint8_t*)line, strlen(line), HAL_MAX_DELAY);
			  //Draw Logic
			  if (HAL_GetTick() - lastMoveTime >= playerMoveDelay && isCalculateDone && isAddSnakeDone) {
				  isCalculateDone = false;
			      lastMoveTime =  HAL_GetTick();
			      moveP1AutoMove(p1TempPrevMoveType);
			      for(int i = 0 ; i < p1Length ; i++) {
			          ILI9341_Draw_Rectangle(10*p1X[i], 10*p1Y[i], 10, 10, 0);
			      }
			  }


		  }
	  } else if(gameState == 3) {

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
