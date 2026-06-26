/*
 * Keyboard.h
 *
 *  Created on: 6 июн. 2026 г.
 *      Author: rubis
 */

#ifndef INC_KEYBOARD_H_
#define INC_KEYBOARD_H_

#define KEYCODE_NOPRESSED   0xFF
#define KEYBOARD_TIK       10	 // Время между опросами клавиатуры (10мс)
#define DEBOUNCE_TIME       3    // Время защиты от дребезга (3 * 10мс = 30мс)
#define START_REPEAT_TIME  50    // Пауза перед началом повтора (50 * 10мс = 500мс)
#define REPEAT_PERIOD      10    // Период автоповтора (10 * 10мс = 100мс)

#define KEY_UP				 9  //колонка2 ряд3
#define KEY_DOWN			13  // 2 4
#define KEY_RIGHT			14  // 3 4
#define KEY_LEFT			12  // 1 4
#define KEY_OK				11  // 4 3
#define KEY_RET				12  // 4 1
#define KEY_PLAY			 7  // 4 2
#define KEY_REW 			 9  // 3 1
#define KEY_FW 			     3  // 4 1

#define KEY_0				13  // 2 4
#define KEY_1				 8  // 1 3
#define KEY_2				 9  // 2 3
#define KEY_3				10  // 1 2
#define KEY_4				 4  // 1 2
#define KEY_5				 5  // 2 2
#define KEY_6				 6  // 3 2
#define KEY_7				 0  // 1 1
#define KEY_8				 1  // 2 1
#define KEY_9				 2  // 3 1

#define KEY_EDIT			11  // 4 3
#define KEY_MENU			 8  // 1 3
#define KEY_STOP			15  // 4 4
#define KEY_ROTATE_F		 6  // 3 2
#define KEY_ROTATE_R		10  // 3 3

#define KEY_POS_STAKER_A			4 // 1 2
#define KEY_POS_STAKER_B			5 // 2 2
#define KEY_MOVE_TO_POS_STAKER_A	0 // 1 1
#define KEY_MOVE_TO_POS_STAKER_B	1 // 2 1

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// Состояния кнопки
typedef enum {
    BUTTON_RELEASED,
    BUTTON_DEBOUNCE,
    BUTTON_PRESSED,
    BUTTON_REPEAT_WAIT,
    BUTTON_REPEATING
} ButtonState_t;

// Структура для хранения данных о кнопке
typedef struct {
	uint8_t keyCode;
    ButtonState_t state;
    uint16_t counter;
} Button_t;


uint8_t Keyboard_Scan(void);
void setTimer(uint32_t *my_timer);
_Bool checkTimer(uint32_t *my_timer, uint32_t daly_ms);
uint8_t Process_Button(Button_t *btn);


#endif /* INC_KEYBOARD_H_ */
