/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "u8g2/u8g2.h"
#include <string.h>
#include "STM32F4xx.h"
#include <stm32f4xx_hal.h>
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint8_t ButtonState = 0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim13;
TIM_HandleTypeDef htim14;

/* Definitions for rewriteDisplay */
osThreadId_t rewriteDisplayHandle;
const osThreadAttr_t rewriteDisplay_attributes = {
  .name = "rewriteDisplay",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityRealtime7,
};
/* Definitions for buttonListen */
osThreadId_t buttonListenHandle;
const osThreadAttr_t buttonListen_attributes = {
  .name = "buttonListen",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* USER CODE BEGIN PV */
// INPUT CAPTURE VARIABLES
//int64_t IC_Val1 = 0;
//int64_t IC_Val2 = 0;
int64_t Difference = 0;
uint8_t valid_for_capturing = 2;
//uint8_t Is_First_Captured = 0;
const uint64_t TIMCLOCK = 84000000; //168000000
const uint64_t COEFFICIENT = 1000000000;
//const uint64_t translateus2Y = 1562;
///////////////////////////////////////////////////////////////////////////////////////////////////////

// METAL INDICATOR PARAMETERS
//const double min_gs_speed = 0.003; // obvious, no exact
//const double max_hss_speed = 0.009; // obvious, no exact
//const double max_gs_speed = 0.0049; // mm/ns
//const double min_hss_speed = 0.0055; // mm/ns

const double min_gs_speed = 0.0015; // obvious, no exact
const double max_gs_speed = 0.00245; // mm/ns
const double min_hss_speed = 0.00275; // mm/ns
const double max_hss_speed = 0.0045; // obvious, no exact

const uint32_t TEST_DELAY = 400;  // ns

///////////////////////////////////////////////////////////////////////////////////////////////////////

// ARRAY VARIABLES FOR CONST MEASURING LENGTH
const uint8_t array_size = 8;
const uint8_t array_len = 6;
int data_array[][6] = {
		{4, 8, 0, 0, 0, 0},
		{5, 6, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0}
};
int32_t menu1position = 0;
const uint8_t max_menu_position = 5;

/////////////////////////////////////////////////////////////////////////////////////////////////////////

// MESSAGE VARIABLES IN CHANGING LENGTH MODE
int pointer_y_coordinate = 0;
char messages[][25] = {
		{"L1="},
		{"L2="},
		{"L3="},
		{"L4="},
		{"L5="},
		{"L6="},
		{"L7="},
		{"L8="},
};
char current_messages[][25] = {
		{" L1=480,000мм"},
		{" L2=560,000мм"},
		{" L3=000,000мм"},
		{" L4=000,000мм"},
		{" L5=000,000мм"},
		{" L6=000,000мм"},
		{" L7=000,000мм"},
		{" L8=000,000мм"},
};

const int line_points_max_index = 1024;
int line_points_x[1025] = {0};  // initialized in main
// readed values
int line_points_y[1025] = {0};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// USER U8G2 VARIALBLE:
static u8g2_t u8g2;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM10_Init(void);
static void MX_TIM9_Init(void);
static void MX_TIM13_Init(void);
static void MX_TIM14_Init(void);
static void MX_RTC_Init(void);
void rewriteDisplayFunction(void *argument);
void buttonListenFunction(void *argument);

/* USER CODE BEGIN PFP */

// Example menu item specific enter callback function, run when the associated menu item is entered.
static void Level1Item2_Enter(void) {
	u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
	u8g2_FirstPage(&u8g2);
	do {
		u8g2_DrawStr(&u8g2, 0, 15, "ENTER");
	} while (u8g2_NextPage(&u8g2));
}

// Example menu item specific select callback function, run when the associated menu item is selected.
static void Item_Select_Callback(uint8_t x, uint8_t y) {
	u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
	u8g2_SetDrawColor(&u8g2, 1);	// change color of text (inverse)
	u8g2_DrawRBox(&u8g2, x, y, 100, 20, 8);	// draw frame
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////D

// DRAW DOTTED LINE
void u8g2_DrawDLine(u8g2_t *u8g2, u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2, u8g2_uint_t y2, u8g2_int_t d) {
	u8g2_uint_t tmp;
	u8g2_uint_t x,y;
	u8g2_uint_t dx, dy;
	u8g2_int_t err;
	u8g2_int_t ystep;
	uint8_t swapxy = 0;
	/* no intersection check at the moment, should be added... */
	if (x1>x2) dx=x1-x2; else dx=x2-x1;
	if (y1>y2) dy=y1-y2; else dy=y2-y1;
	if (dy>dx) {
		swapxy = 1;
		tmp=dx; dx=dy; dy=tmp;
		tmp=x1; x1=y1; y1=tmp;
		tmp=x2; x2=y2; y2=tmp;
	}
	if (x1>x2) tmp=x1, x1=x2, x2=tmp, tmp=y1, y1=y2, y2=tmp;
	err = dx >> 1;
	if (y2>y1) ystep = 1; else ystep = -1, y = y1;
	#ifndef  U8G2_16BIT
		if (x2 == 255) x2--;
	#else
		if (x2 == 0xffff) x2--;
	#endif
	for(x=x1; x<=x2; x++) {
		if (swapxy == 0) {
			if(d==0) {
				u8g2_DrawPixel(u8g2, x, y); /* solid line */
			} else if(d==1){
				if(x%2==0) u8g2_DrawPixel(u8g2, x, y); /* dotted line */
			} else if(d>1){
				if((x/d)%2==0) u8g2_DrawPixel(u8g2, x, y); /* dashed line */
			} else if(d<0){
				if((x/-d)%2!=0) u8g2_DrawPixel(u8g2, x, y); /* dashed line inverted */
			}
		} else {
			if(d==0) {
				u8g2_DrawPixel(u8g2, y, x); /* solid line */
			} else if(d==1){
				if(x%2==0) u8g2_DrawPixel(u8g2, y, x); /* dotted line */
			} else if(d>1){
				if((x/d)%2==0) u8g2_DrawPixel(u8g2, y, x); /* dashed line */
			} else if(d<0){
				if((x/-d)%2!=0) u8g2_DrawPixel(u8g2, y, x); /* dashed line inverted */
			}
		}
		err -= (uint8_t)dy;
		if (err<0) y += (u8g2_uint_t)ystep, err += (u8g2_uint_t)dx;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// DRAW GRAPH FOR CHANGING LENGTH POINT IN MENU (2ND)
uint64_t used_difference_ = 0;
uint8_t already_used_ = 0;
static void DrawGraphChangingLength(void) {
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_FirstPage(&u8g2);
	do {
		u8g2_DrawFrame(&u8g2, 10, 4, 220, 120); // x, y, w, h
		u8g2_SetFont(&u8g2, u8g2_font_10x20_t_cyrillic);
		u8g2_DrawStr(&u8g2, 115, 30, "?");
		// draw unicode symbols
		// horizontal lines:
		u8g2_DrawDLine(&u8g2, 10, 40, 229, 40, 1); // x1, y1, x2, y2
		u8g2_DrawDLine(&u8g2, 10, 110, 229, 110, 1); // x1, y1, x2, y2
		// vertical lines:
		u8g2_DrawDLine(&u8g2, 78, 4, 78, 40, 1); // x1, y1, x2, y2
		u8g2_DrawDLine(&u8g2, 162, 4, 162, 40, 1); // x1, y1, x2, y2
		if (!already_used_) {
			used_difference_ = 0;
			for (int i = 0; i <= line_points_max_index; ++i) {
				used_difference_ += line_points_y[i];
			}
			used_difference_ /= line_points_max_index+1;
			already_used_ = 1;
		}
		if (already_used_ && used_difference_) {
			u8g2_DrawUTF8(&u8g2, 30, 30, "СЧ");
			u8g2_DrawUTF8(&u8g2, 190, 30, "ВЧ");
			u8g2_DrawStr(&u8g2, 85, 80, " > L > ");
			u8g2_DrawUTF8(&u8g2, 35, 95, "мм");
			u8g2_DrawUTF8(&u8g2, 185, 95, "мм");
			char mes1[25];
			char mes2[25];
			double ans1 = used_difference_*max_hss_speed;
			double ans2 = used_difference_*min_gs_speed;
			if (ans1 && ans2) {
				sprintf(mes1, "%.2lf\n", ans1);
				u8g2_DrawUTF8(&u8g2, 20, 80, mes1);
				sprintf(mes2, "%.2lf\n", ans2);
				u8g2_DrawUTF8(&u8g2, 165, 80, mes2);
			}
			already_used_ = 0;
		}
	} while (u8g2_NextPage(&u8g2));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//MENU ITEMS INITIALIZATION
//MENU_ITEM(Name, Next, Previous, Parent,    Child,     SelectFunc,           EnterFunc,         xPos,yPos, cursorWidth, cursorHeight, hasachild, Text)
MENU_ITEM(Menu_1, Menu_2, Menu_2, NULL_MENU, Menu_1_1,  Item_Select_Callback, Level1Item2_Enter,       0, 20,  240, 40, 1, "С неизменяющимся измеряемым -размером");
MENU_ITEM(Menu_2, Menu_1, Menu_1, NULL_MENU, NULL_MENU, Item_Select_Callback, DrawGraphChangingLength, 0, 60,  240, 40, 0, "С изменяющимся измеряемым -размером"); // DrawGraphChangingLength
//MENU_ITEM(Menu_3, Menu_1, Menu_2, NULL_MENU, NULL_MENU, Item_Select_Callback, NULL, 0, 100, 240, 40, 0, "График измерений");

MENU_ITEM(Menu_1_1, Menu_1_2, Menu_1_8, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 5, 20, 110, 25, 0, current_messages[0]);
MENU_ITEM(Menu_1_2, Menu_1_3, Menu_1_1, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 5, 45, 110, 25, 0, current_messages[1]);
MENU_ITEM(Menu_1_3, Menu_1_4, Menu_1_2, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 5, 70, 110, 25, 0, current_messages[2]);
MENU_ITEM(Menu_1_4, Menu_1_5, Menu_1_3, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 5, 95, 110, 25, 0, current_messages[3]);
MENU_ITEM(Menu_1_5, Menu_1_6, Menu_1_4, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 5, 120, 110, 25, 0, current_messages[4]);
MENU_ITEM(Menu_1_6, Menu_1_7, Menu_1_5, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 125, 20, 110, 25, 0, current_messages[5]);
MENU_ITEM(Menu_1_7, Menu_1_8, Menu_1_6, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 125, 45, 110, 25, 0, current_messages[6]);
MENU_ITEM(Menu_1_8, Menu_1_1, Menu_1_7, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 125, 70, 110, 25, 0, current_messages[7]);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// FUNCTION FOR WRITE TEXT AND VALUES IN MENU ITEMS
static void Generic_Write(Menu_Item_t *currentMenuItem, Menu_Item_t *currentMenuLayerTopItem) {
	Menu_Item_t *bufptr = currentMenuLayerTopItem;
	u8g2_FirstPage(&u8g2);
	do {
		u8g2_SetDrawColor(&u8g2, 1);	// inverse color
		u8g2_DrawBox(&u8g2, currentMenuItem->xPos, currentMenuItem->yPos - 17, currentMenuItem->cursorWidth, currentMenuItem->cursorHeigth);	// select frame
		u8g2_SetFont(&u8g2, u8g2_font_unifont_t_cyrillic);	// set cyrillic font
		do {
			// if have same coordinates
			if (bufptr->yPos == currentMenuItem->yPos && bufptr->xPos == currentMenuItem->xPos) {
				u8g2_SetDrawColor(&u8g2, 0); // set ordinary text color
				uint8_t stringInterval = 0;
				char str[100];
				strcpy(str, (bufptr->Text)); // copy text from buffer
				const char s[2] = "-";
				char *token;
				token = strtok(str, s);
				while (token != NULL) {
					u8g2_DrawUTF8(&u8g2, bufptr->xPos, bufptr->yPos + stringInterval, token);
					token = strtok(NULL, s);
					stringInterval += 15;
				}
				u8g2_SetDrawColor(&u8g2, 1);
			} else {
				u8g2_SetDrawColor(&u8g2, 1); // set inverse text color
				uint8_t stringInterval = 0;
				char str[100];
				strcpy(str, (bufptr->Text));
				const char s[2] = "-";
				char *token;
				token = strtok(str, s);
				while (token != NULL) {
					u8g2_DrawUTF8(&u8g2, bufptr->xPos, bufptr->yPos + stringInterval, token);
					token = strtok(NULL, s);
					stringInterval += 15;
				}
			}
			bufptr = bufptr->Next;
			// IF FIRST MENU ITEM - DRAW UNDERLINE SYMBOLS
			if (Menu_GetCurrentMenu()->Parent == &Menu_1) {
				uint8_t index_ = 0;
				uint8_t row = 0;
				if (Menu_GetCurrentMenu() == &Menu_1_1) {
					index_ = 0, row = 0;
				} else if (Menu_GetCurrentMenu() == &Menu_1_2) {
					index_ = 1, row = 0;
				} else if (Menu_GetCurrentMenu() == &Menu_1_3) {
					index_ = 2, row = 0;
				} else if (Menu_GetCurrentMenu() == &Menu_1_4) {
					index_ = 3, row = 0;
				} else if (Menu_GetCurrentMenu() == &Menu_1_5) {
					index_ = 4, row = 0;
				} else if (Menu_GetCurrentMenu() == &Menu_1_6) {
					index_ = 0, row = 1;
				} else if (Menu_GetCurrentMenu() == &Menu_1_7) {
					index_ = 1, row = 1;
				} else if (Menu_GetCurrentMenu() == &Menu_1_8) {
					index_ = 2, row = 1;
				}
				u8g2_SetDrawColor(&u8g2, 0);
				if (row == 0) {
					if (menu1position <= 2) {
						u8g2_DrawHLine(&u8g2, 37+8*menu1position, 21+25*index_, 8);
						u8g2_DrawHLine(&u8g2, 37+8*menu1position, 22+25*index_, 8);
					} else {
						u8g2_DrawHLine(&u8g2, 45+8*menu1position, 21+25*index_, 8);
						u8g2_DrawHLine(&u8g2, 45+8*menu1position, 22+25*index_, 8);
					}
				} else if (row == 1){
					if (menu1position <= 2) {
						u8g2_DrawHLine(&u8g2, 157+8*menu1position, 21+25*index_, 8);
						u8g2_DrawHLine(&u8g2, 157+8*menu1position, 22+25*index_, 8);
					} else {
						u8g2_DrawHLine(&u8g2, 165+8*menu1position, 21+25*index_, 8);
						u8g2_DrawHLine(&u8g2, 165+8*menu1position, 22+25*index_, 8);
					}
				}
			}
		} while (bufptr != currentMenuLayerTopItem);
	} while (u8g2_NextPage(&u8g2));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TRANSFORM DOUBLE FROM data_array TO STRING
double double_string(int x) {
	int last_part = 0;
	for (int i = 5; i > 2; --i){
		last_part += data_array[x][i] * pow(10, 5-i);
	}
	int first_part = 0;
	for (int i = 0; i < 3; ++i) {
		first_part += data_array[x][i] * pow(10, 2-i);
	}
	return (double)first_part + (double)last_part/1000;
}

// ADD ELEMENT IN GRAPH (3 POINT IN START MENU)
void add_element(int a) {
	for (int i = 0; i < line_points_max_index; ++i) {
		line_points_y[i] = line_points_y[i+1];
	}
	line_points_y[line_points_max_index] = a;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// DRAW OBJECT LENGTH LIMITS (2 MENU ITEM)
uint64_t used_difference = 0;
static void DrawStateConstatntLength() {
	uint8_t already_used = 0;
	u8g2_FirstPage(&u8g2);
	u8g2_SetDrawColor(&u8g2, 1);
	const int max_x_point = 220;
	const int max_y_point = 110;
	do {
		// draw frame
		u8g2_DrawFrame(&u8g2, 10, 10, max_x_point, max_y_point+1);
		u8g2_SetFont(&u8g2, u8g2_font_10x20_t_cyrillic);
		int index_ = 9;
		if (Menu_GetCurrentMenu() == &Menu_1_1) {
			index_ = 0;
		} else if (Menu_GetCurrentMenu() == &Menu_1_2) {
			index_ = 1;
		} else if (Menu_GetCurrentMenu() == &Menu_1_3) {
			index_ = 2;
		} else if (Menu_GetCurrentMenu() == &Menu_1_4) {
			index_ = 3;
		} else if (Menu_GetCurrentMenu() == &Menu_1_5) {
			index_ = 4;
		} else if (Menu_GetCurrentMenu() == &Menu_1_6) {
			index_ = 5;
		} else if (Menu_GetCurrentMenu() == &Menu_1_7) {
			index_ = 6;
		} else if (Menu_GetCurrentMenu() == &Menu_1_8) {
			index_ = 7;
		}
		if (index_ != 9) {
			char mes[25];
			sprintf(mes, "L = %d%d%d,%d%d%d мм", data_array[index_][0], data_array[index_][1], data_array[index_][2], data_array[index_][3], data_array[index_][4], data_array[index_][5]);
			u8g2_DrawUTF8(&u8g2, 50, 30, mes);
		}
		if (!already_used) {
			used_difference = 0;
			for (int i = 0; i <= 240; ++i) {
				used_difference += line_points_y[i];
			}
			used_difference /= 241;
			already_used = 1;
		}
		// draw frame
		u8g2_DrawFrame(&u8g2, 90, 40, 60, 30);
		double length = double_string(index_);
		double min_gs_length = min_gs_speed*used_difference;
		double max_hss_length = max_hss_speed*used_difference;
		double max_gs_length = max_gs_speed*used_difference;
		double min_hss_length = min_hss_speed*used_difference;
		if (used_difference) {
			if (length <= max_gs_length && length > min_gs_length) {
				u8g2_DrawUTF8(&u8g2, 110, 60, "СЧ");
			} else if (length >= min_hss_length && length < max_hss_length) {
				u8g2_DrawUTF8(&u8g2, 110, 60, "ВЧ");
			} else {
				u8g2_DrawUTF8(&u8g2, 115, 60, "?");
			}
		} else {
			u8g2_DrawUTF8(&u8g2, 115, 60, "?");
		}
		u8g2_DrawHLine(&u8g2, 10, 73, 220);
		char mes3[25];
		char mes4[25];
		sprintf(mes3, "СЧ: L<=%.3lfмм\n", max_gs_length);
		sprintf(mes4, "ВЧ: L>=%.3lfмм\n", min_hss_length);
		u8g2_DrawUTF8(&u8g2, 30, 90, mes3);
		u8g2_DrawUTF8(&u8g2, 30, 110, mes4);
	} while (u8g2_NextPage(&u8g2));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ALTERNATIVE SAFER DELAY FUNCTION FOR MICROSECONDS DELAY
void delay_us(uint8_t us) {
	__HAL_TIM_SET_COUNTER(&htim13, 0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(&htim13) < us);
	// wait for the counter to reach the us input in the parameter
}

// U8G2 FUNCTION TO WORK PROPERLY (DISPLAY PINS ARE ASSIGNED HERE)
uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr) {
	switch (msg) {
		case U8X8_MSG_GPIO_AND_DELAY_INIT:
			HAL_GPIO_WritePin(WR1_GPIO_Port, WR1_Pin, 1);
			HAL_GPIO_WritePin(WR0_GPIO_Port, WR0_Pin, 1);
			osDelay(1);
			break;
		case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
			delay_us(arg_int / 10);
			break;
		case U8X8_MSG_DELAY_10MICRO:		// delay arg_int * 10 micro seconds
			delay_us(arg_int * 10);
			break;
		case U8X8_MSG_DELAY_MILLI:			// delay arg_int * 1 milli second
			osDelay(arg_int);
			break;
		case U8X8_MSG_DELAY_NANO:			// delay arg_int * 1 milli second
			delay_us(arg_int / 1000);
			break;
		case U8X8_MSG_GPIO_D0:
			HAL_GPIO_WritePin(DB0_GPIO_Port, DB0_Pin, arg_int);
			break;
		case U8X8_MSG_GPIO_D1:
			HAL_GPIO_WritePin(DB1_GPIO_Port, DB1_Pin, arg_int);
			break;
		case U8X8_MSG_GPIO_D2:
			HAL_GPIO_WritePin(DB2_GPIO_Port, DB2_Pin, arg_int);
			break;
		case U8X8_MSG_GPIO_D3:
			HAL_GPIO_WritePin(DB3_GPIO_Port, DB3_Pin, arg_int);
			break;
		case U8X8_MSG_GPIO_D4:
			HAL_GPIO_WritePin(DB4_GPIO_Port, DB4_Pin, arg_int);
			break;
		case U8X8_MSG_GPIO_D5:
			HAL_GPIO_WritePin(DB5_GPIO_Port, DB5_Pin, arg_int);
			break;
		case U8X8_MSG_GPIO_D6:
			HAL_GPIO_WritePin(DB6_GPIO_Port, DB6_Pin, arg_int);
			break;
		case U8X8_MSG_GPIO_D7:
			HAL_GPIO_WritePin(DB7_GPIO_Port, DB7_Pin, arg_int);
			break;
		case U8X8_MSG_GPIO_DC:
			HAL_GPIO_WritePin(CD_GPIO_Port, CD_Pin, arg_int);
			break;					// used as E2 for the SED1520
		case U8X8_MSG_GPIO_E: // 72
			HAL_GPIO_WritePin(WR0_GPIO_Port, WR0_Pin, arg_int);
			break;
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// initialize beackup strategy
void serialize_to_bkp_registers() {
    uint32_t value = 0;
    uint32_t index = 0;
    HAL_PWR_EnableBkUpAccess();
    for (uint32_t i = 0; i < array_size; i++) {
        for (uint32_t j = 0; j < array_len; j++) {
            value = value * 10 + data_array[i][j];
        }
        if (index < RTC_BKP_NUMBER) {
            HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0 + index, value);
            index++;
            value = 0;
        }
    }
    HAL_PWR_DisableBkUpAccess();
}


void deserialize_from_bkp_registers() {
    uint32_t index = 0;
    uint32_t value;

    for (uint32_t i = 0; i < array_size; i++) {
        if (index < RTC_BKP_NUMBER) {
            value = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0 + index);
            if (value) {
				index++;

				for (int j = array_len - 1; j >= 0; j--) {
					data_array[i][j] = value % 10;
					value /= 10;
				}
            }
        }
    }
}


// CAPTURE TIME BETWEEN TWO PULSES
uint32_t number_of_overflowing = 0;  // TO DO
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if ((htim->Instance == TIM10) && (valid_for_capturing == 1)) {
		Difference = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
		if ((Difference > TEST_DELAY) && (Difference < 30000)) {
			add_element(round(COEFFICIENT*(double)(Difference)/TIMCLOCK) - TEST_DELAY);
			valid_for_capturing = 0;
		}
	}
}

// stop generating ultra-sound
//uint8_t is_period_started = 0;
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM14) {
		__HAL_TIM_SET_COUNTER(&htim10, 0x0000);
		valid_for_capturing = 1;
	}
}

// reset timer for input capture, to get only ticks of tim2 after
// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
// left in bottom

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
  MX_TIM10_Init();
  MX_TIM9_Init();
  MX_TIM13_Init();
  MX_TIM14_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_OC_Start_IT(&htim14, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start_IT(&htim14, TIM_CHANNEL_1);

  HAL_TIM_PWM_Start(&htim13, TIM_CHANNEL_1);

//  __HAL_TIM_ENABLE_IT(&htim10, TIM_IT_UPDATE);
  HAL_TIM_IC_Start_IT(&htim10, TIM_CHANNEL_1);


  HAL_Delay(1500);

  u8g2_Setup_uc1608_240x128_2(&u8g2, U8G2_R0, u8x8_byte_8bit_8080mode, u8g2_gpio_and_delay_stm32); //u8x8_byte_8bit_6800mode
  // DISPLAY INITIALIZATION
  u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
  u8g2_SetPowerSave(&u8g2, 0); // wake up display
  // START MENU WORK
  Menu_SetGenericWriteCallback(Generic_Write, &Menu_1);
  Menu_SetCurrentLayerTopMenu(&Menu_1);
  Menu_Navigate(&Menu_1, Menu_GetCurrentLayerTopMenu());

  // get all data from backup
  deserialize_from_bkp_registers();

  for (uint32_t i = 0; i <= line_points_max_index; ++i) {
	  line_points_x[i] = i;
  }

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of rewriteDisplay */
  rewriteDisplayHandle = osThreadNew(rewriteDisplayFunction, NULL, &rewriteDisplay_attributes);

  /* creation of buttonListen */
  buttonListenHandle = osThreadNew(buttonListenFunction, NULL, &buttonListen_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 6;
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

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM9 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM9_Init(void)
{

  /* USER CODE BEGIN TIM9_Init 0 */

  /* USER CODE END TIM9_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM9_Init 1 */

  /* USER CODE END TIM9_Init 1 */
  htim9.Instance = TIM9;
  htim9.Init.Prescaler = 84-1;
  htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim9.Init.Period = 1000-1;
  htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 200;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM9_Init 2 */

  /* USER CODE END TIM9_Init 2 */
  HAL_TIM_MspPostInit(&htim9);

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 0;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 65535;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim10, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}

/**
  * @brief TIM13 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM13_Init(void)
{

  /* USER CODE BEGIN TIM13_Init 0 */

  /* USER CODE END TIM13_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM13_Init 1 */

  /* USER CODE END TIM13_Init 1 */
  htim13.Instance = TIM13;
  htim13.Init.Prescaler = 84-1;
  htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim13.Init.Period = 65536-1;
  htim13.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim13.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim13) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim13) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim13, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM13_Init 2 */

  /* USER CODE END TIM13_Init 2 */
  HAL_TIM_MspPostInit(&htim13);

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 0;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 56000-1;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 10;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim14, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */
  HAL_TIM_MspPostInit(&htim14);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, WR0_Pin|DB7_Pin|DB6_Pin|DB5_Pin
                          |DB4_Pin|DB3_Pin|DB2_Pin|DB1_Pin
                          |DB0_Pin|WR1_Pin|CD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, COL1_Pin|COL4_Pin|COL3_Pin|COL2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 PE4 PE6
                           PE7 PE8 PE9 PE10
                           PE11 PE12 PE13 PE14
                           PE15 PE0 PE1 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC13 PC0 PC1 PC2
                           PC3 PC4 PC5 PC6
                           PC7 PC8 PC9 PC10
                           PC11 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PF0 PF1 PF2 PF3
                           PF4 PF5 PF7 PF10
                           PF11 PF12 PF13 PF14
                           PF15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3
                           PA4 PA5 PA6 PA7
                           PA8 PA9 PA10 PA11
                           PA12 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10
                           PB11 PB12 PB13 PB14
                           PB15 PB3 PB4 PB5
                           PB6 PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PG0 PG10 PG11 PG13
                           PG14 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_13
                          |GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : WR0_Pin DB7_Pin DB6_Pin DB5_Pin
                           DB4_Pin DB3_Pin DB2_Pin DB1_Pin
                           DB0_Pin WR1_Pin CD_Pin */
  GPIO_InitStruct.Pin = WR0_Pin|DB7_Pin|DB6_Pin|DB5_Pin
                          |DB4_Pin|DB3_Pin|DB2_Pin|DB1_Pin
                          |DB0_Pin|WR1_Pin|CD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : COL1_Pin COL4_Pin COL3_Pin COL2_Pin */
  GPIO_InitStruct.Pin = COL1_Pin|COL4_Pin|COL3_Pin|COL2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PD9 PD11 PD1 PD2
                           PD3 PD5 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_11|GPIO_PIN_1|GPIO_PIN_2
                          |GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : ROW5_Pin ROW4_Pin ROW1_Pin ROW3_Pin
                           ROW2_Pin */
  GPIO_InitStruct.Pin = ROW5_Pin|ROW4_Pin|ROW1_Pin|ROW3_Pin
                          |ROW2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_rewriteDisplayFunction */
/**
  * @brief  Function implementing the rewriteDisplay thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_rewriteDisplayFunction */
void rewriteDisplayFunction(void *argument)
{
  /* USER CODE BEGIN 5 */
//	u8g2_ClearBuffer(&u8g2);
//	u8g2_ClearDisplay(&u8g2);
  /* Infinite loop */
	for(;;){
	  switch(ButtonState) {
		// MOVE FORWARD IN MENU
		case 1:
			Menu_Navigate(Menu_GetCurrentMenu()->Next, Menu_GetCurrentLayerTopMenu()); // Next, Child, Parent, Previous
			ButtonState = 0;
			break;
		// MOVE BACKWARDS IN MENU
		case 2:
			Menu_Navigate(Menu_GetCurrentMenu()->Previous, Menu_GetCurrentLayerTopMenu()); // Next, Child, Parent, Previous
			ButtonState = 0;
			break;
		// MOVE IN CHILD ELEMENT, PROGRESS (FOR GRAPHS AND 2 MENU DETERMINATION BREAK U8G2 RULES)
		case 3:
			if (Menu_GetCurrentMenu() == &Menu_2) {
				u8g2_FirstPage(&u8g2);
				do {
					DrawGraphChangingLength();
				} while (u8g2_NextPage(&u8g2));
				break;
			} else if (Menu_GetCurrentMenu()->Parent == &Menu_1) {
				u8g2_FirstPage(&u8g2);
				do {
					DrawStateConstatntLength();
				} while (u8g2_NextPage(&u8g2));
				break;
			} else {
				Menu_Navigate(Menu_GetCurrentMenu()->Child, Menu_GetCurrentLayerTopMenu()->Child); // Next, Child, Parent, Previous
			}
			ButtonState = 0;
			break;
		// MOVE INTO PARENT ELEMENT, GET BACK IN PROGRESSION
		case 4:
			Menu_Navigate(Menu_GetCurrentMenu()->Parent, Menu_GetCurrentLayerTopMenu()->Parent); // Next, Child, Parent, Previous
			ButtonState = 0;
			break;
		// FIRST MENU - MOVE CHANGEBLE POSITION TO LOWER
		case 5:
			if (Menu_GetCurrentMenu()->Parent == &Menu_1) {
				if (menu1position < max_menu_position) {
					menu1position += 1;
				}
				ButtonState = 0;
			}
			break;
		// IF FIRST MENU - MOVE TO HIGHER POSITION
		case 6:
			if (Menu_GetCurrentMenu()->Parent == &Menu_1) {
				if (menu1position > 0) {
					menu1position -= 1;
				}
				ButtonState = 0;
			}
			break;
		case 7:
			if (Menu_GetCurrentMenu()->Parent == &Menu_1) {
				int index_ = 9;
				if (Menu_GetCurrentMenu() == &Menu_1_1) {
					index_ = 0;
				} else if (Menu_GetCurrentMenu() == &Menu_1_2) {
					index_ = 1;
				} else if (Menu_GetCurrentMenu() == &Menu_1_3) {
					index_ = 2;
				} else if (Menu_GetCurrentMenu() == &Menu_1_4) {
					index_ = 3;
				} else if (Menu_GetCurrentMenu() == &Menu_1_5) {
					index_ = 4;
				} else if (Menu_GetCurrentMenu() == &Menu_1_6) {
					index_ = 5;
				} else if (Menu_GetCurrentMenu() == &Menu_1_7) {
					index_ = 6;
				} else if (Menu_GetCurrentMenu() == &Menu_1_8) {
					index_ = 7;
				}
				if (index_ != 9) {
					if (data_array[index_][menu1position] > 0) {
						data_array[index_][menu1position] -= 1;
					}
				}
				for (size_t i = 0; i < array_size; ++i) {
					sprintf(current_messages[i], " %s%d%d%d,%d%d%dмм", messages[i], data_array[i][0], data_array[i][1], data_array[i][2], data_array[i][3], data_array[i][4], data_array[i][5]);
				}
				ButtonState = 0;
				serialize_to_bkp_registers();
			}
			break;
		// IF GRAPH - MOVE DOWN, THEN REDRAW
		case 8:
			if (Menu_GetCurrentMenu()->Parent == &Menu_1) {
				int index_ = 9;
				if (Menu_GetCurrentMenu() == &Menu_1_1) {
					index_ = 0;
				} else if (Menu_GetCurrentMenu() == &Menu_1_2) {
					index_ = 1;
				} else if (Menu_GetCurrentMenu() == &Menu_1_3) {
					index_ = 2;
				} else if (Menu_GetCurrentMenu() == &Menu_1_4) {
					index_ = 3;
				} else if (Menu_GetCurrentMenu() == &Menu_1_5) {
					index_ = 4;
				} else if (Menu_GetCurrentMenu() == &Menu_1_6) {
					index_ = 5;
				} else if (Menu_GetCurrentMenu() == &Menu_1_7) {
					index_ = 6;
				} else if (Menu_GetCurrentMenu() == &Menu_1_8) {
					index_ = 7;
				}
				if (index_ != 9) {
					if (data_array[index_][menu1position] < 9) {
						data_array[index_][menu1position] += 1;
					}
				}
				for (size_t i = 0; i < array_size; ++i) {
					sprintf(current_messages[i], " %s%d%d%d,%d%d%dмм", messages[i], data_array[i][0], data_array[i][1], data_array[i][2], data_array[i][3], data_array[i][4], data_array[i][5]);
				}
				ButtonState = 0;
				serialize_to_bkp_registers();
			}
			break;
		case 0:
			Menu_Navigate(Menu_GetCurrentMenu(), Menu_GetCurrentLayerTopMenu());
			break;
		default:
			ButtonState = 0;
		}
		osDelay(10);
	}
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_buttonListenFunction */
/**
* @brief Function implementing the buttonListen thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_buttonListenFunction */
void buttonListenFunction(void *argument)
{
  /* USER CODE BEGIN buttonListenFunction */
  /* Infinite loop */
		//0  1  2  3  {+1}
		//4  5  6  7  {+1}
		//8  9  10 11 {+1}
		//12 13 14 15 {+1}
		const int rows_count = 4;
		const int columns_count = 4;
		GPIO_TypeDef* row_ports[] = {ROW1_GPIO_Port, ROW2_GPIO_Port, ROW3_GPIO_Port, ROW4_GPIO_Port, ROW5_GPIO_Port};
		uint16_t row_pins[] = {ROW1_Pin, ROW2_Pin, ROW3_Pin, ROW4_Pin, ROW5_Pin};
		GPIO_TypeDef* column_ports[] = {COL1_GPIO_Port, COL2_GPIO_Port, COL3_GPIO_Port, COL4_GPIO_Port};
		uint16_t column_pins[] = {COL1_Pin, COL2_Pin, COL3_Pin, COL4_Pin};
		for(;;){
			for (int i = 0; i < columns_count; i++) {
				for (int j = 0; j < columns_count; j++) {
					HAL_GPIO_WritePin(column_ports[j], column_pins[j], (i==j) ? GPIO_PIN_RESET : GPIO_PIN_SET);  //Pull low
				}
				for (int j = 0; j < rows_count; j++) {
					if (HAL_GPIO_ReadPin(row_ports[j], row_pins[j]) == GPIO_PIN_RESET) {  // if the Col j is high
//						TIM9->CCR2 = 500;
						HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
						while (HAL_GPIO_ReadPin(row_ports[j], row_pins[j]) == GPIO_PIN_RESET);
						ButtonState = j * columns_count + i + 1;
						HAL_TIM_PWM_Stop(&htim9, TIM_CHANNEL_1);
					}
				}
			}
		}
  /* USER CODE END buttonListenFunction */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
//  if (htim->Instance == TIM4) {
//	HAL_TIM_PWM_Start(&htim13, TIM_CHANNEL_1);
//	//delay_us(1);
//	//HAL_GPIO_WritePin(Biased_A_PWM_GPIO_Port, Biased_A_PWM_Pin, GPIO_PIN_SET);
//  }
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
//  if (htim->Instance == TIM4) {
//	  is_period_started = 1;
//  }
  /* USER CODE END Callback 1 */
}

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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
