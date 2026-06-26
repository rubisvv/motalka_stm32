/*
 * menu.c
 *
 *  Created on: 9 июн. 2026 г.
 *      Author: rubis
 */
#include <stdio.h>
#include "liquidcrystal_i2c.h"
#include "Keyboard.h"
#include "ui.h"


typedef struct MenuItem {
	const char *name;        // Имя пункта для экрана
	void (*action)(void);    // Функция-действие (если есть)
	struct MenuItem *parent;        // На уровень вверх
	struct MenuItem *child;         // На уровень вниз (подменю)
	struct MenuItem *next;          // Следующий пункт на этом же уровне
	struct MenuItem *prev;          // Предыдущий пункт на этом же уровне
} MenuItem;


//typedef struct MenuItem MenuItem;



MenuItem menu_main, menu_settings, menu_info, menu_brightness;

// Настройка связей меню
MenuItem menu_hard_setings;
MenuItem menu_save_mem;
MenuItem menu_load_mem;
MenuItem menu_stacker;
MenuItem menu_winding_motor;
MenuItem menu_br_sens_enable;
MenuItem menu_br_sens_norm;
MenuItem menu_screw_pitch;
MenuItem menu_encoder_steps;
MenuItem menu_engine_steps;
MenuItem menu_engine_type;
MenuItem menu_pulse_length;
MenuItem menu_stepper_dir_cor;
MenuItem menu_max_speed;
MenuItem menu_finish_speed;
MenuItem menu_stop_time;

MenuItem* current_item;




// Функции-действия для пунктов меню
void show_info(void) {
	// Здесь может быть вывод версии прошивки на дисплей
	//printf("--> ЭКРАН: Версия ПО: v1.0.0\n");
}

void set_brightness(void) {
	// Здесь может быть изменение ШИМ (PWM) для подсветки
	//printf("--> ЭКРАН: Настройка яркости: 50%%\n");
}


// Функция отображения текущего пункта
void draw_menu(int showmode) {
	//printf("[ ЭКРАН ]: Текщий пункт: %s\n", current_item->name);
	MenuItem* parent_item;
	if (current_item == NULL) return;

	parent_item = current_item->parent;
	if ((parent_item == NULL) | (showmode == 1)) {
		HD44780_Clear();
		HD44780_SetCursor(0, 0);
		HD44780_PrintStr(current_item->name);
	} else {
		HD44780_Clear();
		HD44780_SetCursor(0, 0);
		HD44780_PrintStr(parent_item->name);
		HD44780_SetCursor(3, 1);
		HD44780_PrintStr(current_item->name);
	}
}

// Обработчик кнопок
void handle_button(uint8_t button) {
	switch (button) {
	case KEY_DOWN: // Кнопка "Вниз/Вперед"
		if (current_item->next != NULL) {
			current_item = current_item->next;
		}
		break;

	case KEY_UP: // Кнопка "Вверх/Назад"
		if (current_item->prev != NULL) {
			current_item = current_item->prev;
		}
		break;

	case KEY_RIGHT: // Кнопка "ОК / Ввод"
		if (current_item->child != NULL) {
			// Если есть подменю — заходим в него
			current_item = current_item->child;
		} else if (current_item->action != NULL) {
			// Если подменю нет, но есть действие — выполняем
			current_item->action();
		}
		break;

	case KEY_LEFT: // Кнопка "Назад / Выход"
		//if (current_item->parent != NULL) {
			current_item = current_item->parent;
		//}
		break;
	}

	// После любого нажатия обновляем экран
	draw_menu(0);
}




void createMenu() {
	// Инициализация menu_hard_setings
	menu_hard_setings.next = &menu_save_mem, menu_hard_setings.prev =
			&menu_load_mem, menu_hard_setings.parent = NULL, menu_hard_setings.child =
			&menu_stacker, menu_hard_setings.name = "Hard_Setings", menu_hard_setings.action =
			show_info;

	// Инициализация menu_save_mem
	menu_save_mem.next = &menu_load_mem;
	menu_save_mem.prev = &menu_hard_setings;
	menu_save_mem.parent = NULL;
	menu_save_mem.child = NULL;
	menu_save_mem.name = "Save mem";
	menu_save_mem.action = show_info;

	// Инициализация menu_load_mem
	menu_load_mem.next = &menu_hard_setings;
	menu_load_mem.prev = &menu_save_mem;
	menu_load_mem.parent = NULL;
	menu_load_mem.child = NULL;
	menu_load_mem.name = "Load mem";
	menu_load_mem.action = show_info;

	// Инициализация menu_stacker
	menu_stacker.next = &menu_winding_motor;
	menu_stacker.prev = &menu_br_sens_norm;
	menu_stacker.parent = &menu_hard_setings;
	menu_stacker.child = &menu_screw_pitch;
	menu_stacker.name = "Stacker";
	menu_stacker.action = show_info;

	// Инициализация menu_winding_motor
	menu_winding_motor.next = &menu_br_sens_enable;
	menu_winding_motor.prev = &menu_stacker;
	menu_winding_motor.parent = &menu_hard_setings;
	menu_winding_motor.child = &menu_max_speed;
	menu_winding_motor.name = "Winding motor";
	menu_winding_motor.action = show_info;

	// Инициализация menu_br_sens_enable
	menu_br_sens_enable.next = &menu_br_sens_norm;
	menu_br_sens_enable.prev = &menu_winding_motor;
	menu_br_sens_enable.parent = &menu_hard_setings;
	menu_br_sens_enable.child = NULL;
	menu_br_sens_enable.name = "Br.sens.enable";
	menu_br_sens_enable.action = show_info;

	// Инициализация menu_br_sens_norm
	menu_br_sens_norm.next = &menu_stacker;
	menu_br_sens_norm.prev = &menu_br_sens_enable;
	menu_br_sens_norm.parent = &menu_hard_setings;
	menu_br_sens_norm.child = NULL;
	menu_br_sens_norm.name = "Br.sens.norm";
	menu_br_sens_norm.action = show_info;

	// Инициализация menu_screw_pitch
	menu_screw_pitch.next = &menu_encoder_steps;
	menu_screw_pitch.prev = &menu_stepper_dir_cor;
	menu_screw_pitch.parent = &menu_stacker;
	menu_screw_pitch.child = NULL;
	menu_screw_pitch.name = "Screw pitch";
	menu_screw_pitch.action = show_info;

	// Инициализация menu_encoder_steps
	menu_encoder_steps.next = &menu_engine_steps;
	menu_encoder_steps.prev = &menu_screw_pitch;
	menu_encoder_steps.parent = &menu_stacker;
	menu_encoder_steps.child = NULL;
	menu_encoder_steps.name = "Encoder steps";
	menu_encoder_steps.action = show_info;

	// Инициализация menu_engine_steps
	menu_engine_steps.next = &menu_engine_type;
	menu_engine_steps.prev = &menu_encoder_steps;
	menu_engine_steps.parent = &menu_stacker;
	menu_engine_steps.child = NULL;
	menu_engine_steps.name = "Engine steps";
	menu_engine_steps.action = hendl_menu_engine_steps;

	// Инициализация menu_engine_type
	menu_engine_type.next = &menu_pulse_length;
	menu_engine_type.prev = &menu_engine_steps;
	menu_engine_type.parent = &menu_stacker;
	menu_engine_type.child = NULL;
	menu_engine_type.name = "Engine Type";
	menu_engine_type.action = show_info;

	// Инициализация menu_pulse_length
	menu_pulse_length.next = &menu_stepper_dir_cor;
	menu_pulse_length.prev = &menu_engine_type;
	menu_pulse_length.parent = &menu_stacker;
	menu_pulse_length.child = NULL;
	menu_pulse_length.name = "Pulse length";
	menu_pulse_length.action = show_info;

	// Инициализация menu_stepper_dir_cor
	menu_stepper_dir_cor.next = &menu_screw_pitch;
	menu_stepper_dir_cor.prev = &menu_pulse_length;
	menu_stepper_dir_cor.parent = &menu_stacker;
	menu_stepper_dir_cor.child = NULL;
	menu_stepper_dir_cor.name = "Stepper dir cor";
	menu_stepper_dir_cor.action = show_info;

	// Инициализация menu_max_speed
	menu_max_speed.next = &menu_finish_speed;
	menu_max_speed.prev = &menu_stop_time;
	menu_max_speed.parent = &menu_winding_motor;
	menu_max_speed.child = NULL;
	menu_max_speed.name = "Max speed";
	menu_max_speed.action = show_info;

	// Инициализация menu_finish_speed
	menu_finish_speed.next = &menu_stop_time;
	menu_finish_speed.prev = &menu_max_speed;
	menu_finish_speed.parent = &menu_winding_motor;
	menu_finish_speed.child = NULL;
	menu_finish_speed.name = "Finish speed";
	menu_finish_speed.action = show_info;

	// Инициализация menu_stop_time
	menu_stop_time.next = &menu_max_speed;
	menu_stop_time.prev = &menu_finish_speed;
	menu_stop_time.parent = &menu_winding_motor;
	menu_stop_time.child = NULL;
	menu_stop_time.name = "Stop time";
	menu_stop_time.action = show_info;

	//	menu_hard_setings = {
	//			.next = &menu_save_mem,
	//			.prev = &menu_load_mem,
	//			.parent = NULL, .child = &menu_stacker, .name = "Hard_Setings",
	//			.action = show_info };
//	MenuItem menu_save_mem = { .next = &menu_load_mem, .prev = &menu_hard_setings,
//			.parent = NULL, .child = NULL, .name = "Save mem", .action = show_info };
//
//	MenuItem menu_load_mem = { .next = &menu_hard_setings, .prev = &menu_save_mem,
//			.parent = NULL, .child = NULL, .name = "Load mem", .action = show_info };
//
//	MenuItem menu_stacker = { .next = &menu_winding_motor, .prev =
//			&menu_br_sens_norm, .parent = &menu_hard_setings, .child =
//			&menu_screw_pitch, .name = "Stacker", .action = show_info };
//
//	MenuItem menu_winding_motor = { .next = &menu_br_sens_enable, .prev =
//			&menu_stacker, .parent = &menu_hard_setings, .child = &menu_max_speed,
//			.name = "Winding motor", .action = show_info };
//
//	MenuItem menu_br_sens_enable = { .next = &menu_br_sens_norm, .prev =
//			&menu_winding_motor, .parent = &menu_hard_setings, .child = NULL,
//			.name = "Br.sens.enable", .action = show_info };
//
//	MenuItem menu_br_sens_norm = { .next = &menu_stacker, .prev =
//			&menu_br_sens_enable, .parent = &menu_hard_setings, .child = NULL,
//			.name = "Br.sens.norm", .action = show_info };
//
//	MenuItem menu_screw_pitch = { .next = &menu_encoder_steps, .prev =
//			&menu_stepper_dir_cor, .parent = &menu_stacker, .child = NULL, .name =
//			"Screw pitch", .action = show_info };
//
//	MenuItem menu_encoder_steps = { .next = &menu_engine_steps, .prev =
//			&menu_screw_pitch, .parent = &menu_stacker, .child = NULL, .name =
//			"Encoder steps", .action = show_info, };
//
//	MenuItem menu_engine_steps = { .next = &menu_engine_type, .prev =
//			&menu_encoder_steps, .parent = &menu_stacker, .child = NULL, .name =
//			"Engine steps", .action = show_info, };
//
//	MenuItem menu_engine_type = { .next = &menu_pulse_length, .prev =
//			&menu_engine_steps, .parent = &menu_stacker, .child = NULL, .name =
//			"Engine Type", .action = show_info, };
//
//	MenuItem menu_pulse_length = { .next = &menu_stepper_dir_cor, .prev =
//			&menu_engine_type, .parent = &menu_stacker, .child = NULL, .name =
//			"Pulse length", .action = show_info, };
//
//	MenuItem menu_stepper_dir_cor = { .next = &menu_screw_pitch, .prev =
//			&menu_pulse_length, .parent = &menu_stacker, .child = NULL, .name =
//			"Stepper dir cor", .action = show_info, };
//
//	MenuItem menu_max_speed = { .next = &menu_finish_speed, .prev = &menu_stop_time,
//			.parent = &menu_winding_motor, .child = NULL, .name = "Max speed",
//			.action = show_info, };
//
//	MenuItem menu_finish_speed = {
//			.next = &menu_stop_time,
//			.prev = &menu_max_speed,
//			.parent = &menu_winding_motor, .child = NULL, .name = "Finish speed",
//			.action = show_info, };
//
//	MenuItem menu_stop_time = {
//			.next = &menu_max_speed,
//			.prev = &menu_finish_speed,
//			.parent = &menu_winding_motor,
//			.child = NULL,
//			.name = "Stop time",
//			.action = show_info, };

	current_item = &menu_hard_setings;
}

//**********************
void menu() {

	current_item = &menu_hard_setings;

	draw_menu(0);

	extern Button_t myButton;

	while (current_item != NULL) {
		uint8_t keyCode = Process_Button(&myButton);

		if (keyCode == KEYCODE_NOPRESSED) {
			continue;
		}

		handle_button(keyCode);
	}
}


//*************************************************************************************************


// ====================================================
// Обработка события выбора пункта меню
void MenuKeyEventsHandling(uint8_t HandlerNum)
{
//	char res;
//

//		break;
//
//		case HE_SET_QUANT :// Установка общего количества витков
//		ProgState = PS_SET_QUANT;
//		ShowMenu(MenuState, 1);
//		NumEditPos = 1;
//		NumTypePos = 1;
//		SetQuant();
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		//lcd_puts("5");
//		//lcd_puts(ProgState+48);
//		break;
//
//		case HE_SET_SCREW_STEP: // Установка шага подаюего винта
//		ProgState = PS_SET_SCREW_STEP;
//		ShowMenu(MenuState, 1);
//		NumEditPos = 1;
//		NumTypePos = 1;
//		SetScrewStep();
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//

//
//		case HE_SET_QPULS_VAL:	// Установка количества импульсов датчика вращения за один оборот подающего винта
//		ProgState = PS_SET_QPULS_VAL;
//		ShowMenu(MenuState, 1);
//		NumEditPos = 1;
//		NumTypePos = 1;
//		SetQPulsVal();
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//
//		case HE_SET_WIND_DIR:	// Установка начального направления намотки
//		ProgState = PS_SET_WIND_DIR;
//		ShowMenu(MenuState, 1);
//		SetWindDir();
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//
//		case HE_INP_QUANT_IN_LAY :// Установка количества витков в слое
//		ProgState = PS_INP_QUANT_IN_LAY;
//		ShowMenu(MenuState, 1);
//		NumEditPos = 1;
//		NumTypePos = 1;
//		SetQuantInLay();
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//
//		case HE_SET_WIND_MODE :// Установка режима намотки
//		ProgState = PS_SET_WIND_MODE;
//		ShowMenu(MenuState, 1);
//		NumEditPos = 1;
//		NumTypePos = 1;
//		SetWindMode();
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//
//		case HE_SET_STEP_ENG_PULSE_LEN :// Установка длины импульса
//		ProgState = PS_SET_STEP_ENG_PULSE_LEN;
//		ShowMenu(MenuState, 1);
//		NumEditPos = 1;
//		NumTypePos = 1;
//		SetStepIngPulseLen2();
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//
//		case HE_SET_TYPE_ENGINE:	// Установка начального направления намотки
//		ProgState = PS_SET_TYPE_ENGINE;
//		ShowMenu(MenuState, 1);
//		SetEngType();
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//
//		case HE_WIND:
//		res = Winding();
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//
//		case HE_SET_STEP_ENG_DIR_CORR :	// Установка начального направления намотки
//		//ProgState = PS_SET_TYPE_ENGINE;
//		ShowMenu(MenuState, 1);
//		SetStepperDirCorrection();
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//
//		case HE_SET_WIRE_BREACK_SENSOR_ENABLE :	// Установка начального направления намотки
//		//ProgState = PS_SET_TYPE_ENGINE;
//		ShowMenu(MenuState, 1);
//		SetWireBreakSensorEnable();
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//
//		case HE_SET_WIRE_BREACK_SENSOR_NORMAL_VAL :	// Установка начального направления намотки
//		//ProgState = PS_SET_TYPE_ENGINE;
//		ShowMenu(MenuState, 1);
//		SetWireBreakSensorNormVal ();
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//
//		case HE_SET_MAX_WIND_MOTOR_SPEED:
//		ShowMenu(MenuState, 1);
//		NumEditPos = 1;
//		NumTypePos = 1;
//		SetMaxWinMotorSpeed(0);
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//
//		case HE_SET_WIND_MOTOR_FINISH_SPEED:
//		ShowMenu(MenuState, 1);
//		NumEditPos = 1;
//		NumTypePos = 1;
//		SetMaxWinMotorSpeed(1);
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//
//		case HE_SET_WIND_MOTOR_STOP_TIME:
//		ProgState = PS_SET_QPULS_VAL;
//		ShowMenu(MenuState, 1);
//		NumEditPos = 1;
//		NumTypePos = 1;
//		SetWinMotorStopTime();
//		ProgState = PS_MENU;
//		ShowMenu(MenuState, 0);
//		break;
//
//	}//lcd_putc("3");
}





