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
uint32_t VR[4];

/*Game Control*/
//uint8_t spawnPosX[10] = {10,11,12,13,13,13,14,15,16,17};
//uint8_t spawnPosY[10] = {10,10,10,10,11,12,12,12,12,12};
uint8_t spawnPosX[5] = {11,11,11,11,11};
uint8_t spawnPosY[5] = {10,11,12,13,14};
//uint8_t spawnP2PosX[10] = {8,9,10};
//uint8_t spawnP2PosY[10] = {8,8,8};
uint8_t spawnP2PosX[5] = {19,19,19,19,19};
uint8_t spawnP2PosY[5] = {10,11,12,13,14};

bool isGameStart = false;
uint32_t playerP1MoveDelay = 250;
uint32_t playerP2MoveDelay = 250;
uint8_t pointLoc[2] = {0,0};
uint32_t p1Score = 0;
uint32_t p2Score = 0;
uint8_t timer = 180; //In secs
uint8_t gameState = 0; //0 = Title Page, 1 = Setting Mode Page, 2 = Game Start Page , 3 = Game End Page
uint8_t gameDiffType = 1; //1 = Easy 2 = Normal 3 = Hard
uint8_t playerAmount = 1;
uint8_t p1MoveControlType = 0; //0 = IDLE 1 = LEFT 2 = RIGHT 3 = UP 4 = DOWN
uint8_t p2MoveControlType = 0;
uint8_t mainMenuSelectType = 0; //0 = Difficult 1 = Players, 2 =  Start
uint32_t currentStateDelay = 150;
uint32_t currentGameoverStateDelay = 300;
uint32_t playerMoveDelay = 100;
uint8_t gameLoseType = 0; //1 = P1 Lost , 2 = P2 Lose

bool isButtonPress = false;
uint32_t buttonDebouce = 0;
bool isDrawBorderGame1 = false;
bool isDrawBorderGame2 = false;
bool isDrawBorderMain = false;
uint32_t pointsTimer = 15000;
uint32_t currentPointsTimer = 0;

uint32_t p1Color = 63680;
uint32_t p2Color = 31;

bool isPointSpawn = false;
bool isPause = false;
bool isStopBGSound = false;
//bool isCalculateDone = true;
//bool isAddSnakeDone = true;

uint32_t lastP1MoveTime = 0;
uint32_t lastP2MoveTime = 0;
uint32_t lastControlTime = 0;
uint32_t lastTimerCounter = 0;

bool checkIsgameoverBG = false;

/*Player Control*/
//uint8_t p1X[500] = {10,11,12,13,13,13,14,15,16,17};
//uint8_t p1Y[500] = {10,10,10,10,11,12,12,12,12,12};
uint8_t p1X[500] = {11,11,11,11,11};
uint8_t p1Y[500] = {10,11,12,13,14};
uint32_t textColor[8] = {12058,25859,9466,49459,12532,59613,15794,46211};
uint8_t p1Length = 5;
uint8_t p1TempPrevMoveType = 1; //0 = IDLE 1 = LEFT 2 = RIGHT 3 = UP 4 = DOWN

uint8_t p2X[500] = {19,19,19,19,19};
uint8_t p2Y[500] = {10,11,12,13,14};
uint8_t p2Length = 5;
uint8_t p2TempPrevMoveType = 2;

uint32_t currentGlobalTimer = 0;
uint32_t currentGlobalIndex = 0;

bool isAddP1Snake = false;
bool isAddP2Snake = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

bool isSnakeBitePoint(uint8_t type) {
	if(p1X[0] == pointLoc[0] && p1Y[0] == pointLoc[1] && type == 1) {
		//isAddSnakeDone = false;
		//HAL_UART_Transmit(&huart3, "COME ADD POINT\n\r", strlen("COME ADD POINT\n\r"), HAL_MAX_DELAY);
		addSnakeLength(1);
		randomPointLoc(false);
		currentPointsTimer = HAL_GetTick();
		p1Score += 1;
		if(!isStopBGSound) {
			Send_cmd(0x0D, 0, 0);
			isStopBGSound = true;
		}
		Send_cmd(0x03, 0x00, 3);
		return true;
	} else if(p2X[0] == pointLoc[0] && p2Y[0] == pointLoc[1] && playerAmount == 2 && type ==2) {
		//isAddSnakeDone = false;
		//HAL_UART_Transmit(&huart3, "COME ADD POINT\n\r", strlen("COME ADD POINT\n\r"), HAL_MAX_DELAY);
		addSnakeLength(2);
		randomPointLoc(false);
		currentPointsTimer = HAL_GetTick();
		p2Score += 1;
		if(!isStopBGSound) {
			Send_cmd(0x0D, 0, 0);
			isStopBGSound = true;
		}
		Send_cmd(0x03, 0x00, 3);
		return true;
	}
	return false;
}

void addSnakeLength(uint8_t addType) {

	if(addType == 1) {
		p1Length = p1Length + 1;
		p1X[p1Length-1] = p1X[p1Length-2]; //Add last index as previous last index
		p1Y[p1Length-1] = p1Y[p1Length-2];
		isAddP1Snake = true;
		//isCalculateDone = false;
	    moveP1AutoMove(p1TempPrevMoveType,true);
	} else {
		p2Length = p2Length + 1;
		p2X[p2Length-1] = p1X[p2Length-2]; //Add last index as previous last index
		p2Y[p2Length-1] = p1Y[p2Length-2];
		isAddP2Snake = true;
		moveP2AutoMove(p2TempPrevMoveType,true);
	}
	//char hexStringX[8];
	//snprintf(hexStringX, sizeof(hexStringX), " %d", p1Length);
	//while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET) {}
	//	HAL_UART_Transmit(&huart3, (uint8_t*)hexStringX, strlen(hexStringX), HAL_MAX_DELAY);
}

void gameOverInit() {
	ILI9341_Fill_Screen(BLACK);
	gameState = 3;
	ILI9341_Draw_Text("GAME OVER", 50, 50,WHITE, 4, BLACK);
	Send_cmd(0x03, 0x00, 5);
	//Send_cmd(0x08, 0x00, 0);
	//Send_cmd(0x03, 0x00, 1);

	if(playerAmount == 2) {

		char p1ScoreBuffer[20];
		sprintf(p1ScoreBuffer, "P1: %03d", p1Score);
		ILI9341_Draw_Text(p1ScoreBuffer, 50, 130, WHITE, 2, BLACK);

		char p2ScoreBuffer[20];
		sprintf(p2ScoreBuffer, "P2: %03d", p2Score);
		ILI9341_Draw_Text(p2ScoreBuffer, 175, 130, WHITE, 2, BLACK);

		char result[20];
		uint8_t xPlus = 0;
	    if (gameLoseType == 1) {
	        sprintf(result, "P2 WIN!");
	    } else if (gameLoseType == 2) {
	        sprintf(result, "P1 WIN!");
	    } else {
	        sprintf(result, "DRAW");
	        xPlus = 15;
	    }
		ILI9341_Draw_Text(result, 115+xPlus, 90, WHITE, 2, BLACK);
	} else {
		char p1ScoreBuffer[20];
		sprintf(p1ScoreBuffer, "P1: %03d", p1Score);
		ILI9341_Draw_Text(p1ScoreBuffer, 115, 120, WHITE, 2, BLACK);
	}

}

void randomPointLoc(bool isOutofTime) {

    //char hexStringX[8];
	uint8_t randX[600]; //Bad case 30 x 20
	uint8_t randY[600]; //Bad case 30 x 20
	uint8_t index = 0;
	if(isOutofTime) {
		ILI9341_Draw_Rectangle(10*pointLoc[0]+2, 10*pointLoc[1]+2, 8, 8, WHITE);
	}
	for(int i = 6 ; i < 18 ; i++) { //y
		for(int j = 8 ; j < 22 ; j++) { //x
			bool isFound = false;
			for(int k = 0 ; k < p1Length ; k++) {
				if(j == p1X[k] && i == p1Y[k]) {
					isFound = true;
					break;
				}
			}
			if(playerAmount == 2) {
				for(int k = 0 ; k < p2Length ; k++) {
					if(j == p2X[k] && i == p2Y[k]) {
						isFound = true;
						break;
					}
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
	ILI9341_Draw_Rectangle(10*pointLoc[0]+2, 10*pointLoc[1]+2, 8, 8, 2019);
	isPointSpawn = true;

}

void resetP2Position() {

	p2Length = 5;
	p2TempPrevMoveType = 2;
	for(int i = 0 ; i < p2Length ; i++) {
		p2X[i] = spawnP2PosX[i];
	}

	for(int i = 0 ; i < p2Length ; i++) {
		p2Y[i] = spawnP2PosY[i];
	}

	for (int i = 5; i < 100; i++) {
	      p2X[i] = 0;
	      p2Y[i] = 0;
	}

	for(int i = 0 ; i < p2Length ; i++) {
		ILI9341_Draw_Rectangle(10*p2X[i], 10*p2Y[i], 10, 10, p2Color);
	}
}

void resetP1Position() {

	p1Length = 5;
	p1TempPrevMoveType = 1;
	for(int i = 0 ; i < p1Length ; i++) {
		p1X[i] = spawnPosX[i];
	}

	for(int i = 0 ; i < p1Length ; i++) {
		p1Y[i] = spawnPosY[i];
	}

	for (int i = 5; i < 100; i++) {
	      p1X[i] = 0;
	      p1Y[i] = 0;
	}

	for(int i = 0 ; i < p1Length ; i++) {
		ILI9341_Draw_Rectangle(10*p1X[i], 10*p1Y[i], 10, 10, p1Color);
	}
}

void gameOver(uint32_t index) {
	 //char gameOverText[20];
	 //sprintf(gameOverText, "Score %03d", p1Score);
	drawBorder(textColor[index]);

	ILI9341_Draw_Text("PRESS BUTTON", 80, 170,textColor[index], 2, BLACK);
	ILI9341_Draw_Text("TO GO MAIN MENU", 60, 195,textColor[index], 2, BLACK);
}

void gameInit() {
	ILI9341_Fill_Screen(BLACK);
	ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
}

void resetMap() {
	p1Score = 0;
	p2Score = 0;
	isGameStart = false;
	//isCalculateDone = true;
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

	//Reset Score Text
	ILI9341_Draw_Rectangle(20, 7, 200, 20, WHITE);

	uint32_t timerX = 150;
	uint32_t timerY = 7;
	if(playerAmount == 1) {
		char p1ScoreBuffer[20];
		sprintf(p1ScoreBuffer, "Score %03d", p1Score);
		ILI9341_Draw_Text(p1ScoreBuffer, 20, 7, textColor[timer%8], 2, WHITE);
	} else {
		timerX = 110;
		timerY = 7;
	}

	char timerBuffer[20];
	sprintf(timerBuffer, "Timer %03d", timer);
	ILI9341_Draw_Text(timerBuffer, timerX, timerY, textColor[timer%8], 2, WHITE);
	resetP1Position();
	if(playerAmount == 2) {
		resetP2Position();
	}
	randomPointLoc(false);
	isGameStart = true;
}

bool checkIsP1BiteSelf() {
	for(int i = 1 ; i < p1Length ; i++) {
		if(p1X[i] == p1X[0] && p1Y[i] == p1Y[0]) {
				//resetMap();
				gameOverInit();
				gameLoseType = 1;
				return true;
			}
		}
	return false;
}

bool checkIsP2BiteSelf() {
	for(int i = 1 ; i < p2Length ; i++) {
		if(p2X[i] == p2X[0] && p2Y[i] == p2Y[0]) {
				gameOverInit();
				gameLoseType = 2;
				return true;
			}
		}
	return false;
}

bool checkIsBiteOther(uint8_t type) {
	if(type == 1) { //Check P2
		for(int i = 0 ; i < p2Length ; i++) {
			if(p2X[i] == p1X[0] && p2Y[i] == p1Y[0]) {
					gameOverInit();
					gameLoseType = 1;
					return true;
			}
		}
		return false;
	} else {
		for(int i = 0 ; i < p1Length ; i++) {
			if(p1X[i] == p2X[0] && p1Y[i] == p2Y[0]) {
				gameOverInit();
				gameLoseType = 2;
				return true;
			}
		}
		return false;
	}
}

void moveP2AutoMove(uint8_t moveType,bool isAdd) {
	HAL_UART_Transmit(&huart3, "P2 AUTO MOVE\n\r", strlen("P2 AUTO MOVE\n\r"),HAL_MAX_DELAY);
	uint32_t tempp2X[p2Length];
	uint32_t tempp2Y[p2Length];
	bool isMove = false;
	if(moveType == 1) {
		if(p2X[0] > 1) {
			tempp2X[0] = p2X[0]-1;
			isMove = true;
		} else {
			gameOverInit();
			gameLoseType = 2;
			return;
		}
		tempp2Y[0] = p2Y[0];
	} else if(moveType == 2) {
		if(p2X[0] < 30) {
			tempp2X[0] = p2X[0]+1;
			isMove = true;
		} else {
			gameOverInit();
			gameLoseType = 2;
			return;
		}
		tempp2Y[0] = p2Y[0];
	} else if(moveType == 3) {
		if(p2Y[0] > 4) {
			tempp2Y[0] = p2Y[0]-1;
			isMove = true;
		} else {
			//tempp1Y[0] = p1Y[0];
			gameOverInit();
			gameLoseType = 2;
			return;
		}
		tempp2X[0] = p2X[0];
	} else if(moveType == 4) {
		if(p2Y[0] < 22) {
			tempp2Y[0] = p2Y[0]+1;
			isMove = true;
		} else {
			//tempp1Y[0] = p1Y[0];
			gameOverInit();
			gameLoseType = 2;
			return;
		}
		tempp2X[0] = p2X[0];
	}
	if(isAdd && gameState != 3) {
		lastP2MoveTime =  HAL_GetTick();
	} else {
		if(isSnakeBitePoint(2)) {
			checkBiteSelfAfterGetPoint(tempp2X[0],tempp2Y[0],2);
			checkBiteOtherAfterGetPoint(tempp2X[0],tempp2Y[0],2);
			return;
		}
	}
	if(isMove && gameState != 3) {
		if(!isAddP2Snake) {
			ILI9341_Draw_Rectangle(10*p2X[p2Length-1], 10*p2Y[p2Length-1], 10, 10, 65535); //Clear last index
		} else {
			isAddP2Snake = false;
		}

		for(int i = 0 ; i < p2Length-1 ; i++) {
			tempp2X[i+1] = p2X[i];
			tempp2Y[i+1] = p2Y[i];
		}
		for(int i = 0 ; i < p2Length ; i++) {
			p2X[i] = tempp2X[i];
			p2Y[i] = tempp2Y[i];
		}
		if(!isAdd) {
			checkIsP2BiteSelf();
			if(playerAmount == 2) {
				checkIsBiteOther(2);
			}
		}

	}

}

void checkBiteSelfAfterGetPoint(uint32_t indexX,uint32_t indexY,uint8_t type) {
	if(type == 1) {
		for(int i = 1 ; i < p1Length ; i++) {
			if(p1X[i] == indexX && p1Y[i] == indexY) {
				gameOverInit();
				gameLoseType = 1;
				return;
			}
		}
	} else {
		for(int i = 1 ; i < p2Length ; i++) {
			if(p2X[i] == indexX && p2Y[i] == indexY) {
				gameOverInit();
				gameLoseType = 2;
				return;
			}
		}
	}
}

void checkBiteOtherAfterGetPoint(uint32_t indexX,uint32_t indexY,uint8_t type) {
	if(type == 1) {
		for(int i = 0 ; i < p2Length ; i++) {
			if(p2X[i] == indexX && p2Y[i] == indexY) {
				gameOverInit();
				gameLoseType = 1;
				return;
			}
		}
	} else {
		for(int i = 0; i < p1Length ; i++) {
			if(p1X[i] == indexX && p1Y[i] == indexY) {
				gameOverInit();
				gameLoseType = 2;
				return;
			}
		}
	}
}


void moveP1AutoMove(uint8_t moveType,bool isAdd) {
	uint32_t tempp1X[p1Length];
	uint32_t tempp1Y[p1Length];
	bool isMove = false;
	if(moveType == 1) {
		if(p1X[0] > 1) {
			tempp1X[0] = p1X[0]-1;
			isMove = true;
		} else {
			gameOverInit();
			gameLoseType = 1;
			return;
		}
		tempp1Y[0] = p1Y[0];
	} else if(moveType == 2) {
		if(p1X[0] < 30) {
			tempp1X[0] = p1X[0]+1;
			isMove = true;
		} else {
			gameOverInit();
			gameLoseType = 1;
			return;
		}
		tempp1Y[0] = p1Y[0];
	} else if(moveType == 3) {
		if(p1Y[0] > 4) {
			tempp1Y[0] = p1Y[0]-1;
			isMove = true;
		} else {
			//tempp1Y[0] = p1Y[0];
			gameOverInit();
			gameLoseType = 1;
			return;
		}
		tempp1X[0] = p1X[0];
	} else if(moveType == 4) {
		if(p1Y[0] < 22) {
			tempp1Y[0] = p1Y[0]+1;
			isMove = true;
		} else {
			//tempp1Y[0] = p1Y[0];
			gameOverInit();
			gameLoseType = 1;
			return;
		}
		tempp1X[0] = p1X[0];
	}
	if(isAdd && gameState != 3) {
		lastP1MoveTime =  HAL_GetTick();
	} else {
		if(isSnakeBitePoint(1)) {
			checkBiteSelfAfterGetPoint(tempp1X[0],tempp1Y[0],1);
			checkBiteOtherAfterGetPoint(tempp1X[0],tempp1Y[0],1);
			return;
		}
	}
	if(isMove && gameState != 3) {
		if(!isAddP1Snake) {
			ILI9341_Draw_Rectangle(10*p1X[p1Length-1], 10*p1Y[p1Length-1], 10, 10, 65535); //Clear last index
		} else {
			isAddP1Snake = false;
		}

		for(int i = 0 ; i < p1Length-1 ; i++) {
			tempp1X[i+1] = p1X[i];
			tempp1Y[i+1] = p1Y[i];
		}
		for(int i = 0 ; i < p1Length ; i++) {
			p1X[i] = tempp1X[i];
			p1Y[i] = tempp1Y[i];
		}
		if(!isAdd) {
			checkIsP1BiteSelf();
			if(playerAmount == 2) {
				checkIsBiteOther(1);
			}
		}

	}

}

void gameMainMenu(uint32_t index) {
	drawBorder(textColor[index]);
	ILI9341_Draw_Text("PRESS TO START", 75, 160, textColor[index], 2, BLACK);
	//HAL_Delay(10);
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

void drawMenuText() {
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
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */
  ILI9341_Init();//initial driver setup to drive ili9341
  HAL_ADC_Start_DMA(&hadc1, VR, 4);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  srand(HAL_RNG_GetRandomNumber(&hrng));
  int scaledValueX = VR[0];
  int scaledValueY = VR[1];
  int scaledValueXP2 = VR[2];
  int scaledValueYP2 = VR[3];
  uint32_t controlMoveDelay = 0;
  uint32_t lastGameoverUpdate = 0;
  uint32_t currentP1MoveDelay = 0;
  uint32_t currentP2MoveDelay = 0;
  gameInit();

  //drawDifficultText(12,0,65535);
  //drawPlayersText(12,0,65535);
  //drawStartText(-5,0,65535);
  drawMenuText();
  DF_Init(30);
  //resetMap();
  //ILI9341_Fill_Screen(BLACK);
  //ILI9341_Draw_Text("GAME OVER", 50, 50,WHITE, 4, BLACK);
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

	  if(HAL_GetTick() - currentGlobalTimer >= 100) {
		  currentGlobalTimer = HAL_GetTick();
		  currentGlobalIndex += 1;
		  if(currentGlobalIndex == 8) {
			  currentGlobalIndex = 0;
		  }
	  }

	  uint32_t currentTime = HAL_GetTick();
	  scaledValueX = VR[0];
	  scaledValueY = VR[1];
	  scaledValueXP2 = VR[2];
	  scaledValueYP2 = VR[3];

	  //snprintf(hexStringX, sizeof(hexStringX), "%d", scaledValueXP2);
	  //snprintf(hexStringY, sizeof(hexStringY), " %d", scaledValueYP2);


	  if(currentTime-controlMoveDelay >= currentStateDelay) {
		  controlMoveDelay = HAL_GetTick();
		  if(scaledValueX < 300) {
			  //Up
			  p1MoveControlType = 3;
		  } else if(scaledValueX > 3000) {
			  //Down
			  p1MoveControlType = 4;
		  } else if(scaledValueY > 3000) {
			  //Left
			  p1MoveControlType = 1;
		  } else if(scaledValueY < 1000) {
			  //Right
			  p1MoveControlType = 2;
		  } else {
			  //Idle
			  p1MoveControlType = 0;
		  }

		  if(scaledValueXP2 < 300) {
			  //Up
			  p2MoveControlType = 3;
		  } else if(scaledValueXP2 > 3000) {
			  //Down
			  p2MoveControlType = 4;
		  } else if(scaledValueYP2 > 3000) {
			  //Left
			  p2MoveControlType = 1;
		  } else if(scaledValueYP2 < 1000) {
			  //Right
			  p2MoveControlType = 2;
		  } else {
			  //Idle
			  p2MoveControlType = 0;
		  }
		  if(gameState == 1) {
		  	if(p1MoveControlType == 1) {
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
		  	} else if(p1MoveControlType == 2) {
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
		  	} else if(p1MoveControlType == 3) {
		  		if(mainMenuSelectType == 0) {
		  			mainMenuSelectType = 2;
		  		} else {
		  			mainMenuSelectType--;
		  		}
		  	} else if(p1MoveControlType == 4) {
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
			  //Movement Logic
			  if(!isPause) {
				  uint8_t tempMoveType = p1MoveControlType;
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

				  if(p1TempPrevMoveType != tempMoveType && canMove && HAL_GetTick() - currentP1MoveDelay >= playerMoveDelay && gameState != 3) {
					  moveP1AutoMove(tempMoveType,false);
					  currentP1MoveDelay = HAL_GetTick();
					  lastP1MoveTime =  HAL_GetTick();
					  ILI9341_Draw_Rectangle(10*p1X[0], 10*p1Y[0], 10, 10, p1Color);
					  p1TempPrevMoveType = tempMoveType;
				  }

				  if(playerAmount == 2) {
					  uint8_t tempP2MoveType = p2MoveControlType;
					  bool canP2Move = true;
					  if((tempP2MoveType == 2 && p2TempPrevMoveType == 1)) { //Move Right If current is left --> not change!
						  canP2Move = false;
					  } else if((tempP2MoveType == 1 && p2TempPrevMoveType == 2)) { //Move Left If current is right --> not change!
						  canP2Move = false;
					  } else if((tempP2MoveType == 3 && p2TempPrevMoveType == 4)) { //Move Up If current is down --> not change!
						  canP2Move = false;
					  } else if((tempP2MoveType == 4 && p2TempPrevMoveType == 3)) { //Move Down If current is up --> not change!
						  canP2Move = false;
					  }

					  if(tempP2MoveType == 0) {
						  tempP2MoveType = p2TempPrevMoveType;
					  }

					  if(p2TempPrevMoveType != tempP2MoveType && canP2Move &&  HAL_GetTick() - currentP2MoveDelay >= playerMoveDelay && gameState != 3) {
						  moveP2AutoMove(tempP2MoveType,false);
						  lastP2MoveTime =  HAL_GetTick();
						  currentP2MoveDelay = HAL_GetTick();
						  ILI9341_Draw_Rectangle(10*p2X[0], 10*p2Y[0], 10, 10, p2Color);
						  p2TempPrevMoveType = tempP2MoveType;
					  }
				  }
			  }
		  }

	  }

	  if(gameState == 0) {
		  if(!isDrawBorderMain) {
			  ILI9341_Fill_Screen(BLACK);
			  isDrawBorderMain = true;
			  drawMenuText();
			  Send_cmd(0x03, 0x00, 2);
			  Send_cmd(0x08, 0x00, 0);
			  //Send_cmd(0x08, 0x00, 0);
		  }
		  gameMainMenu(currentGlobalIndex);
		  //HAL_UART_Transmit(&huart3,(uint8_t *)"Test2",sizeof("Test2"),100);
	  } else if(gameState == 1) {
		  	 if(!isDrawBorderGame1) {
		  		ILI9341_Fill_Screen(BLACK);
		  		isDrawBorderGame1 = true;
		  	 }
			drawBorder(textColor[HAL_GetTick()%8]);
	  } else if(gameState == 2) {
		  if(!isDrawBorderGame2) {
			  resetMap();
			  Send_cmd(0x0E, 0, 0);
			  isDrawBorderGame2 = true;
		  }

		  if(!isPause) {
			  if(HAL_GetTick() - currentPointsTimer >= pointsTimer-(gameDiffType*2000)) {
				  currentPointsTimer = HAL_GetTick();
				  randomPointLoc(true);
			  }
			  currentStateDelay = 10;
			  if(HAL_GetTick() - lastTimerCounter >= 1000) {
				  timer--;
				  lastTimerCounter = HAL_GetTick();

				  uint32_t timerX = 150;
				  uint32_t timerY = 7;
				  if(playerAmount == 1) {
					char p1ScoreBuffer[20];
					sprintf(p1ScoreBuffer, "Score %03d", p1Score);
					ILI9341_Draw_Text(p1ScoreBuffer, 20, 7, textColor[timer%8], 2, WHITE);
				  } else {
					timerX = 110;
					timerY = 7;
				  }
				  char timerBuffer[20];
				  sprintf(timerBuffer, "Timer %03d", timer);
				  ILI9341_Draw_Text(timerBuffer, timerX, timerY, textColor[timer%8], 2, WHITE);

				  if(timer == 0) {
					  gameOverInit();

					  if(p1Score > p2Score) {
						  gameLoseType = 2;
					  } else if(p2Score > p1Score) {
						  gameLoseType = 1;
					  } else {
						  gameLoseType = 0;
					  }
				  }
			  }
			  if(isGameStart) {
				  //Draw Logic
				  if (HAL_GetTick() - lastP1MoveTime >= (playerP1MoveDelay-((gameDiffType-1)*50))) { //isCalculateDone
					 // isCalculateDone = false;
					  lastP1MoveTime =  HAL_GetTick();
					  moveP1AutoMove(p1TempPrevMoveType,false);
				      ILI9341_Draw_Rectangle(10*p1X[0], 10*p1Y[0], 10, 10, p1Color);
				  }
				  if(playerAmount == 2) {
					  if (HAL_GetTick() - lastP2MoveTime >= (playerP2MoveDelay-((gameDiffType-1)*50))) { //isCalculateDone
						 // isCalculateDone = false;
						  lastP2MoveTime =  HAL_GetTick();
						  moveP2AutoMove(p2TempPrevMoveType,false);
					      ILI9341_Draw_Rectangle(10*p2X[0], 10*p2Y[0], 10, 10, p2Color);
					  }
				  }

			  }
		  }
	  } else if(gameState == 3) {
		  if(HAL_GetTick() - lastGameoverUpdate >= currentGameoverStateDelay) {
			  if(!checkIsgameoverBG) {
				  gameOverInit();
				  checkIsgameoverBG = true;
			  }
			  gameOver(currentGlobalIndex);
			  //
			  //HAL_UART_Transmit(&huart3,(uint8_t *)"Test",sizeof("Test"),100);
			  lastGameoverUpdate =  HAL_GetTick();
		  }
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
  RCC_OscInitStruct.PLL.PLLN = 216;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
		if(GPIO_Pin == GPIO_PIN_1)
		{
			if(!isButtonPress && HAL_GetTick() - buttonDebouce >= 1000) {
				buttonDebouce = HAL_GetTick();
				if(gameState == 3) {
					ILI9341_Fill_Screen(BLACK);
					gameState = 0;
					checkIsgameoverBG = false;
					isDrawBorderGame1 = false;
					isDrawBorderGame2 = false;
					isDrawBorderMain = false;
					isStopBGSound = false;
					currentStateDelay = 150;
					currentPointsTimer = 0;
					timer = 180;
					HAL_UART_Transmit(&huart3,(uint8_t *)"Test",sizeof("Test"),100);
				} else if(gameState == 0) {
					gameState = 2;
					gameDiffType = 1;
					playerAmount = 1;
					gameState = 1;
					ILI9341_Fill_Screen(BLACK);
				} else if(gameState == 1 && mainMenuSelectType == 2) {
					resetMap();
					gameState = 2;
				} else if(gameState == 2) { //Pause & Resume
					isPause = !isPause;
					if(!isPause) {
						currentPointsTimer = HAL_GetTick();
					}
				}
			}
		}
	}
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
