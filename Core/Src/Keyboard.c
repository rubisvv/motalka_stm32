/*
 * Keyboard.c
 *
 *  Created on: 6 июн. 2026 г.
 *      Author: rubis
 */
#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include "Keyboard.h"

Button_t myButton = { 0, BUTTON_RELEASED, 0 };
uint32_t keyboard_timer = 0;

// *** Сканирование матричной клавиатуры
uint8_t Keyboard_Scan(void)
{
    const uint16_t rows[4] =
    {
        GPIO_PIN_0,
        GPIO_PIN_1,
        GPIO_PIN_2,
        GPIO_PIN_3
    };

    const uint16_t cols[4] =
    {
        GPIO_PIN_4,
        GPIO_PIN_5,
        GPIO_PIN_6,
        GPIO_PIN_7
    };

    for(uint8_t row = 0; row < 4; row++)
    {
        /* Все строки в 1 */
        HAL_GPIO_WritePin(GPIOA,
                          GPIO_PIN_0 | GPIO_PIN_1 |
                          GPIO_PIN_2 | GPIO_PIN_3,
                          GPIO_PIN_SET);

        /* Активная строка = 0 */
        HAL_GPIO_WritePin(GPIOA, rows[row], GPIO_PIN_RESET);

        for(volatile int i = 0; i < 100; i++);

        for(uint8_t col = 0; col < 4; col++)
        {
            if(HAL_GPIO_ReadPin(GPIOA, cols[col]) == GPIO_PIN_RESET)
            {
                return row * 4 + col;
            }
        }
    }

    return KEYCODE_NOPRESSED;
}


// *** Запуск таймера
void setTimer(uint32_t *my_timer) {
	*my_timer = HAL_GetTick();
}


// *** Проверка переполнения таймера
_Bool checkTimer(uint32_t *my_timer, uint32_t daly_ms) {
	if ((HAL_GetTick() - *my_timer) >= daly_ms) {
		return 1;
	} else {
		return 0;
	}
}


/**
  * @brief  Эта функция должна вызываться циклически (например, в прерывании таймера каждые 10 мс)
  * @retval Возвращает true в момент нажатия или во время автоповтора
  */
uint8_t Process_Button(Button_t *btn) {
	uint8_t resultKeyCode = KEYCODE_NOPRESSED;

	if (!checkTimer(&keyboard_timer, KEYBOARD_TIK)) {
		return resultKeyCode;
	}
	else
	{
		setTimer(&keyboard_timer);
	}

	uint8_t	keyCode = Keyboard_Scan();
	bool isPressedNow = keyCode != KEYCODE_NOPRESSED;


	if ((btn->state == BUTTON_RELEASED) || (btn->keyCode!= keyCode)) {
		if (isPressedNow) {
			btn->keyCode = keyCode;
			btn->counter = 0;
			btn->state = BUTTON_DEBOUNCE;
		}
		else{
			btn->state == BUTTON_RELEASED;
			btn->counter = 0;
			btn->keyCode = keyCode;
		}
	}
	else if (btn->state == BUTTON_DEBOUNCE) {
		if (isPressedNow) {
			btn->counter++;
			if (btn->counter >= DEBOUNCE_TIME) {
				btn->keyCode = keyCode;
				btn->state = BUTTON_PRESSED;
			}
		} else {
			btn->state = BUTTON_RELEASED;
			btn->keyCode = KEYCODE_NOPRESSED;
		}
	}
	else if (btn->state == BUTTON_PRESSED) {
		// Фиксируем первое нажатие
		resultKeyCode = btn->keyCode;
		btn->counter = 0;
		btn->state = BUTTON_REPEAT_WAIT;
	}
	else if (btn->state == BUTTON_REPEAT_WAIT) {
		if (isPressedNow && btn->keyCode == keyCode) {
			btn->counter++;
			if (btn->counter >= START_REPEAT_TIME) {
				btn->counter = 0;
				btn->state = BUTTON_REPEATING;
			}
		} else {
			btn->state = BUTTON_RELEASED;
		}
	}
	else if (btn->state == BUTTON_REPEATING) {
		if (isPressedNow && btn->keyCode == keyCode) {
			btn->counter++;
			if (btn->counter >= REPEAT_PERIOD) {
				// Генерируем повторное нажатие
				resultKeyCode = btn->keyCode;
				btn->counter = 0;
			}
		} else {
			btn->state = BUTTON_RELEASED;
		}
	}

    return resultKeyCode;
}



