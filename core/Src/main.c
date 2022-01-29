#include "main.h"
#include "main.h"
#include "u8g2/u8g2.h"
#include <string.h>
#include "STM32F4xx.h"
#include <stm32f4xx_hal.h>
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"

// BUGS: CAN CHANGE CRAPH SETTINGS WHEN ITS TURNED OFF (SOLVATION: MAKE STATE VARIABLE FOR MENU MODE)

const uint8_t ButtonMemory = 4;
const uint8_t RewriteMemory = 48;
const uint8_t RandomMemory = 4;

// system and muttexes
TIM_HandleTypeDef htim10;
UART_HandleTypeDef huart2;
osThreadId_t buttonListenHandle;
osThreadId_t setRandomHandle;
osThreadId_t rewriteDisplayHandle;

uint8_t ButtonState = 0;

const osThreadAttr_t buttonListen_attributes = {
  .name = "buttonListen",
  .stack_size = configMINIMAL_STACK_SIZE * ButtonMemory,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for rewriteDisplay */
osThreadId_t rewriteDisplayHandle;
const osThreadAttr_t rewriteDisplay_attributes = {
  .name = "rewriteDisplay",
  .stack_size = configMINIMAL_STACK_SIZE * RewriteMemory,
  .priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t setRandomHandle;
const osThreadAttr_t setRandom_attributes = {
  .name = "setRandom",
  .stack_size = configMINIMAL_STACK_SIZE * RandomMemory,
  .priority = (osPriority_t) osPriorityNormal,
};

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM10_Init(void);
void buttonListen(void *argument);
void reDraw(void *argument);
void setRandomState(void *argument);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const uint8_t array_size = 8;
const uint8_t used_elements = 5;
int data_array[] = {1825, 1925, 2025, 2125, 2225, 0, 0, 0};

char messages[][25] = {
		{"L1 = "},
		{"L2 = "},
		{"L3 = "},
		{"L4 = "},
		{"L5 = "},
		{"L6 = "},
		{"L7 = "},
		{"L8 = "},
};

char current_messages[][25] = {
		{"L1 = **,*"},
		{"L2 = **,*"},
		{"L3 = **,*"},
		{"L4 = **,*"},
		{"L5 = **,*"},
		{"L6 = **,*"},
		{"L7 = **,*"},
		{"L8 = **,*"},
};

double multiplier[] = {0.5, 0.75, 1.0, 1.5, 2.0};
char multiplier_names[][25] = {
		{"OX: 500mV"},
		{"OX: 750mV"},
		{"OX: 1000mV"},
		{"OX: 1500mV"},
		{"OX: 2000mV"},
};
int current_multiplier_index = 2;
int max_multiplier_index = 4;

int bias[] = {-32, -24, -16, -8, 0, 8, 16, 24, 32};
char bias_names[][25] = {
		{"Bias: 1000mV"},
		{"Bias: 750mV"},
		{"Bias: 500mV"},
		{"Bias: 250mV"},
		{"Bias: 0mV"},
		{"Bias: -250mV"},
		{"Bias: -500mV"},
		{"Bias: -750mV"},
		{"Bias: -1000mV"},
};
int current_bias_index = 4;
int max_bias_index = 8;

int line_points_x[] = {  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,
        13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,
        26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
        39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,
        52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,
        65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,
        78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,
        91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103,
       104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
       117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
       130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
       143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155,
       156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
       169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
       182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
       195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
       208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220,
       221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
       234, 235, 236, 237, 238, 239, 240};

int line_points_max_index = 240;

int line_points_y[] = {60,  61,  63,  64,  66,  67,  69,  70,  72,  74,  75,  77,  78,
        80,  81,  82,  84,  85,  87,  88,  90,  91,  92,  93,  95,  96,
        97,  98, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
       111, 111, 112, 113, 114, 114, 115, 116, 116, 117, 117, 117, 118,
       118, 118, 119, 119, 119, 119, 119, 119, 120, 119, 119, 119, 119,
       119, 119, 118, 118, 118, 117, 117, 117, 116, 116, 115, 114, 114,
       113, 112, 111, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102,
       101, 100,  98,  97,  96,  95,  93,  92,  91,  90,  88,  87,  85,
        84,  82,  81,  80,  78,  77,  75,  74,  72,  70,  69,  67,  66,
        64,  63,  61,  60,  58,  56,  55,  53,  52,  50,  49,  47,  45,
        44,  42,  41,  39,  38,  37,  35,  34,  32,  31,  30,  28,  27,
        26,  24,  23,  22,  21,  19,  18,  17,  16,  15,  14,  13,  12,
        11,  10,   9,   8,   8,   7,   6,   5,   5,   4,   3,   3,   2,
         2,   2,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0,   0,   0,   1,   1,   1,   2,   2,   2,   3,   3,
         4,   5,   5,   6,   7,   8,   8,   9,  10,  11,  12,  13,  14,
        15,  16,  17,  18,  19,  21,  22,  23,  24,  26,  27,  28,  30,
        31,  32,  34,  35,  37,  38,  39,  41,  42,  44,  45,  47,  49,
        50,  52,  53,  55,  56,  58,  59};

// USER:
static u8g2_t u8g2;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void add_element(int a) {
	for (int i = 0; i < line_points_max_index; ++i) {
		line_points_y[i] = line_points_y[i+1];
	}
	line_points_y[line_points_max_index] = a;
}
/** Example menu item specific enter callback function, run when the associated menu item is entered. */
static void Level1Item2_Enter(void) {
	u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
	u8g2_FirstPage(&u8g2);
	do {
		u8g2_DrawStr(&u8g2, 0, 15, "ENTER");
	} while (u8g2_NextPage(&u8g2));
}

int getRandInt(int low, int high)
{
   return rand() % ((high + 1) - low) + low;
}

static void BackButton_Enter(void) {
	// move to parent
	Menu_SetCurrentLayerTopMenu(Menu_GetCurrentMenu()->Parent);
	Menu_Navigate(Menu_GetCurrentMenu()->Parent, Menu_GetCurrentLayerTopMenu());
}

static void DrawGraphChangingLength(void) {
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_SetBitmapMode(&u8g2, 1);
	int rand = getRandInt(0, 2)*10;
	u8g2_FirstPage(&u8g2);
	do {
		u8g2_DrawXBMP(&u8g2, rand, 0, u8g_logo_width, u8g_logo_height, u8g_logo_bits);
	} while (u8g2_NextPage(&u8g2));
}

void u8g2_DrawDLine(u8g2_t *u8g2, u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2, u8g2_uint_t y2, u8g2_int_t d)
{
	u8g2_uint_t tmp;
	u8g2_uint_t x,y;
	u8g2_uint_t dx, dy;
	u8g2_int_t err;
	u8g2_int_t ystep;

	uint8_t swapxy = 0;

	/* no intersection check at the moment, should be added... */

	if ( x1 > x2 ) dx = x1-x2; else dx = x2-x1;
	if ( y1 > y2 ) dy = y1-y2; else dy = y2-y1;

	if ( dy > dx ) {
		swapxy = 1;
		tmp = dx; dx =dy; dy = tmp;
		tmp = x1; x1 =y1; y1 = tmp;
		tmp = x2; x2 =y2; y2 = tmp;
	}
	if ( x1 > x2 ) {
		tmp = x1; x1 =x2; x2 = tmp;
		tmp = y1; y1 =y2; y2 = tmp;
	}
	err = dx >> 1;
	if ( y2 > y1 ) ystep = 1; else ystep = -1;
	y = y1;

	#ifndef  U8G2_16BIT
	if ( x2 == 255 )
	x2--;
	#else
	if ( x2 == 0xffff )
	x2--;
	#endif

	for( x = x1; x <= x2; x++ ) {
		if ( swapxy == 0 ) {
			if(d==0) {
				/* solid line */
				 u8g2_DrawPixel(u8g2, x, y);
			} else if(d==1){
				/* dotted line */
				if(x%2==0) u8g2_DrawPixel(u8g2, x, y);
			} else if(d>1){
				/* dashed line */
				if((x/d)%2==0) u8g2_DrawPixel(u8g2, x, y);
			} else if(d<0){
				/* dashed line inverted */
				if((x/-d)%2!=0) u8g2_DrawPixel(u8g2, x, y);
			}
		} else {
			if(d==0){
				/* solid line */
				u8g2_DrawPixel(u8g2, y, x);
			}else if(d==1){
				/* dotted line */
				if(x%2==0) u8g2_DrawPixel(u8g2, y, x);
			}else if(d>1){
				/* dashed line */
				if((x/d)%2==0) u8g2_DrawPixel(u8g2, y, x);
			}else if(d<0){
				/* dashed line inverted */
				if((x/-d)%2!=0) u8g2_DrawPixel(u8g2, y, x);
			}
		}
		err -= (uint8_t)dy;
		if ( err < 0 ) {
			y += (u8g2_uint_t)ystep;
			err += (u8g2_uint_t)dx;
		}
	}
}

static void displayGraphPoints() {
	// axis name and value
	u8g2_SetFont(&u8g2,  u8g2_font_balthasar_titling_nbp_tr);
	u8g2_DrawStr(&u8g2, 10, 120, bias_names[current_bias_index]);
	u8g2_DrawStr(&u8g2, 100, 120, multiplier_names[current_multiplier_index]);
	u8g2_DrawStr(&u8g2, 175, 120,"OX: 500 ms");
	// static counter for new drawing
	u8g2_SetDrawColor(&u8g2, 1);
	const int max_x_point = 220;
	const int max_y_point = 100;
	// draw frame
	u8g2_DrawFrame(&u8g2, 10, 0, max_x_point, max_y_point);
	// vertical lines for grid
	int x_steps = 14;
	for (int i = 1; i < x_steps; ++i) {
		u8g2_DrawDLine(&u8g2, i*15 + 15, 0, i*15 + 15, max_y_point, 1);
	}
	// horizontal lines for grid
	int y_steps = 6;
	for (int i = 1; i < y_steps; ++i) {
		u8g2_DrawDLine(&u8g2, 10, i*16 + 2, max_x_point+10, i*16 + 2, 1);
	}
	// main axises:
	u8g2_DrawVLine(&u8g2, 120, 0, max_y_point);
	u8g2_DrawHLine(&u8g2, 10, 50, max_x_point);

	// draw lines for point forward
	for (int i = 10; i < max_x_point+10; ++i) {
		int start_y = (float)((line_points_y[i]-max_y_point/2)*multiplier[current_multiplier_index]) + bias[current_bias_index] + max_y_point/2;
		int end_y = (float)((line_points_y[i+1]-max_y_point/2)*multiplier[current_multiplier_index]) + bias[current_bias_index] + max_y_point/2;
		if ((start_y <= max_y_point) && (end_y <= max_y_point) && (start_y >= 0) && (end_y >= 0)) {
			u8g2_DrawLine(&u8g2, i, start_y, i+1, end_y);
		} else if ((start_y <= 0) && (end_y >= 0)) {
			u8g2_DrawLine(&u8g2, i, 0, i+1, end_y);
		} else if ((start_y >= 0) && (end_y <= 0)) {
			u8g2_DrawLine(&u8g2, i, start_y, i+1, 0);
		} else if ((start_y > max_y_point) && (start_y <= max_y_point)){
			u8g2_DrawLine(&u8g2, i, max_y_point, i+1, end_y);
		} else if (start_y <= max_y_point && start_y > max_y_point){
			u8g2_DrawLine(&u8g2, i, start_y, i+1, max_y_point);
		}
	}

	add_element(line_points_y[0]);
	osDelay(2);
}

static void DrawGraphPoints(void) {
	u8g2_FirstPage(&u8g2);
	do {
		displayGraphPoints(0, 20, 9, 160, 80);
	} while (u8g2_NextPage(&u8g2));
}

/** Example menu item specific select callback function, run when the associated menu item is selected. */
static void Item_Select_Callback(uint8_t x, uint8_t y) {
	u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
	u8g2_SetDrawColor(&u8g2, 1);	// change color of text (inverse)
	u8g2_DrawBox(&u8g2, x, y * 40, 80, 20);	// draw frame
}

static void Generic_Write(Menu_Item_t *currentMenuItem, Menu_Item_t *currentMenuLayerTopItem)
{
	Menu_Item_t *bufptr = currentMenuLayerTopItem;
	u8g2_FirstPage(&u8g2);
	do {
		u8g2_SetDrawColor(&u8g2, 1);	// inverse
		u8g2_DrawBox(&u8g2, currentMenuItem->xPos, currentMenuItem->yPos - 17, currentMenuItem->cursorWidth, currentMenuItem->cursorHeigth);	// select frame
		u8g2_SetFont(&u8g2, u8g2_font_unifont_t_cyrillic);	// set cyrillic font
		do {
			if (bufptr->yPos == currentMenuItem->yPos && bufptr->xPos == currentMenuItem->xPos) {
				u8g2_SetDrawColor(&u8g2, 0);
				uint8_t stringInterval = 0;
				char str[80];
				strcpy(str, (bufptr->Text));
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
				u8g2_SetDrawColor(&u8g2, 1);
				uint8_t stringInterval = 0;
				char str[80];
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
		} while (bufptr != currentMenuLayerTopItem);
	} while (u8g2_NextPage(&u8g2));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void delay_us(uint8_t us) {
	__HAL_TIM_SET_COUNTER(&htim10, 0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(&htim10) < us);
	// wait for the counter to reach the us input in the parameter
}

uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr) {
	switch (msg) {
	case U8X8_MSG_GPIO_AND_DELAY_INIT:
		HAL_GPIO_WritePin(WR1_GPIO_Port, WR1_Pin, 1);
		HAL_GPIO_WritePin(WR0_GPIO_Port, WR0_Pin, 1);
		HAL_Delay(1);
		break;
	case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
		delay_us(arg_int / 10);
		break;
	case U8X8_MSG_DELAY_10MICRO:		// delay arg_int * 10 micro seconds
		delay_us(arg_int * 10);
		break;
	case U8X8_MSG_DELAY_MILLI:			// delay arg_int * 1 milli second
		HAL_Delay(arg_int);
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

//MENU_ITEM(Name, Next, Previous, Parent,    Child,     SelectFunc,           EnterFunc,       xPos,yPos, cursorWidth, cursorHeight, hasachild, Text)
MENU_ITEM(Menu_1, Menu_2, Menu_2, NULL_MENU, Menu_1_1, Item_Select_Callback, Level1Item2_Enter, 0, 20, 240, 40, 1, "С неизменяющимся измеряемым -размером");
MENU_ITEM(Menu_2, Menu_1, Menu_1, NULL_MENU, NULL_MENU, Item_Select_Callback, DrawGraphChangingLength, 0, 70, 240, 40, 1, "С изменяющимся измеряемым -размером"); // DrawGraphChangingLength

MENU_ITEM(Menu_1_1, Menu_1_2, Menu_1_9, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 0, 20, 100, 25, 0, current_messages[0]);
MENU_ITEM(Menu_1_2, Menu_1_3, Menu_1_1, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 0, 45, 100, 25, 0, current_messages[1]);
MENU_ITEM(Menu_1_3, Menu_1_4, Menu_1_2, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 0, 70, 100, 25, 0, current_messages[2]);
MENU_ITEM(Menu_1_4, Menu_1_5, Menu_1_3, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 0, 95, 100, 25, 0, current_messages[3]);
MENU_ITEM(Menu_1_5, Menu_1_6, Menu_1_4, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 0, 120, 100, 25, 0, current_messages[4]);
MENU_ITEM(Menu_1_6, Menu_1_7, Menu_1_5, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 120, 20, 100, 25, 0, current_messages[5]);
MENU_ITEM(Menu_1_7, Menu_1_8, Menu_1_6, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 120, 45, 100, 25, 0, current_messages[6]);
MENU_ITEM(Menu_1_8, Menu_1_9, Menu_1_7, Menu_1, NULL_MENU, Item_Select_Callback, NULL, 120, 70, 100, 25, 0, current_messages[7]);
MENU_ITEM(Menu_1_9, Menu_1_1, Menu_1_8, Menu_1, NULL_MENU, Item_Select_Callback, BackButton_Enter, 120, 95, 100, 25, 0, "Назад");

//MENU_ITEM(Menu_2_1, Menu_2_2, Menu_2_2, Menu_2, NULL_MENU, Item_Select_Callback, Level1Item2_Enter, 30, 30, 0, 0, 0, "");
//MENU_ITEM(Menu_2_2, Menu_2_1, Menu_2_1, Menu_2, NULL_MENU, Item_Select_Callback, Level1Item2_Enter, 100, 100, 0, 0, 0, "Назад");

void buttonListen(void *argument) {
	for(;;){
		if ((Menu_GetCurrentMenu() == &Menu_2) && (HAL_GPIO_ReadPin(RIGHT_GPIO_Port, RIGHT_Pin) == 1)) {
			ButtonState = 5;
		} else if((Menu_GetCurrentMenu() == &Menu_2) && (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == 0)){
			ButtonState = 6;
		} else if (HAL_GPIO_ReadPin(DOWN_GPIO_Port, DOWN_Pin) == 1) {
			ButtonState = 1;
		} else if (HAL_GPIO_ReadPin(UP_GPIO_Port, UP_Pin) == 1) {
			ButtonState = 2;
		} else if (HAL_GPIO_ReadPin(RIGHT_GPIO_Port, RIGHT_Pin) == 1) {
			ButtonState = 3;
		} else if (HAL_GPIO_ReadPin(LEFT_GPIO_Port, LEFT_Pin) == 1) {
			ButtonState = 4;
		}
		osDelay(2);
	}
}

void setRandomState(void *argument) {
	for(;;){
		int lengthNumber = getRandInt(0, used_elements);
		data_array[lengthNumber] = getRandInt(1, 9999);
		sprintf(current_messages[lengthNumber], "%s %d", messages[lengthNumber], data_array[lengthNumber]);
		getRandInt(0, 100);
		osDelay(250);
	}
}

void reDraw(void *argument) {
	for(;;){
		switch(ButtonState) {
			case 1:
				Menu_Navigate(Menu_GetCurrentMenu()->Next, Menu_GetCurrentLayerTopMenu()); // Next, Child, Parent, Previous
				ButtonState = 0;
				break;
			case 2:
				Menu_Navigate(Menu_GetCurrentMenu()->Previous, Menu_GetCurrentLayerTopMenu()); // Next, Child, Parent, Previous
				ButtonState = 0;
				break;
			case 3:
				Menu_Navigate(Menu_GetCurrentMenu()->Child, Menu_GetCurrentLayerTopMenu()->Child); // Next, Child, Parent, Previous
				ButtonState = 0;
				break;
			case 4:
				Menu_Navigate(Menu_GetCurrentMenu()->Parent, Menu_GetCurrentLayerTopMenu()->Parent); // Next, Child, Parent, Previous
				ButtonState = 0;
				break;
			case 6:
				//if(current_multiplier_index < max_multiplier_index) {
				//	current_multiplier_index++;
				//}
				if (!current_multiplier_index) { // check if this is not equal to zero
					current_multiplier_index--;
				}
				//if (current_bias_index < max_bias_index) {
				//	current_bias_index++;
				//}
				//if (!current_bias_index) { // check if this is not equal to zero
				//	current_bias_index--;
				//}
			case 5:
				u8g2_FirstPage(&u8g2);
				do {
					displayGraphPoints();
				} while (u8g2_NextPage(&u8g2));
				break;
			case 0:
				Menu_Navigate(Menu_GetCurrentMenu(), Menu_GetCurrentLayerTopMenu());
				break;
		}
		osDelay(50);
	}
}

int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_TIM10_Init();
	osKernelInitialize();

	srand(HAL_GetTick());
	for (size_t i = 0; i < used_elements; ++i) {
		sprintf(current_messages[i], "%s %d", messages[i], data_array[i]);
	}

	u8g2_Setup_uc1608_240x128_2(&u8g2, U8G2_R0, u8x8_byte_8bit_8080mode, u8g2_gpio_and_delay_stm32); //u8x8_byte_8bit_6800mode
	// DISPLAY INITIALIZATION
	u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_SetPowerSave(&u8g2, 0); // wake up display
	// START MENU WORK
	Menu_SetGenericWriteCallback(Generic_Write, &Menu_1);
	Menu_SetCurrentLayerTopMenu(&Menu_1);
	Menu_Navigate(&Menu_1, Menu_GetCurrentLayerTopMenu());

	buttonListenHandle = osThreadNew(buttonListen, NULL, &buttonListen_attributes);
	rewriteDisplayHandle = osThreadNew(reDraw, NULL, &rewriteDisplay_attributes);
	setRandomHandle = osThreadNew(setRandomState, NULL, &setRandom_attributes);

    osKernelStart();
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM10_Init(void)
{
  TIM_OC_InitTypeDef sConfigOC = {0};
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 0;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 65535;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim10);
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|CD_Pin|WR0_Pin|DB4_Pin
                          |DB5_Pin|WR1_Pin, GPIO_PIN_RESET);
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DB3_Pin|DB0_Pin|DB2_Pin|DB1_Pin
                          |DB7_Pin, GPIO_PIN_RESET);
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DB6_GPIO_Port, DB6_Pin, GPIO_PIN_RESET);
  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);
  /*Configure GPIO pins : LD2_Pin CD_Pin WR0_Pin DB4_Pin
                           DB5_Pin WR1_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|CD_Pin|WR0_Pin|DB4_Pin
                          |DB5_Pin|WR1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  /*Configure GPIO pins : LEFT_Pin UP_Pin DOWN_Pin RIGHT_Pin */
  GPIO_InitStruct.Pin = LEFT_Pin|UP_Pin|DOWN_Pin|RIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  /*Configure GPIO pins : DB3_Pin DB0_Pin DB2_Pin DB1_Pin
                           DB7_Pin */
  GPIO_InitStruct.Pin = DB3_Pin|DB0_Pin|DB2_Pin|DB1_Pin
                          |DB7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = DB6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DB6_GPIO_Port, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1);
}
#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line);
#endif
