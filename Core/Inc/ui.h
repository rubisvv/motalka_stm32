/*
 * screen.h
 *
 *  Created on: 13 июн. 2026 г.
 *      Author: rubis
 */

#ifndef INC_UI_H_
#define INC_UI_H_

#define MAX_Q_FIELDS 8

typedef struct {
	const uint8_t     Nom;
	const uint8_t     Next;
	const uint8_t     Previous;
	//const uint8_t     HandlerNum;
	const char Text[16];
	const char Short[5];
} ListItem;

typedef struct {
	uint8_t Ind;		// номер поля
	uint8_t Col;		// колонка
	uint8_t Row;		// ряд
	uint8_t ComaPos;	// позиция запятой
	uint8_t NumLen;   // длина числа (знаков без запятой)
	uint8_t FieldType;   // 0 - число, 1- список
	uint8_t QBytes;   // длина поля в байтах
} DialogFieldStuct;

typedef struct {
	uint16_t	Num;
	uint8_t	CommaPos;
	} Num16;

typedef struct {
	uint8_t	Num;
	uint8_t	CommaPos;
} Num8;


void hendl_menu_engine_steps();

#endif /* INC_UI_H_ */
